#include <Arduino.h>
#include <PID_v1.h>
#include <Encoder.h>
#include <string.h>
#include <string>
#include "MotorControl.h"

void encoder::initialize()
{
	pinMode(left_pin.A, INPUT);
	pinMode(left_pin.B, INPUT);
	pinMode(right_pin.A, INPUT);
	pinMode(right_pin.B, INPUT);
}

void encoder::Read()
{
	counter.right = rightEnc.read();
	counter.left = leftEnc.read();
}
void encoder::Reset()
{
	rightEnc.write(0);
	leftEnc.write(0);
}

const pin_2 encoder::left_pin = {21, 20};
const pin_2 encoder::right_pin = {19, 18};
decltype(encoder::counter) encoder::counter;

Encoder encoder::rightEnc(encoder::right_pin.A, encoder::right_pin.B);
Encoder encoder::leftEnc(encoder::left_pin.A, encoder::left_pin.B);

void Motor::PID_initialize()
{
	leftPID.SetMode(AUTOMATIC);
	leftPID.SetSampleTime(timePeriod);
	leftPID.SetOutputLimits(-100, 100);

	rightPID.SetMode(AUTOMATIC);
	rightPID.SetSampleTime(timePeriod);
	rightPID.SetOutputLimits(-100, 100);
}
void Motor::initialize()
{
	pinMode(left_pin.A, OUTPUT);
	pinMode(left_pin.B, OUTPUT);

	pinMode(right_pin.A, OUTPUT);
	pinMode(right_pin.B, OUTPUT);

	encoder::initialize();
	PID_initialize();
}

void Motor::setPWM(int pwm, bool isRight)
{
	pwm = std::min(pwm, 255);
	pwm = std::max(pwm, -255);
	if (isDebug)
	{
		Serial.print(String("setting ") + (isRight ? "right" : "left") + String(" pwm : "));
		Serial.println(pwm);
	}
	pin_2 pin = isRight ? right_pin : left_pin;
	if (pwm > 0)
	{
		analogWrite(pin.A, pwm);
		analogWrite(pin.B, 0);
	}
	else
	{
		analogWrite(pin.A, 0);
		analogWrite(pin.B, -pwm);
	}
}

void Motor::PID_compute()
{
	leftPID.Compute();
	rightPID.Compute();
}

void Motor::updateSpeed()
{
	encoder::Read();
	PID_compute();
	setPWM(estimatePWM(targetSpeed) + rightOutput, true);
	setPWM(estimatePWM(targetSpeed) + leftOutput, false);
	encoder::Reset();
}

double Motor::estimatePWM(double targeteSpeed)
{
	return 4.5 * targeteSpeed;
}

const pin_2 Motor::left_pin = {12, 13};
const pin_2 Motor::right_pin = {6, 7};
bool Motor::isDebug = false;

const int Motor::timePeriod = 50;
// double basePWM = 0;
double Motor::rightOutput = 0;
double Motor::leftOutput = 0;
double Motor::targetSpeed = 0;

PID Motor::rightPID = PID(&encoder::counter.right, &rightOutput, &targetSpeed, 1, 0.00001, 0.00001, DIRECT);
PID Motor::leftPID = PID(&encoder::counter.left, &leftOutput, &targetSpeed, 1, 0.00001, 0.00001, DIRECT);
