/*************************************************************************
 * Title:    I2C master library using hardware TWI interface
 * Author:   Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
 * File:     $Id: twimaster.c,v 1.3 2005/07/02 11:14:21 Peter Exp $
 * Software: AVR-GCC 3.4.3 / avr-libc 1.2.3
 * Target:   any AVR device with hardware TWI
 * Usage:    API compatible with I2C Software Library i2cmaster.h
 **************************************************************************/
#include <inttypes.h>
#include <compat/twi.h>

/* I2C clock in Hz */
#define SCL_CLOCK  100000L

/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/
void i2c_init(void);
unsigned char i2c_start(unsigned char address);
void i2c_start_wait(unsigned char address);
unsigned char i2c_rep_start(unsigned char address);
void i2c_stop(void);
unsigned char i2c_write( unsigned char data );
unsigned char i2c_readAck(void);
unsigned char i2c_readNak(void);
