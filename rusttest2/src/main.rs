#![no_std]
#![no_main]
#![feature(abi_avr_interrupt)]

use arduino_uno::prelude::*;
use core::cell;
use core::str::from_utf8;
use panic_halt as _;

use arduino_uno::hal::port;
static mut NIC0_DATA: Option<port::Pin<port::mode::Input<port::mode::Floating>>> = None;

static NN_rx_buf: avr_device::interrupt::Mutex<cell::Cell<u16>> = avr_device::interrupt::Mutex::new(cell::Cell::new(0));
static NN_new_bit: avr_device::interrupt::Mutex<cell::Cell<bool>> = avr_device::interrupt::Mutex::new(cell::Cell::new(false));

const NN_START_OF_HEADER: u8 = 0x01;
const NN_START_OF_TEXT: u8 = 0x02;
const NN_END_OF_TEXT: u8 = 0x03;
const NN_END_OF_TRANS: u8 = 0x04;

enum State {
    IDLE,
    RX_WAITING,
    RX_HEADER,
    RX_PAYLOAD,
    RX_CRC,
}

struct NanoNet {
    clock_pin: Option<port::portd::PD2<port::mode::Input<port::mode::Floating>>>,
    // data_pin: port::portd::PD4<port::mode::Input<port::mode::Floating>>,
    tx_rate: u8,
    ca_rate: u8,
    cd_rate: u8,
    address: u8,
    state: State,
}
impl NanoNet {
    fn new(clock_pin: port::portd::PD2<port::mode::Input<port::mode::Floating>>, address: u8) -> Self {
        Self { clock_pin: Some(clock_pin), tx_rate: 10, ca_rate: 4, cd_rate: 4, address, state: State::IDLE }
    }
    fn recieve_frame(&mut self, buf: &mut [u8], led_output: &mut port::Pin<port::mode::Output>) -> u8 {
        let mut buf_pos = 0;
        let mut bit_pos = 0;
        self.state = State::RX_WAITING;
        let mut destination = 0u8;
        let mut source = 0u8;
        let mut rx_crc = 0u16;
        loop {
            let new_bit = avr_device::interrupt::free(|cs| NN_new_bit.borrow(cs).get());
            if new_bit {
                avr_device::interrupt::free(|cs| NN_new_bit.borrow(cs).set(false));
                let buffer = avr_device::interrupt::free(|cs| NN_rx_buf.borrow(cs).get());
                let [highbuffer, lowbuffer] = buffer.to_be_bytes();
                match self.state {
                    State::IDLE => unreachable!(),
                    State::RX_WAITING => {
                        if highbuffer == 0xFF && lowbuffer == NN_START_OF_HEADER {
                            self.state = State::RX_HEADER;
                            bit_pos = 0;
                            buf_pos = 0;
                            //TODO: clear crc
                        }
                    }
                    State::RX_HEADER => {
                        bit_pos += 1;
                        if bit_pos == 16 {
                            // return (highbuffer != self.address) as u8;
                            if highbuffer != self.address {
                                self.state = State::RX_WAITING;
                            } else {
                                destination = highbuffer;
                                source = lowbuffer;
                                //TODO: update CRC with bytes
                            }
                        } else if bit_pos > 16 && bit_pos % 8 == 0 {
                            if lowbuffer == NN_START_OF_TEXT {
                                self.state = State::RX_PAYLOAD;
                                bit_pos = 0;
                            } else {
                                //TODO: add header crc
                            }
                        }
                    }
                    State::RX_PAYLOAD => {
                        bit_pos += 1;
                        if bit_pos == 8 {
                            bit_pos = 0;
                            if lowbuffer == NN_END_OF_TEXT {
                                buf[buf_pos] = 0x00;
                                self.state = State::RX_CRC;
                                buf_pos = 0;
                            } else {
                                buf[buf_pos] = lowbuffer;
                                buf_pos += 1;
                                //TODO: update crc with byte
                            }
                        }
                    }
                    State::RX_CRC => {
                        bit_pos += 1;
                        if bit_pos == 8 {
                            rx_crc = lowbuffer as u16;
                        } else if bit_pos == 16 {
                            rx_crc |= (highbuffer as u16) << 8;
                        } else if bit_pos == 24 && lowbuffer == NN_END_OF_TRANS {
                            //TODO: check crc
                            break
                        }
                    }
                }
            }
        }
        self.state = State::IDLE;
        source
    }
}

#[avr_device::interrupt(atmega328p)]
unsafe fn INT0() {
    avr_device::interrupt::free(|cs| {
        if let Some(pin) = NIC0_DATA.as_mut() {
            let rx_bit = pin.is_high().unwrap();
            let rx_buf_cell = NN_rx_buf.borrow(cs);
            let mut rx_buf = rx_buf_cell.get() << 1;
            if rx_bit {
                rx_buf |= 0x01;
            }
            rx_buf_cell.set(rx_buf);
            let new_bit_cell = NN_new_bit.borrow(cs);
            new_bit_cell.set(true);
        }
    })
}

#[arduino_uno::entry]
fn main() -> ! {
    let dp = arduino_uno::Peripherals::take().unwrap();

    let mut pins = arduino_uno::Pins::new(dp.PORTB, dp.PORTC, dp.PORTD);
    let mut serial = arduino_uno::Serial::new(
        dp.USART0,
        pins.d0,
        pins.d1.into_output(&mut pins.ddr),
        9600.into_baudrate(),
    );

    let mut clock_pin = pins.d2;
    let mut data_pin = pins.d4.into_floating_input(&mut pins.ddr);
    unsafe {
        NIC0_DATA = Some(data_pin.downgrade());
    }
    let ei = dp.EXINT;
    // w.isc0() is INT0, w.isc1() is INT1
    // Values are listed at:
    // https://rahix.github.io/avr-hal/arduino_uno/pac/exint/eicra/struct.ISC0_W.html
    ei.eicra.write(|w| w.isc0().val_0x03());
    // w.int0() and w.int1() are the two interrupts
    ei.eimsk.write(|w| w.int0().bit(true));
    unsafe {
        avr_device::interrupt::enable();
    }

    let mut nanonet = NanoNet::new(clock_pin.into_floating_input(&mut pins.ddr), 0x01);

    let mut led = pins.d13.into_output(&mut pins.ddr).downgrade();
    ufmt::uwriteln!(&mut serial, "Started\r").void_unwrap();

    loop {
        ufmt::uwriteln!(&mut serial, "Looping\r").void_unwrap();
        let mut message_buf: [u8; 64] = [b'\0'; 64];
        let src = nanonet.recieve_frame(&mut message_buf, &mut led);
        let msg = from_utf8(&message_buf).unwrap();
        ufmt::uwriteln!(&mut serial, "Message recieved from {}: {}\r", src, msg).void_unwrap();
    }
    // led.set_low().void_unwrap();
    // let mut buf_pos = 0;
    // let mut checking = false;
    // loop {
    //     let new_bit = avr_device::interrupt::free(|cs| NN_new_bit.borrow(cs).get());
    //     let buffer = avr_device::interrupt::free(|cs| NN_rx_buf.borrow(cs).get());
    //     let highbuffer = buffer & 0xFF00;
    //     let lowbuffer = buffer & 0x00FF;
    //     if checking {
    //         if new_bit {
    //             avr_device::interrupt::free(|cs| NN_new_bit.borrow(cs).set(false));
    //             buf_pos = buf_pos + 1;
    //             if buf_pos == 16 {
    //                 if lowbuffer == 0x04 {
    //                     led.set_low().void_unwrap();
    //                     checking = false;
    //                 }
    //             }
    //         }
    //     }
    //     if buffer == 0xFF01 {
    //         led.set_high().void_unwrap();
    //     } else if highbuffer == 0x0300 {
    //         checking = true;
    //         buf_pos = 0;
    //     }
    // }
}
