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
struct Frame {
	char destination_addr;
	char source_addr;
	char payload_len;
	char payload;
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
		twobytes nn_rx_rcrc;
		enum reply_byte nn_rx_reply;
	} nn_rx_state;
	struct frames_ll {
		struct frames_ll* next;
		struct Frame* frame;
	}* nn_frames;
	Task* nn_taskReadBits;
	Task* nn_taskWait;
	Task* nn_taskReceive;
	Task* nn_taskReply;
	StatusRequest* nn_new_bit;
	StatusRequest* nn_new_frame;
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
	if(nn_globalScheduler == NULL) { return {errno::NO_SCHEDULER}; }
	nn->nn_new_bit = new StatusRequest();
	nn->nn_new_frame = new StatusRequest();
	Task* read_bit_task = new Task(
		0, TASK_FOREVER,
		&read_bit::onrun, nn_globalScheduler, false,
		&read_bit::onenable, &read_bit::ondisable
	);
	read_bit_task->setLtsPointer(nn);
	Task* wait_task = new Task(&rxframe_fsm::waiting, nn_globalScheduler);
	wait_task->setLtsPointer(nn);
	Task* receive_task = new Task(&rxframe_fsm::header, nn_globalScheduler);
	receive_task->setLtsPointer(nn);
	Task* reply_task = new Task(&rxframe_fsm::reply, nn_globalScheduler);
	reply_task->setLtsPointer(nn);
	nn->nn_taskReadBits = read_bit_task;
	nn->nn_taskWait = wait_task;
	nn->nn_taskReceive = receive_task;
	nn->nn_taskReply = reply_task;
	return Ok();
}
void nn_receive(NanoNet* nn) {
	if(nn_globalScheduler == NULL) { return {errno::NO_SCHEDULER}; }
	nn->nn_taskReadBits->enable();
}
void nn_frames_append(NanoNet* nanonet, struct NanoNet::frames_ll* frame) {
	if(nanonet->nn_frames == NULL) {
		nanonet->nn_frames = frame;
	} else {
		struct NanoNet::frames_ll* last = nanonet->nn_frames;
		while(last->next != NULL) {
			last = last->next;
		}
		last->next = frame;
	}
}
void nn_unload_frame(NanoNet* nn) {
	NanoNet::NNReceiveState* rx_state = &(nn->nn_rx_state);
	if(rx_state->nn_rx_payload_buf != NULL) {
		struct Frame* frame = malloc(rx_state->nn_rx_length + sizeof(Frame));
		struct NanoNet::frames_ll* frame_ll = malloc(sizeof(NanoNet::frames_ll));
		frame->destination_addr = rx_state->nn_rx_dest;
		frame->source_addr = rx_state->nn_rx_source;
		frame->payload_len = rx_state->nn_rx_length;
		memcpy(&(frame->payload), rx_state->nn_rx_payload_buf, rx_state->nn_rx_length+1);
		free(rx_state->nn_rx_payload_buf);
		rx_state->nn_rx_payload_buf = NULL;
		frame_ll->next = NULL;
		frame_ll->frame = frame;
		nn_frames_append(nn, frame_ll);
	}
}
struct Frame * nn_pop_frame(NanoNet* nn) {
	if(nn->nn_frames == NULL) {
		return NULL;
	} else {
		struct Frame* frame = nn->nn_frames->frame;
		struct NanoNet::frames_ll* next = nn->nn_frames->next;
		free(nn->nn_frames);
		nn->nn_frames = next;
		return frame;
	}
}
bool read_bit::onenable() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	nanonet->nn_state = NanoNet::State::RX_WAITING;
	nanonet->nn_rx_state.nn_rx_bit_pos = 0;
	nanonet->nn_rx_state.nn_rx_byte_pos = 0;
	nanonet->nn_rx_state.nn_rx_dest = 0;
	nanonet->nn_rx_state.nn_rx_source = 0;
	nanonet->nn_rx_state.nn_rx_length = 0;
	nanonet->nn_rx_state.nn_rx_options = 0;
	nanonet->nn_rx_state.nn_rx_crc.clearCrc();
	nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
	nanonet->nn_new_bit->setWaiting(1);
	nanonet->nn_taskWait->waitFor(nanonet->nn_new_bit);
	pinMode(nanonet->nn_clock_pin, INPUT);
	pinMode(nanonet->nn_data_pin, INPUT);
	return true;
}
void read_bit::ondisable() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	nanonet->nn_state = NanoNet::State::IDLE;
	nanonet->nn_rx_state.nn_rx_last_clock = LOW;
	nanonet->nn_rx_state.nn_rx_buf.s = 0;
	nn_unload_frame(nanonet);
	nanonet->nn_new_frame->signal();
	rxframe_fsm::reset(nanonet);
	nanonet->nn_taskReadBits->enable();
}
void read_bit::onrun() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	NanoNet::State nn_state = nanonet->nn_state;
	bool clock = digitalRead(nanonet->nn_clock_pin);
	if(clock != rx_state->nn_rx_last_clock) {
		rx_state->nn_rx_last_clock = clock;
		if(nanonet->nn_state == NanoNet::State::RX_REPLY) {
			if(clock == LOW) {
				rx_state->nn_rx_bit_pos += 1;
				nanonet->nn_new_bit->signal();
			}
		} else {
			if(clock == HIGH) {
				bool rx_bit = digitalRead(nanonet->nn_data_pin) & 0x01;
				rx_state->nn_rx_buf.s = rx_state->nn_rx_buf.s << 1;
				rx_state->nn_rx_buf.s |= rx_bit;
				rx_state->nn_rx_bit_pos += 1;
				rx_state->nn_rx_bit_pos %= 8;
				rx_state->nn_rx_byte_pos += rx_state->nn_rx_bit_pos == 0;
				nanonet->nn_new_bit->signal();
			}
		}
	}
}

void rxframe_fsm::reset(NanoNet* nanonet) {
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
	nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
	nanonet->nn_new_bit->setWaiting(1);
	nanonet->nn_taskWait->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::waiting() {
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
		if(nanonet->nn_rx_state.nn_rx_payload_buf != NULL) {
			free(nanonet->nn_rx_state.nn_rx_payload_buf);
			nanonet->nn_rx_state.nn_rx_payload_buf = NULL;
		}
		nanonet->nn_new_bit->setWaiting(8);
		nanonet->nn_taskReceive->waitFor(nanonet->nn_new_bit);
		nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
	} else {
		nanonet->nn_new_bit->setWaiting(1);
		nanonet->nn_taskWait->waitFor(nanonet->nn_new_bit);
	}
}
void rxframe_fsm::header() {
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
			rx_state->nn_rx_payload_buf = malloc(rx_state->nn_rx_length+1);
		} else {
			_LOG_WARN(F("Received unknown header byte: "));
			IFWARN(printb(curbyte));
			LOG_WARN();
			rx_state->nn_rx_crc.updateCrc(curbyte);
		}
		break;
	}
	nanonet->nn_new_bit->setWaiting(8);
	nanonet->nn_taskReceive->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::payload() {
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
	nanonet->nn_new_bit->setWaiting(8);
	nanonet->nn_taskReceive->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::crc() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	char byte_pos = rx_state->nn_rx_byte_pos;
	char curbyte = rx_state->nn_rx_buf.b[0];
	if(bit_pos != 0) { return; }
	switch(byte_pos) {
		case 1:
			_LOG_BYTE(curbyte);
			rx_state->nn_rx_rcrc.b[0] = curbyte;
			break;
		case 2:
			_LOG_BYTE(curbyte);
			rx_state->nn_rx_rcrc.b[1] = curbyte;
			LOG_WARN();
			twobytes comp_crc;
			comp_crc.s = rx_state->nn_rx_crc.getCrc();
			_LOG_TRACE(F("Computed CRC: "));
			_LOG_BYTE(comp_crc.b[0]);
			LOG_BYTE(comp_crc.b[1]);
			if(rx_state->nn_rx_options & OptRACK) {
				if(comp_crc.s != rx_state->nn_rx_rcrc.s) {
					LOG_WARN(F("Bad data, Sending NACK"));
					rx_state->nn_rx_reply = NACK;
				} else {
					LOG_WARN(F("Success, Sending ACK"));
					rx_state->nn_rx_reply = ACK;
				}
				nanonet->nn_state = NanoNet::State::RX_REPLY;
				rx_state->nn_rx_bit_pos = -1;
				rx_state->nn_rx_byte_pos = 0;
				pinMode(nanonet->nn_data_pin, OUTPUT);
				nanonet->nn_new_bit->setWaiting(1);
				nanonet->nn_taskReply->waitFor(nanonet->nn_new_bit);
				return;
			}
			break;
		case 3:
			if(curbyte == 0x04) {
				LOG_WARN(F("End of Transmission"));
				if(rx_state->nn_rx_crc.getCrc() != rx_state->nn_rx_rcrc.s) {
					LOG_FATAL(F("Bad data"));
				} else {
					LOG_INFO(F("Success"));
				}
				nanonet->nn_state = NanoNet::State::IDLE;
				nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
				nanonet->nn_taskReadBits->disable();
				nn_unload_frame(nanonet);
				return;
			}
			break;
	}
	nanonet->nn_new_bit->setWaiting(8);
	nanonet->nn_taskReceive->waitFor(nanonet->nn_new_bit);
}
void rxframe_fsm::reply() {
	Task* curtask = nn_globalScheduler->getCurrentTask();
	NanoNet* nanonet = curtask->getLtsPointer();
	NanoNet::NNReceiveState* rx_state = &(nanonet->nn_rx_state);
	char bit_pos = rx_state->nn_rx_bit_pos;
	bool reply_bit = rx_state->nn_rx_reply >> (7-bit_pos) & 0x01;
	if(rx_state->nn_rx_bit_pos < 8) {
		digitalWrite(nanonet->nn_data_pin, reply_bit);
		nanonet->nn_new_bit->setWaiting(1);
		nanonet->nn_taskReply->waitFor(nanonet->nn_new_bit);
	} else {
		pinMode(nanonet->nn_data_pin, INPUT);
		nanonet->nn_state = NanoNet::State::IDLE;
		nanonet->nn_taskReceive->setCallback(&rxframe_fsm::header);
		nanonet->nn_taskReadBits->disable();
		nn_unload_frame(nanonet);
	}
}

} // End Namespace
#endif
