#define _TASK_MICRO_RES	
#define _TASK_LTS_POINTER
#include "TaskScheduler.h"

#define RED_PIN 9 
#define GREEN_PIN 10 
#define BLUE_PIN 11 

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


//Continious ADC reading : 
//http://www.gammon.com.au/adc
//volatile for ISR communication
volatile int8_t cross = false;
volatile long count_last = 0, count = 0;
#define THRESHOLD  (4)	//TODO: make dynamic
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


//exponential LED dimming
//R=(255 * math.log10(2))/(math.log10(255))
//[(math.pow(2, (float(x)/R)) - 1) for x in range(255)]
//"".join(['const int8_t pwm[] = { 0']+[ ", %d" % (int((math.pow(2, (float(x)/R))))) for x in range(1,255)]+[', 255 };'])
const int8_t led_log[] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 41, 42, 43, 44, 45, 46, 47, 48, 49, 51, 52, 53, 54, 55, 56, 58, 59, 60, 62, 63, 64, 66, 67, 69, 70, 72, 73, 75, 77, 78, 80, 82, 84, 86, 87, 89, 91, 93, 95, 98, 100, 102, 104, 106, 109, 111, 114, 116, 119, 121, 124, 127, 130, 132, 135, 138, 141, 144, 148, 151, 154, 158, 161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 200, 205, 209, 214, 219, 223, 228, 233, 238, 244, 249, 255 };
class LED
{
	public:
		void write(void) {
			//~ Serial.print(spinner.color_flash[0],HEX);
			//~ Serial.print(":");
			//~ Serial.print(spinner.color_flash[1],HEX);
			//~ Serial.print(":");
			//~ Serial.println(spinner.color_flash[2],HEX);
			analogWrite(RED_PIN,   led_log[min(pr_color, 255)]);
			analogWrite(GREEN_PIN, led_log[min(pg_color, 255)]);
			analogWrite(BLUE_PIN,  led_log[min(pb_color, 255)]);
		}

		void add(uint8_t *color) {
			pr_color += color[0];
			pg_color += color[1];
			pb_color += color[2];
		}

		void sub(uint8_t *color) {
			pr_color -= color[0];
			pg_color -= color[1];
			pb_color -= color[2];
		}

	private:
		uint16_t pr_color = 0, pg_color = 0, pb_color = 0;

} led;

Scheduler runner;

extern "C"
{
	// callback typedef
	typedef void(*sceneTask) ();
} 
#define FRAC_SHIFT 13

bool ledon(void);
void ledoff(void);

class Spinner
{
	public:
		Spinner(int led, uint32_t period, uint8_t power, sceneTask scene)
			: period(period), power(power), scene(scene) {
				this->t.set(period, TASK_FOREVER, scene, NULL, NULL);
				this->t.setLtsPointer(this);
				this->flashtask.set(0, TASK_ONCE, ledoff, ledon, NULL);
				this->flashtask.setLtsPointer(this);
			}

		void updateTrigger(uint16_t tick_rev) {	//slow, run at rev speed
			this->triggered = false;
			this->trigger = (this->angle_tick_ratio * tick_rev)>>FRAC_SHIFT;
		}

		void setAngle(uint16_t angle) {	//slower, run at scene speed
			this->angle = angle;
			//~ this->angle_tick_ratio = (((uint32_t)angle) << FRAC_SHIFT)/120;	//expensive division. test speed.
			//~ this->angle_tick_ratio = (((uint32_t)angle) << (FRAC_SHIFT-7));  //divide 128
			//from Hackers delight.
			//"0.0000 0010 0010 0010 0010 0010 0010 0010"  // 1/120.0
			//"1000 1001 0001 0001 0001 0001 0"  // remove leading zeroes
			uint32_t tmp = (uint32_t)angle << FRAC_SHIFT-6;
			this->angle_tick_ratio =  tmp >> 1;
			this->angle_tick_ratio += tmp >> 5;
			this->angle_tick_ratio += tmp >> 8;
			this->angle_tick_ratio += tmp >> 12;
			//~ this->angle_tick_ratio += ((uint32_t)angle) >> 16;	//too much accuracy
			//~ this->angle_tick_ratio += ((uint32_t)angle) >> 20;	//too much accuracy
			//~ this->angle_tick_ratio >>=   6;		//already done above
		}
		
		void doCheckTrigger(int tick) {		//fast, runs at frame speed
			if( !this->triggered && (tick >= this->trigger) ) {
				this->flashtask.restartDelayed(this->duration);
				this->triggered = true;
			}
		}

	public:
		Task t, flashtask;
		bool flash = false;
		int period;
		uint16_t duration = 700;
		uint16_t angle;
		uint8_t color[3] = {50,50,50};
		uint8_t power = 255;
		uint8_t color_flash[3];
		bool triggered = false;
	
	private:
		uint16_t trigger;
		uint32_t angle_tick_ratio;	//fraction to calculate tick for given angle
		sceneTask scene;		//the scene task C callback function
};

bool ledon(void){
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	if(spinner.flash)	//already on, remove last
		led.sub(spinner.color_flash);

	spinner.color_flash[0] = (spinner.color[0] * spinner.power) >> 8;
	spinner.color_flash[1] = (spinner.color[1] * spinner.power) >> 8;
	spinner.color_flash[2] = (spinner.color[2] * spinner.power) >> 8;

	led.add(spinner.color_flash);
	led.write();
	spinner.flash=true;

	return true;
}

void ledoff(void){
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	led.sub(spinner.color_flash);
	led.write();
	spinner.flash=false;
}

void scene1() {
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	static uint16_t iter = 0;
	uint16_t angle;
	spinner.color[0] = ((iter>>8)&1)*50;
	spinner.color[1] = ((iter>>9)&1)*75;
	spinner.color[2] = ((iter>>10)&1)*128;
	//~ spinner.color[0] = (sin(iter/(300*3.1416))*100)+50;
	//~ spinner.color[1] = (sin(iter/(200*3.1416))*150)+75;
	//~ spinner.color[2] = (sin(iter/(100*3.1416))*255)+127;
	//~ Serial.print(spinner.color[0]);
	//~ Serial.print(":");
	//~ Serial.print(spinner.color[1]);
	//~ Serial.print(":");
	//~ Serial.println(spinner.color[2]);
	spinner.power = 255;
	spinner.duration = 300;
	//~ angle = (uint16_t)((sin(iter++/(2*3.1416))*60)+60);
	//~ spinner.duration = (sin(iter/(30*3.1416))*2000)+3000;
	//~ spinner.setAngle( angle );
	iter++;
}

void scene2() {
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	static uint16_t iter = 0;
	uint16_t angle;
	//~ spinner.color[0] = 15;	//15
	spinner.color[0] = 0;
	spinner.color[1] = 255;
	spinner.color[2] = 0;
	spinner.power = 100;
	spinner.duration = 400;
	//~ spinner.power = ((sin(iter/(12*3.1416))*15)+20);
	angle = (uint16_t)((cos(iter++/(6*3.1416))*60)+60);
	//~ Serial.println(angle);
	spinner.setAngle( angle );
}

void scene3() {
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());

	spinner.color[0] = 0;
	spinner.color[1] = 0;
	spinner.color[2] = 255;
	spinner.power = 255;
	spinner.duration = 400;
	
}

void scene4() {
	static uint16_t angle = 0;
	//get reference to Spinner object
	Spinner& spinner = *((Spinner*) runner.currentLts());
	//~ spinner.color[0] = 15;	//15
	spinner.color[0] = 0;
	spinner.color[1] = 0;
	spinner.color[2] = 255;
	spinner.power = 100;
	spinner.duration = 600;
	if(angle > 120)
		angle = 0;
	angle = angle+10;
	spinner.setAngle( angle );
}

Spinner spinners[] = {
					{1, 5000, 6, scene1},
					{1, 50000, 6, scene2},
					//~ {1, 50000, 6, scene3},
					{1, 50000, 6, scene4},
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
	//faster PWM to allow short flashes
	//---------------------------------------------- Set PWM frequency for D9 & D10 ------------------------------
	TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz
	//~ TCCR1B = TCCR1B & B11111000 | B00000010;    // set timer 1 divisor to     8 for PWM frequency of  3921.16 Hz	
	//---------------------------------------------- Set PWM frequency for D3 & D11 ------------------------------
	TCCR2B = TCCR2B & B11111000 | B00000001;    // set timer 2 divisor to     1 for PWM frequency of 31372.55 Hz

	pinMode(RED_PIN,OUTPUT);
	pinMode(GREEN_PIN,OUTPUT);
	pinMode(BLUE_PIN,OUTPUT);
	pinMode(7,INPUT);

	//setup task scheduler
	runner.init();
	
	//add spinners to scheduler
	for(int i = 0; i < sizeof(spinners)/sizeof(spinners[0]); i++) {
		runner.addTask(spinners[i].t);
		spinners[i].t.enable();
		runner.addTask(spinners[i].flashtask);
		spinners[i].flashtask.enable();
	}

}

uint8_t kill[3] = {0,0,0};
void loop ()
{
	static long cc = 0;
	
	while(!cross) {
		for(int i = 0; i < sizeof(spinners)/sizeof(spinners[0]); i++) {
			spinners[i].doCheckTrigger(count);
			runner.execute();
		}
	}
	cross = 0;	//reset rev trigger

	for(int i = 0; i < sizeof(spinners)/sizeof(spinners[0]); i++) {
		spinners[i].updateTrigger(count_last);
		runner.execute();
	}
	runner.execute();
	//~ led.off(kill);

	//~ Serial.println(count_last-cc);
	cc=count_last;
}
