/*
 * Simulator for avionics and motor sensors
 *
 * Author:
 * 		Maxime Guillemette
 * 
 * Brief:
 * 		Simulates the avionics by sending avionics data packets
 * 		(with slight randomness) and simulates the motor sensors
 * 		using 5 analog outputs (all normal distributions with 
 * 		various means and variances)
 *
 * Hardware:
 * 		The simulator is made to work on a Arduino Mega board.
 * 		Since Arduino boards output analog signals using a PWM,
 * 		some extra hardware (resistors and capacitors) should
 * 		be used to filter the Arduino's output and create a
 * 		true analog signal.
 */


#define pi 3.14159265f

/* analog pins simulating sensors */
#define SENSORS_NUM 5

typedef struct {
	int pin;
	unsigned char val;
	unsigned char mu;
	unsigned char sigma;
} sensor_sim_t;

sensor_sim_t sensors[VALVES_NUM] = {
	{.pin = 2,	.val = 0,	.mu = 10,	.sigma = 1},
	{.pin = 3,	.val = 0,	.mu = 128,	.sigma = 10},
	{.pin = 4,	.val = 0,	.mu = 50,	.sigma = 20},
	{.pin = 5,	.val = 0,	.mu = 200,	.sigma = 4},
	{.pin = 6,	.val = 0,	.mu = 30,	.sigma = 5},
};


/*
 * Generate random values on a normal distribution using the 
 * Box-Muller method
 */
void get_random_analog(sensor_sim_t* sensor) {
	float u1;
	float u2;
	float z0;

	/* generate random value z0 in Normal(0, 1) distribution */
	u1 = 0.001 * random(0, 1000);
	u2 = 0.001 * random(0, 1000);
	z0 = sqrt(-2 * log(u1)) * cos(2 * pi * u2);

	/* normalize distribution to Normal(mu, sigma^2) */
	z0 = (z0 * sensor->sigma) + sensor->mu;

	/* clip range to fit in analog values */
	if (z0 < 0) {
		z0 = 0;
	} else if (z0 > 255) {
		z0 = 255;
	}

	sensor->val = (unsigned char) z0;
}


void setup(void) {
	/* init analog outputs */
	for (int i = 0; i < VALVES_NUM; ++i) {
		pinMode(sensors[i].pin, OUTPUT);
	}

	/* init debug serial port */
	Serial.begin(115200);
	Serial.print("start\n");

	/* init avionics simulation serial port */
	Serial1.begin(115200);
}


void loop(void) {
	/* check if it's time to send another avionics packet */
	/* TODO */

	/* change analog values */
	for (int i = 0; i < VALVES_NUM; ++i) {
		get_random_analog(&sensors[i]);
		analogWrite(sensors[i].pin, sensors[i].val);
	}

	/* log values written */
	for (int i = 0; i < VALVES_NUM; ++i) {
		Serial.print(sensors[i].val);
		Serial.print(", ");
	}
	Serial.print("\n");

	/* delay */
	delay(200);
}
