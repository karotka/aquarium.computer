#include "rtc8563.h"

void read(unsigned char position) {
    i2c_start_wait(PCF_addr + I2C_WRITE);      // PCF8563 write
    i2c_write(position);                       // address
    i2c_rep_start(PCF_addr + I2C_READ);        // read from PCF8563
    BCD = i2c_readNak();
    i2c_stop();                                // stop
}

void write(unsigned char value, unsigned char position) {
    //  Prevede DEC do BCD
    BCD = ((((value / 10) << 4) & 0xF0) | ((value % 10) & 0x0F));
    i2c_start_wait(PCF_addr + I2C_WRITE);
    i2c_write(position);                         // vyber adresy( hodiny minuty atd)
    i2c_write(BCD);                              //  zapise BCD na vybranou adresu
    i2c_stop();         //  stop
}

// cte sekundy z RTC
void readSecond() {
    read(second_RTC);
    second = ((((BCD & 0b01111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
    if (second > 59) {
        second = 0;
    }
}

void readMinute() {
    read(minute_RTC);                                // cte minuty z RTC
    minute = ((((BCD & 0b01111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
    if (minute > 59) {
        minute = 0;
    }
}

void readHour() {
    read(hour_RTC);                                // cte hodiny z RTC
    hour = ((((BCD & 0b00111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
    if (hour > 23) {
        hour = 0;
    }
}

void readYear() {
    read(year_RTC);
    year = (((BCD & 0xf0) >> 4) * 10 + (BCD & 0x0f)); // read and format years
}

void readDate() {
    read(day_RTC);                                        // cte dny z RTC
    day = ((((BCD & 0b00111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));

    read(month_RTC);                                // cte mesice z RTC
    month = ((((BCD & 0b10011111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
}

void getTime() {
    readYear();
    readDate();

    read(dayOfTheWeek_RTC);                                      // cte tydny z RTC
    dayOfTheWeek = BCD & 0b00000111;

    readHour();
    readMinute();
    readSecond();
}
