#include    <avr/io.h>
#include    <stdio.h>
#include    <util/delay.h>
#include    <avr/interrupt.h>
#include    "twimaster.h"
//#include    "uart.h"

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

#define SEVEN_SEGMENT_PORT PORTD
#define SEVEN_SEGMENT_DDR DDRD

#define true 1
#define false 0

volatile uint8_t i = 0;

char buffer[30];

// promenny pro RTC
volatile unsigned int  BCD;
volatile unsigned char second;
volatile unsigned char oldSecond;
volatile unsigned char minute;
volatile unsigned char hour;
volatile unsigned char day;
volatile unsigned char dayOfTheWeek;
volatile unsigned char month;
volatile unsigned char year;
volatile unsigned char blinkDot;
volatile unsigned char show = 0;
volatile unsigned int set = 0;
volatile unsigned char blickCounter = 0;
volatile uint8_t digits[4];

enum {set_none = 0, set_hour, set_minute, set_day, set_month, set_year};
enum {show_time = 0, show_date, show_year};


void Print(uint16_t num);
void SevenSegment(uint8_t n, uint8_t dp);
void read(unsigned char position);
void write(unsigned char value, unsigned char position);
void timer0start(void);
void timer1start(void);
void timer1stop(void);

void portConfig(void) {
    // Port c[3,2,1,0] as out put
    DDRC  = 0xff;
    PORTC = 0x00;

    DDRB  = 0b11111100;
    PORTB = 0x00;

    // Port D
    SEVEN_SEGMENT_DDR=0xff;

    // Turn off all segments
    SEVEN_SEGMENT_PORT=0xff;
}

void readSecond() {
    read(second_RTC);                               // cte sekundy z RTC
    second = ((((BCD & 0b01111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f)) ;
}

void readMinute() {
    read(minute_RTC);                                // cte minuty z RTC
    minute = ((((BCD & 0b01111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
}

void readHour() {
    read(hour_RTC);                                // cte hodiny z RTC
    hour = ((((BCD & 0b00111111) & 0xf0) >> 4) * 10 + (BCD & 0x0f));
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

unsigned int debounce(volatile uint8_t *port, uint8_t pin) {
    if (!(*port & (1 << pin))) {
        _delay_ms(200);
        return 1;
    }
    return 0;
}

int main() {

    // global interupt enable
    sei();

    // Start I2C
    i2c_init();

    // Start UART
    //UART_init();

    portConfig();

    // timer0 aprox. get data from RTC
    // timer1 for update 7 segment
    timer0start();

    // aprox. 4x per second
    timer1start();

    /*
     * Real time clock configuration
     */
    //write(14, year_RTC);        // 00-99
    //write(7, month_RTC);
    //write(0, dayOfTheWeek_RTC); // 0 Mo, 1 Tu, ...
    //write(28, day_RTC);
    //write(0, hour_RTC);        // Bohuzel PCF8563 umoznuje mit den s 32hodinami:-)
    //write(0, minute_RTC);
    //write(0, second_RTC);

    getTime();

    while(1) {
        if (debounce(&PINB, PB0)) {
            if (set) {
                timer1stop();
                switch (show) {
                case show_time:
                    if(set == set_minute) {
                        minute++;
                        if (minute > 59) {
                            minute = 0;
                        }
                        write(minute, minute_RTC);
                    }
                    if(set == set_hour) {
                        hour++;
                        if (hour > 23) {
                            hour = 0;
                        }
                        write(hour, hour_RTC);
                    }
                    break;
                case show_date:
                    if(set == set_day) {
                        day++;
                        if (day > 31) {
                            day = 1;
                        }
                        write(day, day_RTC);
                    }
                    if(set == set_month) {
                        month++;
                        if (month > 12) {
                            month = 1;
                        }
                        write(month, month_RTC);
                    }
                    break;
                case show_year:
                    if(set == set_year) {
                        year++;
                        if (year > 99) {
                            year = 1;
                        }
                        write(year, year_RTC);
                    }
                    break;
                }
                timer1start();


            } else {
                show++;
                if (show == show_date) {
                    readDate();
                }
                if (show == show_year) {
                    readYear();
                }
            }
            if (show == 3) show = show_time;
        }

        if (debounce(&PINB, PB1)) {
            set++;

            // read date, set date
            if (set == set_day) {
                readDate();
                show = show_date;
            }
            if (set == set_month) {
                readDate();
                show = show_date;
            }
            if (set == set_year) {
                readYear();
                show = show_year;
            }

            if (set == 6) {
                set = set_none;
                show = show_time;
            }
        }

        if (set) {
            PORTB |= (1 << PB2);
        } else {
            PORTB &= ~(1 << PB2);
        }
    }

    return 0;
}

ISR(TIMER1_OVF_vect) {
    readSecond();

    if (second % 2) {
        blinkDot = true;
    } else {
        blinkDot = false;
    }

    if (second == 0) {
        readMinute();
    }
    if (minute == 0) {
        readHour();
    }
}

ISR(TIMER0_OVF_vect) {
    int disp;

    switch (show) {
    case show_time:
        disp = (hour * 100) + minute;
        break;
    case show_date:
        disp = (day * 100) + month;
        break;
    case show_year:
        disp = year + 2000;
        break;
    default:
        disp = (hour * 100) + minute;
    }

    Print(disp);

    PORTC &= ~(1 << PC0);
    PORTC &= ~(1 << PC1);
    PORTC &= ~(1 << PC2);
    PORTC &= ~(1 << PC3);

    if (set == set_hour || set == set_day) {
        blickCounter++;

        if (i == 3 || i == 2) {
            if (blickCounter > 100) {
                PORTC |= (1 << i);
                if (blickCounter == 200) {
                    blickCounter = 0;
                }
            }
        } else {
            PORTC |= (1 << i);
        }

        // blinking minute
    } else if (set == set_minute || set == set_month) {
        blickCounter++;

        if (i == 1 || i == 0) {
            if (blickCounter > 100) {
                PORTC |= (1 << i);
                if (blickCounter == 200) {
                    blickCounter = 0;
                }
            }
        } else {
            PORTC |= (1 << i);
        }

    } else if (set == set_year) {
        blickCounter++;
        if (blickCounter > 100) {
            PORTC |= (1 << i);
            if (blickCounter == 200) {
                blickCounter = 0;
            }
        }
    } else {
        PORTC |= (1 << i);
    }

    if (blinkDot && i == 2) {
        SevenSegment(digits[i], 1);
    } else {
        SevenSegment(digits[i], 0);
    }

    if (i == 3) {
        i = 0;
    } else {
        i++;
    }
}

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

void timer0start(void) {
    TCCR0B |= (1 << CS01) | (1 << CS00);   // Prescaler = FCPU /256
    TIMSK0 |= (1 << TOIE0);  // Enable Overflow Interrupt Enable
    TCNT0 = 0;              // Initialize Counter
}

void timer1start(void) {
    TCCR1B |= (1 << CS11);// | (1 << CS10);   // Prescaler = FCPU / 64
    TIMSK1 |= (1 << TOIE1);  // Enable Overflow Interrupt Enable
    TCNT1 = 0;              // Initialize Counter
}

void timer1stop(void) {
    TIMSK1 &= ~(1 << TOIE1);  // Disable Overflow Interrupt Enable
}


void Print(uint16_t num) {

    uint8_t i = 0;
    uint8_t j;
    if(num > 9999) return;

    while(num) {
        digits[i] = num % 10;
        i++;
        num = num / 10;
    }

    // Fill with leading 0s
    for(j = i; j < 4; j++) digits[j]=0;
}

/**
 * This function writes a digits given by n to the display
 * the decimal point is displayed if dp=1
 * Note: n must be less than 9
 *
 **/
void SevenSegment(uint8_t n, uint8_t dp) {
    if(n < 11) {
        switch (n) {
        case 0:
            SEVEN_SEGMENT_PORT=0b11111100;
            break;

        case 1:
            SEVEN_SEGMENT_PORT=0b01100000;
            break;

        case 2:
            SEVEN_SEGMENT_PORT=0b11011010;
            break;

        case 3:
            SEVEN_SEGMENT_PORT=0b11110010;
            break;

        case 4:
            SEVEN_SEGMENT_PORT=0b01100110;
            break;

        case 5:
            SEVEN_SEGMENT_PORT=0b10110110;
            break;

        case 6:
            SEVEN_SEGMENT_PORT=0b10111110;
            break;

        case 7:
            SEVEN_SEGMENT_PORT=0b11100000;
            break;

        case 8:
            SEVEN_SEGMENT_PORT=0b11111110;
            break;

        case 9:
            SEVEN_SEGMENT_PORT=0b11110110;
            break;

        case 10:
            //A BLANK DISPLAY
            SEVEN_SEGMENT_PORT=0b00000000;
            break;
        }

        if(dp) {
            //if decimal point should be displayed
            //make 0th bit Low
            //SEVEN_SEGMENT_PORT&=0b11111110;
            SEVEN_SEGMENT_PORT |= 0b00000001;
        }
    } else {
        //This symbol on display tells that n was greater than 10
        //so display can't handle it
        //SEVEN_SEGMENT_PORT=0b11111101;
        SEVEN_SEGMENT_PORT=0b00000010;
    }
}
