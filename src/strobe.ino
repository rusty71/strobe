#define _TASK_MICRO_RES	
#define _TASK_LTS_POINTER
#include "TaskScheduler.h"

#define RED_PIN 9 
#define GREEN_PIN 10 
#define BLUE_PIN 11 


//exponential LED dimming
//https://diarmuid.ie/blog/pwm-exponential-led-fading-on-arduino-or-other-platforms/
//R=(255 * math.log10(2))/(math.log10(255))
//[(math.pow(2, (float(x)/R)) - 1) for x in range(255)]
//"".join(['const int8_t pwm[] = { 0']+[ ", %d" % (int((math.pow(2, (float(x)/R))))) for x in range(1,255)]+[', 255 };'])

//Continious ADC reading : 
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



//~ //Band pass bessel filter order=1 alpha1=0.0083333333333333 alpha2=0.03125 
//~ class filter
//~ {
	//~ public:
		//~ filter()
		//~ {
			//~ for(int i=0; i <= 2; i++)
				//~ v[i]=0;
		//~ }
	//~ private:
		//~ short v[3];
	//~ public:
		//~ short step(short x)
		//~ {
			//~ v[0] = v[1];
			//~ v[1] = v[2];
			//~ long tmp = ((((x *  18351L) >>  4)	//= (   7.0004258642e-2 * x)
				//~ + ((v[0] * -28360L) >> 1)	//+( -0.8654637273*v[0])
				//~ + (v[1] * 30407L)	//+(  1.8558660897*v[1])
				//~ )+8192) >> 14; // round and downshift fixed point /16384

			//~ v[2]= (short)tmp;
			//~ return (short)((
				 //~ (v[2] - v[0]))); // 2^
		//~ }
//~ }filt;


//const int8_t pwm[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56, 58, 59, 60, 62, 63, 64, 66, 67, 69, 70, 72, 73, 75, 77, 78, 80, 82, 84, 86, 87, 89, 91, 93, 95, 98, 100, 102, 104, 106, 109, 111, 114, 116, 119, 121, 124, 127, 130, 132, 135, 138, 141, 144, 148, 151, 154, 158, 161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 200, 205, 209, 214, 219, 223, 228, 233, 238, 244, 249, 255 };
//~ const int8_t pwm[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56, 58, 59, 60, 62, 63, 64, 66, 67, 69, 70, 72, 73, 75, 77, 78, 80, 82, 84, 86, 87, 89, 91, 93, 95, 98, 100, 102, 104, 106, 109, 111, 114, 116, 119, 121, 124, 127, 130, 132, 135, 138, 141, 144, 148, 151, 154, 158, 161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 200, 205, 209, 214, 219, 223, 228, 233, 238, 244, 249, 255 };
void off(void) {
	//~ Serial.println("off");
	analogWrite(RED_PIN, 0);
	analogWrite(GREEN_PIN, 0);
	analogWrite(BLUE_PIN,  0);
}	

#define POWER 255

class Flash
{
	public:
		void flash(uint16_t duration, uint8_t * color) {
			//~ Serial.println("on");
			analogWrite(RED_PIN,  color[0]);
			analogWrite(GREEN_PIN, color[1]);
			analogWrite(BLUE_PIN,  color[2]);
			offtimer.restartDelayed(duration);
		}

	Task offtimer = {0, TASK_ONCE, off, NULL, NULL};
} led;

//volatile for ISR communication
volatile int8_t cross = false;
volatile long count_last = 0, count = 0;
#define THRESHOLD  (6)	//TODO: make dynamic
/*
 * The ISR is called every ~100us (9600Hz) and read a sample from the QRD1114
 * sensor. The data is filtered by a bandpass filter.
 * The zero crossings are detected, the positive crossing passed to userspace
 * by the cross doorbell and count_last
 */
ISR (ADC_vect)
{
static bool was_negative = true; //Zero crossing from negative to positive
static short step, phase = 0;

	count++;
	step = filt.step((short)ADC);	//~23us !
	//detect zero crossing
	if(was_negative) {
		if((step - THRESHOLD) > 0) {
			was_negative = false;
			count_last = count;
			count = 0;
			cross++;	//doorbel
		}
	}
	else {
		if((step + THRESHOLD) < 0) {
			was_negative = true;
		}
	}
}

Scheduler runner;

extern "C"
{
	// callback typedef
	typedef void(*sceneTask) ();
} 
#define FRAC_SHIFT 15

class Spinner
{
	public:
		Spinner(int led, uint32_t period, int power, sceneTask scene)
			: period(period), power(power), scene(scene) {
				this->t.set(period, TASK_FOREVER, scene, NULL, NULL);
				this->t.setLtsPointer(this);
			}

		void updateTrigger(uint16_t tick_rev) {	//slow, run at rev speed
			this->triggered = false;
			this->trigger = (this->angle_tick_ratio * tick_rev)>>FRAC_SHIFT;
		}

		void setAngle(uint16_t angle) {	//slower, run at scene speed
			this->angle = angle;
			//~ off();
			this->angle_tick_ratio = (((uint32_t)angle) << FRAC_SHIFT)/120;	//expensive division. test speed.
			//~ this->angle_tick_ratio = (((uint32_t)angle) << (FRAC_SHIFT-7));  //divide 128
		}
		
		void doCheckTrigger(int tick) {		//fast, runs at frame speed
			if( !this->triggered && (tick >= this->trigger) ) {
				this->triggered = true;
				led.flash(this->duration, this->color);
			}
		}

	public:
		Task t;
		int period;
		uint16_t duration = 700;
		uint16_t angle;
		uint8_t color[3] = {50,50,50};
	
	private:
		uint16_t trigger;
		uint32_t angle_tick_ratio;	//fraction to calculate tick for given angle
		int power;
		bool triggered;
		sceneTask scene;		//the scene task C callback function
};


//~ void scene1() {
	//~ //get reference to Spinner object
	//~ Spinner& spinner = *((Spinner*) runner.currentLts());
	//~ spinner.color[0] = 0;
	//~ spinner.color[1] = 255;
	//~ spinner.color[2] = 0;
	//~ if(spinner.angle < 90)
		//~ spinner.setAngle(spinner.angle+5);
	//~ else
		//~ spinner.setAngle(30);
//~ }

void scene1() {
	static uint16_t iter = 0;
	uint16_t angle;
	
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());
	spinner.color[0] = 0;
	spinner.color[1] = 255;
	spinner.color[2] = 0;
	angle = (uint16_t)((sin(iter++/(10*3.1416))*60)+60);
	//~ spinner.duration = (sin(iter/(30*3.1416))*2000)+3000;
	//~ Serial.println(angle);
	spinner.setAngle( angle );
}

void scene2() {
	static uint16_t iter = 0;
	uint16_t angle;
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());
	//~ spinner.color[0] = 15;	//15
	spinner.color[1] = 0;
	spinner.color[2] = 0;
	spinner.color[0] = ((sin(iter/(12*3.1416))*15)+20);
	angle = (uint16_t)((cos(iter++/(33*3.1416))*60)+60);
	//~ Serial.println(angle);
	spinner.setAngle( angle );
}

void scene3() {
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	static uint16_t iter = 0;

	if( !(iter++ % 7) ) {
		spinner.color[0] = 20;
		spinner.color[1] = 255;
		spinner.color[2] = 0;
	}
	else {
		spinner.color[0] = 0;
		spinner.color[1] = 0;
		spinner.color[2] = 0;
	}
	spinner.setAngle( 60 );
	
}

#define NR_OF_SPINNERS 2
Spinner spinners[] = {
					{1, 8000, 6, scene1},
					{1, 8000, 6, scene2},
					{1, 10000, 6, scene3},
	};

void setup ()
{
	Serial.begin (115200);
	Serial.println("Start");
	Serial.println(String());

	ADCSRA =  bit (ADEN);								//turn ADC on
	ADCSRA |= bit (ADPS0)|  bit (ADPS1) | bit (ADPS2);	// Prescaler of 128(9600 Hz), //64 (1 << ADPS2) | (1 << ADPS1);
	ADMUX  =  bit (REFS0)| (0 & 0x07);					//AVcc and select input port
	ADCSRA |= bit (ADSC) | bit (ADIE);
	ADCSRA |= bit (ADATE);
	//---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
	TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
	//~ TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz	
	//---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
	TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz

	pinMode(RED_PIN,OUTPUT);
	pinMode(GREEN_PIN,OUTPUT);
	pinMode(BLUE_PIN,OUTPUT);
	pinMode(7,INPUT);

	//setup task
	runner.init();
	
	runner.addTask(led.offtimer);

	//add spinners to scheduler
	for(int i = 0; i < NR_OF_SPINNERS; i++) {
		runner.addTask(spinners[i].t);
		spinners[i].t.enable();
	}

}

void loop ()
{
	static long cc = 0;
	
	while(!cross) {
		for(int i = 0; i < NR_OF_SPINNERS; i++) {
			spinners[i].doCheckTrigger(count);
			runner.execute();
		}
	}
	cross = 0;	//reset rev trigger

	if( ((count_last-cc) > 0) && ((count_last-cc) < 8) ) {
		for(int i = 0; i < NR_OF_SPINNERS; i++) {
			spinners[i].updateTrigger(count_last);
			runner.execute();
		}
	}
	else {	//use previous rev count when deviation is too big
		for(int i = 0; i < NR_OF_SPINNERS; i++) {
			spinners[i].updateTrigger(cc);
			runner.execute();
		}
	}
	runner.execute();
	cc=count_last;

	Serial.println(count_last-cc);
}

