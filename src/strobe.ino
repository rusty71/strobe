#include "TaskScheduler.h"
//http://www.gammon.com.au/adc

//Band pass bessel filter order=1 alpha1=0.0041666666666667 alpha2=0.083333333333333 
//http://www.schwietering.com/jayduino/filtuino/index.php?characteristic=be&passmode=bp&order=1&usesr=usesr&sr=9600&frequencyLow=40&noteLow=&frequencyHigh=800&noteHigh=&pw=pw&calctype=long&bitres=16&run=Send
class filter
{
	public:
		filter()
		{
			for(int i=0; i <= 2; i++)
				v[i]=0;
		}
	private:
		short v[3];
	public:
		short step(short x)
		{
			v[0] = v[1];
			v[1] = v[2];
			long tmp = ((((x *  29037L) >>  3)	//= (   2.2153254040e-1 * x)
				+ ((v[0] * -19495L) >> 1)	//+( -0.5949374815*v[0])
				+ (v[1] * 25949L)	//+(  1.5837876357*v[1])
				)+8192) >> 14; // round and downshift fixed point /16384

			v[2]= (short)tmp;
			return (short)((
				 (v[2] - v[0]))); // 2^
		}
} filt;

//volatile for ISR communication
volatile int8_t cross = false;
volatile long count_last = 0, count = 0, lcount = 0;
#define THRESHOLD  (4)	//TODO: make dynamic

ISR (ADC_vect)
{
static bool was_negative = true; //Zero crossing from negative to positive
static short step, phase = 0;


	digitalWrite(8, HIGH);	//for measuring timing
	count++;
	lcount++;  //FIXME
	step = filt.step((short)ADC);	//23us !
	//detect zero crossing
	if(was_negative) {	//last valuation was negative
		if((step - THRESHOLD) > 0) { //negative to positive
			was_negative = false;
			count_last = lcount;
			lcount = 0;
			phase = 0;
			count = 0;
			cross++;	//doorbel
		}
	}
	else {
		if((step + THRESHOLD) < 0) {
			was_negative = true;
		}
	}
	digitalWrite(8, LOW);
}

Scheduler runner;
void t1Callback();	//proto
Task t1(30, TASK_FOREVER, &t1Callback);
void t2Callback();	//proto
Task t2(60, TASK_FOREVER, &t2Callback);
void blinkCallback();	//proto
Task blink(5, TASK_ONCE, &blinkCallback);

//~ Task blink(0, TASK_ONCE, &blinkCallback, &ts, false, &BlinkOnEnable, &BlinkOnDisable);

void setup ()
{
	Serial.begin (115200);
	Serial.println("Start");
	//filter speed test, float=51us, int=23
	//~ start = millis();
	//~ for(int j=0; j<10000; j++) {
		//~ result = filt.step(3.13);
	//~ }
	//~ end = millis();
	//~ Serial.println((int)(end - start));

	ADCSRA =  bit (ADEN);								//turn ADC on
	ADCSRA |= bit (ADPS0)|  bit (ADPS1) | bit (ADPS2);	// Prescaler of 128(9600 Hz), //64 (1 << ADPS2) | (1 << ADPS1);
	ADMUX  =  bit (REFS0)| (0 & 0x07);					//AVcc and select input port
	ADCSRA |= bit (ADSC) | bit (ADIE);
	ADCSRA |= bit (ADATE);
	pinMode(9,OUTPUT);
	TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
	pinMode(8,OUTPUT);
	//digitalWrite(9, HIGH);
	digitalWrite(8, LOW);

	//setup task
	runner.init();
	runner.addTask(t1);
	t1.enable();
	runner.addTask(t2);
	t2.enable();

	runner.addTask(blink);
	blink.enableDelayed();
}

typedef struct {
	long angle;
	int next;
	int period;
	int power;
	unsigned long on;
} ANGLE;

ANGLE angles[] = { {10, 0, 10, 50, 0}, {10, 0, 1, 255, 0}, {10, 0, 1, 255, 0} };

#define FRAC_SHIFT 16
//integer degrees 0-359
void setangle(int slot, long angle) {
	angles[slot].angle = (angle << FRAC_SHIFT)/120;	//expensive division. test speed.
}

//returns tick for s give angle, given current speed
//called every revolution, must be fast!
int getangle(int slot, int last_ticks) {
	return (angles[slot].angle * last_ticks)>>FRAC_SHIFT;
}

//called once
void blinkCallback() {
	//~ analogWrite(9, 255);		//ON
	analogWrite(9, 0);		//OFF
}

void t1Callback() {
	static int angle = 0;
	angle = angle + 3;
	if(angle > 120)
		angle = 0;
	setangle(0, angle);	
}

uint8_t lebpwm[16] = { 0, 10, 50,100,150,200,230,255 };
void t2Callback() {
	static int angle = 120;
	static int power = 0;
	
	angle = angle - 5;
	if(angle < 0)
		angle = 120;
	setangle(1, angle);	
	//~ if(power++ > 7)
		//~ power = 0;
	//~ angles[1].power = (int)lebpwm[power];	//TODO: member func
	//~ angles[1].power = 255;	//TODO: member func
}

#define NR_ANGLES 2

void loop ()
{
	while(!cross) {
		for(int angle_i = 0; angle_i < NR_ANGLES; angle_i++) {
			if(count == angles[angle_i].next) {	//TODO: can miss
				analogWrite(9, angles[angle_i].power);
				blink.restartDelayed(angles[angle_i].period);
			}
			else {
			}
			
		}
	runner.execute();
	}
	if(cross > 1)
		Serial.println("missed");
	cross = 0;

	//~ //update angles for next iteration
	for(int angle_i = 0; angle_i < NR_ANGLES; angle_i++) {
		angles[angle_i].next = getangle(angle_i, count_last);	//calculate next fire moment
	}
	//schedule tasks	//SPEED!
	runner.execute();

	//~ Serial.print(count_last);
	//~ Serial.print(":");
	//~ Serial.println(count);
	//~ Serial.print(":");
	//~ Serial.println(angles[1].next);
}

