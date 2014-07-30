#include "twimaster.h"

#define  PCF_addr         0xA2    // PCF8563 address
#define  I2C_READ         1       // PCF8563 read
#define  I2C_WRITE        0       // PCF8563 write
#define  second_RTC       0x02
#define  minute_RTC       0x03
#define  hour_RTC         0x04
#define  day_RTC          0x05
#define  dayOfTheWeek_RTC 0x06
#define  month_RTC        0x07
#define  year_RTC         0x08

volatile unsigned int  BCD;
volatile unsigned char second;
volatile unsigned char oldSecond;
volatile unsigned char minute;
volatile unsigned char hour;
volatile unsigned char day;
volatile unsigned char dayOfTheWeek;
volatile unsigned char month;
volatile unsigned char year;

void read(unsigned char position);
void write(unsigned char value, unsigned char position);
void readSecond();
void readMinute();
void readHour();
void readYear();
void readDate();
void getTime();
