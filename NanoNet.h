/**
 * @author      : Ezra Barrow (barrow@tilde.team)
 * @file        : NanoNet
 * @created     : Monday May 24, 2021 18:11:14 EDT
 *
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

#ifndef NANONET_H
#define NANONET_H

#include "Definitions.h"
#include "Logging.h"
#include "Crc16.h"
#include <TaskSchedulerSleepMethods.h>
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>

namespace NN {

enum struct errno: char {
	SUCCESS = 0,
	NO_SCHEDULER = 1,
	SCHEDULER_ALREADY_SET = 2,
};
typedef struct {
	enum errno errno;
} result_t;

enum frame_opts: char {
	OptRACK    = 1 << 6,
	OptHEADLEN = 1 << 5,
};

enum reply_byte: char {
	ACK  = 0x06,
	NACK = 0x15,
};

struct NanoNet {
	enum struct State: char {
		IDLE       = 0x00,
		RX_WAITING = 0x10,
		RX_HEADER  = 0x11,
		RX_PAYLOAD = 0x12,
		RX_CRC     = 0x13,
		RX_REPLY   = 0x14,
		TX         = 0x20,
	} nn_state = State::IDLE;
	short nn_tx_rate = NN_TX_RATE;
	short nn_ca_rate = 4;
	short nn_cd_rate = 4;
	char nn_max_tries = 5;
	char nn_address = MY_ADDR;
	char nn_clock_pin = 2;
	char nn_data_pin = 4;
	struct NNReceiveState {
		char nn_rx_last_clock;
		char nn_rx_byte_pos = 0;
		char nn_rx_bit_pos = 0;
		twobytes nn_rx_buf;
		char nn_rx_dest;
		char nn_rx_source;
		char nn_rx_length;
		enum frame_opts nn_rx_options;
		char* nn_rx_payload_buf;
		Crc16 nn_rx_crc;
		short nn_rx_rcrc;
		enum reply_byte nn_rx_reply;
	} nn_rx_state;
	Task* nn_taskReadBits;
	Task* nn_taskWait;
	Task* nn_taskReceive;
	Task* nn_taskReply;
	StatusRequest* nn_new_bit;
	StatusRequest* nn_new_byte;
};
result_t nn_setup(Scheduler* ts);
result_t nn_init(NanoNet* nn);
namespace read_bit {
	bool onenable();
	void onrun();
	void ondisable();
}
namespace rxframe_fsm {
	void reset(NanoNet* nanonet);
	void waiting();
	void header();
	void payload();
	void crc();
	bool replyOE();
	void reply();
	void replyOD();
}

Scheduler* nn_globalScheduler;
#define NN_ASSERT_HASSCHED() if(nn_globalScheduler == NULL) { return {errno::NO_SCHEDULER}; }
inline result_t Ok() { return {errno::SUCCESS}; }

result_t nn_setup(Scheduler* ts) {
	if(nn_globalScheduler == NULL) {
		nn_globalScheduler = ts;
		return {errno::SUCCESS};
	} else {
		return {errno::SCHEDULER_ALREADY_SET};
	}
}
result_t nn_init(NanoNet* nn) {
	NN_ASSERT_HASSCHED();
	Task* wait_task = new Task(
		PER_SECOND(nn->nn_tx_rate), TASK_ONCE,
		&rxframe_fsm::waiting, nn_globalScheduler, false
	);
	wait_task->setLtsPointer(nn);
	Task* receive_task = new Task(
		PER_SECOND(nn->nn_tx_rate), TASK_ONCE,
		&rxframe_fsm::header, nn_globalScheduler, false
	);
	receive_task->setLtsPointer(nn);
	Task* read_bit_task = new Task(
		0, TASK_FOREVER,
		&read_bit::onrun, nn_globalScheduler, false,
		&read_bit::onenable, &read_bit::ondisable
	);
	read_bit_task->setLtsPointer(nn);
	Task* reply_task = new Task(
		PER_SECOND(nn->nn_tx_rate), 8,
		&rxframe_fsm::reply, nn_globalScheduler, false,
		&rxframe_fsm::replyOE, &rxframe_fsm::replyOD
	);
	reply_task->setLtsPointer(nn);
	StatusRequest* new_bit = new StatusRequest();
	StatusRequest* new_byte = new StatusRequest();
	nn->nn_taskReadBits = read_bit_task;
	nn->nn_taskWait = wait_task;
	nn->nn_taskReceive = receive_task;
	nn->nn_taskReply = reply_task;
	nn->nn_new_bit = new_bit;
	nn->nn_new_byte = new_byte;
	return Ok();
}
void nn_receive(NanoNet* nn) {
	nn->nn_taskReadBits->enable();
}
bool read_bit::onenable() {
	LOG_FATAL(F("read_bit::onenable()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	rxframe_fsm::reset(nanonet);
	pinMode(nanonet->nn_clock_pin, INPUT);
	pinMode(nanonet->nn_data_pin, INPUT);
	return true;
}
void read_bit::ondisable() {
	LOG_FATAL(F("read_bit::ondisable()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	rxframe_fsm::reset(nanonet);
	nanonet->nn_state = NanoNet::State::IDLE;
	nanonet->nn_rx_state.nn_rx_last_clock = LOW;
	nanonet->nn_rx_state.nn_rx_buf.s = 0;
	nanonet->nn_taskReceive->disable();
}
void read_bit::onrun() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	if(curtask->isFirstIteration()) { LOG_FATAL(F("read_bit::onrun()")); }
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	NanoNet::State nn_state = nanonet->nn_state;
	bool clock = digitalRead(nanonet->nn_clock_pin);
	if(clock != rx_state->nn_rx_last_clock) {
		rx_state->nn_rx_last_clock = clock;
		if(clock == HIGH) {
			bool rx_bit = digitalRead(nanonet->nn_data_pin) & 0x01;
			rx_state->nn_rx_buf.s = rx_state->nn_rx_buf.s << 1;
			rx_state->nn_rx_buf.s |= rx_bit;
			rx_state->nn_rx_bit_pos += 1;
			rx_state->nn_rx_bit_pos %= 8;
			rx_state->nn_rx_byte_pos += rx_state->nn_rx_bit_pos == 0;
			if(nn_state == NanoNet::State::RX_WAITING) {
				nanonet->nn_new_bit->signal();
				return;
			} else if(nn_state != NanoNet::State::RX_REPLY) {
				nanonet->nn_taskReceive->restart();
			}
		}
		if(clock == LOW && nanonet->nn_state == NanoNet::State::RX_REPLY) {
			rx_state->nn_rx_bit_pos += 1;
			nanonet->nn_taskReply->forceNextIteration();
			return;
		}
	}
}

void rxframe_fsm::reset(NanoNet* nanonet) {
	LOG_FATAL(F("rxframe_fsm::reset()"));
	nanonet->nn_state = NanoNet::State::RX_WAITING;
	nanonet->nn_rx_state.nn_rx_bit_pos = 0;
	nanonet->nn_rx_state.nn_rx_byte_pos = 0;
	nanonet->nn_rx_state.nn_rx_dest = 0;
	nanonet->nn_rx_state.nn_rx_source = 0;
	nanonet->nn_rx_state.nn_rx_length = 0;
	nanonet->nn_rx_state.nn_rx_options = 0;
	if(nanonet->nn_rx_state.nn_rx_payload_buf != NULL) {
		free(nanonet->nn_rx_state.nn_rx_payload_buf);
		nanonet->nn_rx_state.nn_rx_payload_buf = NULL;
	}
	nanonet->nn_rx_state.nn_rx_crc.clearCrc();
	nanonet->nn_taskReceive->setCallback(&rxframe_fsm::waiting);
	nanonet->nn_new_bit->setWaiting(1);
	nanonet->nn_taskWait->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::waiting() {
	LOG_FATAL(F("rxframe_fsm::waiting()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	if(nanonet->nn_rx_state.nn_rx_buf.s == 0xFF01) {
		_LOG_WARN(F("Entered Frame"));
		_LOG_TRACE(F(": "));
		_LOG_BYTE(nanonet->nn_rx_state.nn_rx_buf.b[1]);
		_LOG_BYTE(nanonet->nn_rx_state.nn_rx_buf.b[0]);
		LOG_WARN();
		nanonet->nn_state = NanoNet::State::RX_HEADER;
		nanonet->nn_rx_state.nn_rx_bit_pos = 0;
		nanonet->nn_rx_state.nn_rx_byte_pos = 0;
		nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
	}
	nanonet->nn_new_bit->setWaiting(1);
	nanonet->nn_taskWait->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::header() {
	LOG_FATAL(F("rxframe_fsm::header()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	char byte_pos = rx_state->nn_rx_byte_pos;
	char curbyte = rx_state->nn_rx_buf.b[0];
	if(bit_pos != 0) { return; }
	switch(byte_pos) {
	case 1:
		_LOG_INFO(F("Received Destination Address"));
		_LOG_TRACE(F(": "));
		_LOG_BYTE(curbyte);
		LOG_INFO();
		if(curbyte == nanonet->nn_address) {
			rx_state->nn_rx_dest = curbyte;
			rx_state->nn_rx_crc.updateCrc(curbyte);
		} else {
			LOG_INFO(F("Message not for us, resetting"));
			rxframe_fsm::reset(nanonet);
		}
		break;
	case 2:
		_LOG_INFO(F("Received Source Address"));
		_LOG_TRACE(F(": "));
		_LOG_BYTE(curbyte);
		LOG_INFO();
		rx_state->nn_rx_source = curbyte;
		rx_state->nn_rx_crc.updateCrc(curbyte);
		break;
	case 3:
		_LOG_INFO(F("Received Length Byte"));
		_LOG_TRACE(F(": "));
		_LOG_BYTE(curbyte);
		LOG_INFO();
		rx_state->nn_rx_length = curbyte;
		rx_state->nn_rx_crc.updateCrc(curbyte);
		break;
	case 4:
		_LOG_INFO(F("Received Options Byte"));
		_LOG_TRACE(F(": "));
		_LOG_BYTE(curbyte);
		LOG_INFO();
		rx_state->nn_rx_options = curbyte;
		rx_state->nn_rx_crc.updateCrc(curbyte);
		if(curbyte & OptRACK) {
			LOG_INFO(F("RACK bit set"));
		} else {
			LOG_INFO(F("RACK bit unset"));
		}
		break;
	//TODO: implement OptHEADLEN
	default:
		if(curbyte == 0x02) {
			LOG_WARN(F("Received Header"));
			nanonet->nn_state = NanoNet::State::RX_PAYLOAD;
			rx_state->nn_rx_bit_pos = 0;
			rx_state->nn_rx_byte_pos = -1;
			nanonet->nn_taskReceive->setCallback(&rxframe_fsm::payload);
			_LOG_WARN(F("Receiving payload"));
			rx_state->nn_rx_payload_buf = malloc(rx_state->nn_rx_length);
		} else {
			_LOG_WARN(F("Received unknown header byte: "));
			IFWARN(printb(curbyte));
			LOG_WARN();
			rx_state->nn_rx_crc.updateCrc(curbyte);
		}
		break;
	}
}
void rxframe_fsm::payload() {
	LOG_FATAL(F("rxframe_fsm::payload()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	char byte_pos = rx_state->nn_rx_byte_pos;
	char curbyte = rx_state->nn_rx_buf.b[0];
	if(bit_pos != 0) { return; }
	if(byte_pos > rx_state->nn_rx_length) {
		LOG_WARN(F("\n\r Packet longer than length, cancelling"));
		rxframe_fsm::reset(nanonet);
	} else if(byte_pos == rx_state->nn_rx_length && curbyte == 0x03) {
		LOG_WARN(F("\n\rEnd of Text"));
		rx_state->nn_rx_payload_buf[byte_pos] = 0;
		nanonet->nn_state = NanoNet::State::RX_CRC;
		rx_state->nn_rx_bit_pos = 0;
		rx_state->nn_rx_byte_pos = 0;
		nanonet->nn_taskReceive->setCallback(&rxframe_fsm::crc);
		_LOG_WARN(F("Receiving CRC"));
		_LOG_TRACE(F(": "));
	} else {
		_LOG_INFO(F("."));
		rx_state->nn_rx_payload_buf[byte_pos] = curbyte;
		rx_state->nn_rx_crc.updateCrc(curbyte);
	}
}
void rxframe_fsm::crc() {
	LOG_FATAL(F("rxframe_fsm::crc()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	char byte_pos = rx_state->nn_rx_byte_pos;
	char curbyte = rx_state->nn_rx_buf.b[0];
	_LOG_FATAL(F("bit_pos: "));
	_LOG_FATAL(bit_pos, DEC);
	_LOG_FATAL(F(", byte_pos: "));
	LOG_FATAL(byte_pos, DEC);
	if(bit_pos != 0) { return; }
	switch(byte_pos) {
	case 0:
		_LOG_BYTE(curbyte);
		IFFATAL(printb(curbyte));
		break;
	case 1:
		_LOG_BYTE(curbyte);
		IFFATAL(printb(curbyte));
		LOG_WARN();
		short recv_crc = rx_state->nn_rx_buf.s;
		rx_state->nn_rx_rcrc = recv_crc;
		short comp_crc = rx_state->nn_rx_crc.getCrc();
		_LOG_TRACE(F("Computed CRC: "));
		_LOG_BYTE(comp_crc.b[0]);
		_LOG_BYTE(comp_crc.b[1]);
		if(rx_state->nn_rx_options & OptRACK) {
			if(comp_crc != recv_crc) {
				LOG_WARN(F("Bad data, Sending NACK"));
				rx_state->nn_rx_reply = NACK;
			} else {
				LOG_WARN(F("Success, Sending ACK"));
				rx_state->nn_rx_reply = ACK;
			}
			nanonet->nn_taskReply->enable();
		}
		break;
	default:
		if(curbyte == 0x04) {
			LOG_WARN(F("End of Transmission"));
			if(rx_state->nn_rx_crc.getCrc() != rx_state->nn_rx_rcrc) {
				LOG_FATAL(F("Bad data"));
			} else {
				LOG_INFO(F("Success"));
			}
		}
		break;
	}
}
bool rxframe_fsm::replyOE() {
	LOG_FATAL(F("rxframe_fsm::replyOE()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	nanonet->nn_state = NanoNet::State::RX_REPLY;
	rx_state->nn_rx_bit_pos = -1;
	rx_state->nn_rx_byte_pos = 0;
	pinMode(nanonet->nn_data_pin, OUTPUT);
}
void rxframe_fsm::reply() {
	LOG_FATAL(F("rxframe_fsm::reply()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	if(curtask->isFirstIteration()) {
	} else {
	}
}
void rxframe_fsm::replyOD() {
	LOG_FATAL(F("rxframe_fsm::replyOD()"));
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	char byte_pos = rx_state->nn_rx_byte_pos;
	char curbyte = rx_state->nn_rx_buf.b[0];
}

} // End Namespace
#endif
