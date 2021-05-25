/*
 * THIS FILE IS LICENSED UNDER THE FOLLOWING TERMS
 *
 * this code may not be used for any purpose. be gay, do crime
 *
 * THE FOLLOWING MESSAGE IS NOT A LICENSE
 *
 * <barrow@tilde.team> wrote this file.
 * by reading this message, you are reading "TRANS RIGHTS".
 * this file and the content within it is the queer agenda.
 * if we meet some day, and you think this stuff is worth it,
 * you can buy me a beer, tea, or something stronger.
 * -Ezra Barrow
 */

#define _TASK_MICRO_RES
#define _TASK_LTS_POINTER
#define _TASK_STATUS_REQUEST
#include "Definitions.h"
#include "Logging.h"
#include <TaskSchedulerSleepMethods.h>
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
#include "NanoNet.h"

char *message = "Debug Message";
char *message1 = "Hello World!!!";
char *message2 = "Goodbye Non-world!!!";
char *message3 = ":3";
char **messageptr = &message2;

Scheduler ts;

void blink1CB();
Task tBlink1 (
	// Interval
	500 * TASK_MILLISECOND,
	// Iterations
	20,
	// Callback
	&blink1CB,
	// Scheduler
	&ts,
	// Enable
	false
);
void ticksCB();
Task tTicks(TASK_SECOND, 10, &ticksCB, &ts, false);
void print_frame();
Task tFrames(&print_frame, &ts);

NN::NanoNet nanonet;

void setup() {
	Serial.begin(9600);
	Serial.println(F("\n\rI'm "MY_ADDR_S"\n\r"));
	pinMode(13, OUTPUT);
	NN::nn_setup(&ts);
	NN::nn_init(&nanonet);
	NN::nn_receive(&nanonet);
	nanonet.nn_new_frame->setWaiting(3);
	tFrames.waitFor(nanonet.nn_new_frame, 0, 3);
	#ifdef RUN_TEST
	#include RUN_TEST
	#endif
}

void loop() {
	ts.execute();
}

bool LED_state = false;
void blink1CB() {
	if(tBlink1.isFirstIteration()) {
		_LOG_FATAL(millis(), DEC);
		LOG_FATAL(F(": Blink1"));
		LED_state = false;
	}
	if(LED_state) {
		/* LOG_FATAL(F("Turning LED off")); */
		digitalWrite(13, LOW);
		LED_state = false;
	} else {
		/* LOG_FATAL(F("Turning LED on")); */
		digitalWrite(13, HIGH);
		LED_state = true;
	}
	if(tBlink1.isLastIteration()) {
		digitalWrite(13, LOW);
	}
}

void ticksCB() {
	_LOG_FATAL(F("Millis: "));
	LOG_FATAL(millis(), DEC);
}

void print_frame() {
	if(ts.getCurrentTask()->isFirstIteration()) {
		Serial.println(F("\n\rFrames:\n\r"));
	}
	NN::Frame* frame = NN::nn_pop_frame(&nanonet);
	if(frame != NULL) {
		Serial.print(F("Destination: "));
		Serial.println(frame->destination_addr, DEC);
		Serial.print(F("Source: "));
		Serial.println(frame->source_addr, DEC);
		Serial.print(F("Payload Len: "));
		Serial.println(frame->payload_len, DEC);
		Serial.print(F("Payload: "));
		Serial.println(&(frame->payload));
		Serial.println();
	}
}
