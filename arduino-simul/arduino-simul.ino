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


#include "rocket_packet.h"


/* time in ms between avionic data transfers */
#define AVIONICS_PERIOD 10

#define pi 3.14159265f

/* timestamp to check if it's time to send another avionics data packet */
double start;

/* analog pins simulating sensors */
#define SENSORS_NUM 5

typedef struct {
	int pin;
	unsigned char val;
	unsigned char mu;
	unsigned char sigma;
} sensor_sim_t;

sensor_sim_t sensors[SENSORS_NUM] = {
	{.pin = 2,	.val = 0,	.mu = 10,	.sigma = 1},
	{.pin = 3,	.val = 0,	.mu = 60,	.sigma = 10},
	{.pin = 4,	.val = 0,	.mu = 50,	.sigma = 2},
	{.pin = 5,	.val = 0,	.mu = 150,	.sigma = 4},
	{.pin = 6,	.val = 0,	.mu = 30,	.sigma = 5},
};


/* simulated avionics data packet */
AvionicsData avionics_data;
unsigned char data_buf[AVIONICS_DATA_SIZE];


/*
 * Generate random values on a normal distribution using the 
 * Box-Muller method
 */
float get_normal(float mu, float sigma) {
	float u1;
	float u2;
	float z0;

	/* generate random value z0 in Normal(0, 1) distribution */
	u1 = 0.001 * random(0, 1000);
	u2 = 0.001 * random(0, 1000);
	z0 = sqrt(-2 * log(u1)) * cos(2 * pi * u2);

	/* normalize distribution to Normal(mu, sigma^2) */
	return (z0 * sigma) + mu;
}


void get_random_analog(sensor_sim_t* sensor) {
	float z0;

	z0 = get_normal(sensor->mu, sensor->sigma);

	/* clip range to fit in analog values */
	if (z0 < 0) {
		z0 = 0;
	} else if (z0 > 168) {
		z0 = 168;
	}

	sensor->val = (unsigned char) z0;
}


void fill_avionics_data(AvionicsData* data) {
	data->timestamp = millis();
	data->latitude = get_normal(46.781241, 1);
	data->longitude = get_normal(-71.273009, 1);
	data->NSIndicator = 'N';
	data->EWIndicator = 'W';
	data->UTCTime = data->timestamp + 1000;
	data->altitude = get_normal(3000, 50);
	data->pressure = get_normal(100, 3);
	data->temperature = get_normal(25, 5);
	data->acc_x_uncomp = get_normal(1000, 5);
	data->acc_x_uncomp = get_normal(50, 1);
	data->acc_x_uncomp = get_normal(30, 2);
	data->acc_x = get_normal(1, 1);
	data->acc_y = get_normal(0.05, 0.01);
	data->acc_z = get_normal(0.03, 0.02);
	data->mag_x = get_normal(100, 1);
	data->mag_y = get_normal(200, 2);
	data->mag_z = get_normal(300, 3);
	data->x_gyro = get_normal(4000, 40);
	data->y_gyro = get_normal(5000, 50);
	data->z_gyro = get_normal(6000, 60);
}


void setup(void) {
	/* init analog outputs */
	for (int i = 0; i < SENSORS_NUM; ++i) {
		pinMode(sensors[i].pin, OUTPUT);
	}

	/* init debug serial port */
	Serial.begin(115200);
	Serial.print("start\n");

	/* init avionics simulation serial port */
	Serial1.begin(115200);

	start = millis();
}


void loop(void) {
	/* check if it's time to send another avionics packet */
	if (millis() - start >= AVIONICS_PERIOD) {
		fill_avionics_data(&avionics_data);
		pack_avionics_data(&avionics_data, data_buf);
		Serial1.write(data_buf, AVIONICS_DATA_SIZE);
		start = millis();
	}

	/* change analog values */
	for (int i = 0; i < SENSORS_NUM; ++i) {
		get_random_analog(&sensors[i]);
		analogWrite(sensors[i].pin, sensors[i].val);
	}

	/* log values written */
	for (int i = 0; i < SENSORS_NUM; ++i) {
		Serial.print(sensors[i].val);
		Serial.print(", ");
	}
	Serial.print("\n");

	/* delay */
	delay(200);
}
