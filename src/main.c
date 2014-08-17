#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include "twimaster.h"
#include "rtc8563.h"
#include "adc.h"
//#include "uart.h"

#define SEVEN_SEGMENT_PORT PORTD
#define SEVEN_SEGMENT_DDR DDRD

#define true 1
#define false 0

#define SWITCH_COUNT 4
#define DIGITS 4
#define STATUS_EEPROM_POS 8

volatile uint8_t i = 0;

volatile unsigned char showDot;
volatile unsigned char show = 0;
volatile unsigned int set = 0;
volatile unsigned char blickCounter = 0;

volatile uint8_t digits[DIGITS];
volatile char digitsc[DIGITS];

volatile uint8_t on[SWITCH_COUNT];
volatile uint8_t off[SWITCH_COUNT];

volatile uint8_t switchStatus = 0;
volatile unsigned int changeStateCounter = 0;
volatile unsigned int dayStatus[2];

enum {
    d_day = 0,
    d_night
};

enum {
    switch_off = 0,
    switch_night,
    switch_day,
    switch_auto,
    switch_end
};

enum {
    set_none = 0,
    set_first,
    set_second,
    set_both,
    set_end
};

enum {
    show_time = 0,
    show_switch,
    show_date,
    show_temperature,
    show_year,
    show_onTime1,
    show_offTime1,
    show_onTime2,
    show_offTime2,
    show_end
};

unsigned int debounce(volatile uint8_t *port, uint8_t pin);

void Print(uint16_t num);
void PrintChr(char *c);

void SevenSegment(uint8_t n, uint8_t dp);
void SevenSegmentChar(char ch, uint8_t dp);

void timer0start(void);
void timer1start(void);
void timer1stop(void);
void portConfig(void);

void readDataFromEeprom();

int main() {

    // global interupt enable
    sei();

    // Start I2C
    i2c_init();

    // Start UART
    //UART_init();

    // start ADC
    ADC_init();
    ADC_channel(2);

    portConfig();

    // timer0 aprox. get data from RTC
    // timer1 for update 7 segment
    timer0start();

    // aprox. 4x per second
    timer1start();

    // read Epprom
    readDataFromEeprom();

    // read time from RTC
    getTime();
    //char str[20];
    //uint16_t val;

    while(1) {
        // if set is set for modify values PB1
        if (debounce(&PINB, PB1)) {
            changeStateCounter = 0;

            if (set) {
                timer1stop();
                switch (show) {
                case show_time:
                    if(set == set_second) {
                        minute++;
                        if (minute > 59) {
                            minute = 0;
                        }
                        write(minute, minute_RTC);
                    }
                    if(set == set_first) {
                        hour++;
                        if (hour > 23) {
                            hour = 0;
                        }
                        write(hour, hour_RTC);
                    }
                    break;

                case show_switch:
                    break;

                case show_date:
                    if(set == set_first) {
                        day++;
                        if (day > 31) {
                            day = 1;
                        }
                        write(day, day_RTC);
                    }
                    if(set == set_second) {
                        month++;
                        if (month > 12) {
                            month = 1;
                        }
                        write(month, month_RTC);
                    }
                    break;
                case show_year:
                    if(set == set_both) {
                        year++;
                        if (year > 99) {
                            year = 1;
                        }
                        write(year, year_RTC);
                    }
                    break;

                case show_onTime1:
                    if(set == set_first) {
                        on[0]++;
                        if (on[0] > 23) {
                            on[0] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)0, on[0]);
                    }
                    if(set == set_second) {
                        on[1]++;
                        if (on[1] > 59) {
                            on[1] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)1, on[1]);
                    }
                    if(set == set_both) {
                        if (dayStatus[0] == d_day) {
                            dayStatus[0] = d_night;
                        } else {
                            dayStatus[0] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 1, (uint8_t)dayStatus[0]);
                    }
                    break;

                case show_onTime2:
                    if(set == set_first) {
                        on[2]++;
                        if (on[2] > 23) {
                            on[2] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)4, on[2]);
                    }
                    if(set == set_second) {
                        on[3]++;
                        if (on[3] > 59) {
                            on[3] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)5, on[3]);
                    }
                    if(set == set_both) {
                        if (dayStatus[1] == d_day) {
                            dayStatus[1] = d_night;
                        } else {
                            dayStatus[1] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 2, (uint8_t)dayStatus[1]);
                    }
                    break;

                case show_offTime1:
                    if(set == set_first) {
                        off[0]++;
                        if (off[0] > 23) {
                            off[0] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)2, off[0]);
                    }
                    if(set == set_second) {
                        off[1]++;
                        if (off[1] > 59) {
                            off[1] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)3, off[1]);
                    }
                    break;

                case show_offTime2:
                    if(set == set_first) {
                        off[2]++;
                        if (off[2] > 23) {
                            off[2] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)6, off[2]);
                    }
                    if(set == set_second) {
                        off[3]++;
                        if (off[3] > 59) {
                            off[3] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)7, off[3]);
                    }
                    break;
                }
                timer1start();

            } else {

                // this is place for change display
                // daytime, date, yer, set dates, etc.
                show++;
                if (show == show_end) {
                    show = show_time;
                }
            }
        }

        // button for change position for update PC3
        if (debounce(&PINC, PC3)) {
            changeStateCounter = 0;

            switch (show) {
            case show_time:
            case show_date:
                set++;
                if (set == set_both) {
                    set = set_none;
                }
                readDate();
                break;

            case show_onTime1:
            case show_onTime2:
                set++;
                if (set == set_end) {
                    set = set_none;
                }
                break;

            case show_offTime1:
            case show_offTime2:
                set++;
                if (set == set_both) {
                    set = set_none;
                }
                break;

            case show_year:
                if (set == set_both) {
                    set = set_none;
                } else {
                    set = set_both;
                }
                readYear();
                break;

            case show_switch:
                switchStatus++;
                eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS, switchStatus);
                if(switchStatus == switch_end) {
                    switchStatus = switch_off;
                }
                break;
            }
        }

        if (debounce(&PINB, PB0)) {
            changeStateCounter = 0;
            show = show_switch;
            switchStatus++;
            eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS, switchStatus);
            if (switchStatus == switch_end) {
                switchStatus = switch_off;
            }
        }

        //val = getTemperature();
        //
        //sprintf (str, "value: %d.", val);
        //UART_puts(str);
        //UART_puts("\n");
        //_delay_ms(500);
    }
    return 0;
}

/**
 * Timer Overflow for update time from RTC
 * and for switch
 */
ISR(TIMER1_OVF_vect) {
    readSecond();

    // evere second blick the dot
    if (second % 2) {
        showDot = true;
    } else {
        showDot = false;
    }

    if (second == 0) {
        readMinute();
    }
    if (minute == 0) {
        readHour();
    }
    if (show == show_date) {
        readDate();
    }
    if (show == show_year) {
        readYear();
    }

    // timer switch
    if (switchStatus == switch_auto) {
        int t = (hour * 60) + minute;
        unsigned int i, j;
        unsigned int st = false;
        unsigned int daySt = d_day;

        j = 0;
        for (i = 0; i < 3; i = i + 2) {
            int on_ = (on[i] * 60) + on[i + 1];
            int off_ = (off[i] * 60) + off[i + 1];
            if (t >= on_ && t <= off_) {
                st = true;
                daySt = dayStatus[j];
            }
            j++;
        }

        if (daySt == d_day) { // day
            PORTC |= (1 << PC0);
        } else {               // night
            PORTC &= ~(1 << PC0);
        }
        if (st) {
            PORTC |= (1 << PC1);
        } else {
            PORTC &= ~(1 << PC1);
        }
    }
    if (switchStatus == switch_off) {
        PORTC &= ~(1 << PC1);
        PORTC &= ~(1 << PC0);
    }
    if (switchStatus == switch_night) {
        PORTC |= (1 << PC1);
        PORTC &= ~(1 << PC0);
    }
    if (switchStatus == switch_day) {
        PORTC |= (1 << PC1);
        PORTC |= (1 << PC0);
    }

    // automaticaly swith to show time and
    // back to show date
    // and reset state due inactivity
    //if (show == show_time) {
    //    changeStateCounter++;
    //    if (changeStateCounter == 400) {
    //        show = show_date;
    //        set = set_none;
    //        changeStateCounter = 0;
    //    }
    //} else {
    //    changeStateCounter++;
    //    if (changeStateCounter == 300) {
    //        show = show_time;
    //        set = set_none;
    //        changeStateCounter = 0;
    //    }
    //}
}

/**
 * Timer Overflow for update display
 */
ISR(TIMER0_OVF_vect) {
    int disp = 0;
    int temp = 0;
    unsigned int position = 2;
    unsigned int dispc = false;
    char str[5];

    switch (show) {
    case show_time:
        disp = (hour * 100) + minute;
        position = 2;
        Print(disp);
        break;

    case show_switch:
        dispc = true;
        position = 5;
        if (switchStatus == switch_off) {
            PrintChr("L-Of");
        }
        if (switchStatus == switch_night) {
            PrintChr("L-ni");
        }
        if (switchStatus == switch_day) {
            PrintChr("L-dA");
        }
        if (switchStatus == switch_auto) {
            PrintChr("L-AU");
        }
        break;

    case show_date:
        disp = (day * 100) + month;
        showDot = true;
        position = 2;
        Print(disp);
        break;

    case show_temperature:
        temp = getTemperature();
        if (temp == 0) {
            snprintf (str, 5, "---C");
        } else {
            snprintf (str, 5, "%03dC", temp);
        }
        showDot = true;
        dispc = true;
        position = 2;
        PrintChr(str);
        break;

    case show_year:
        disp = year + 2000;
        showDot = false;
        position = 5;
        Print(disp);
        break;

    case show_onTime1:
        showDot = true;
        position = 3;
        if (set == set_both) {
            dispc = true;
            if (dayStatus[0] == d_day) {
                PrintChr("S-dA");
            } else {
                PrintChr("S-ni");
            }
        } else {
            disp = (on[0] * 100) + on[1];
            Print(disp);
        }
        break;

    case show_onTime2:
        showDot = true;
        position = 2;
        if (set == set_both) {
            dispc = true;
            if (dayStatus[1] == d_day) {
                PrintChr("S-dA");
            } else {
                PrintChr("S-ni");
            }
        } else {
            disp = (on[2] * 100) + on[3];
            Print(disp);
        }
        break;

    case show_offTime1:
        position = 3;
        disp = (off[0] * 100) + off[1];
        Print(disp);
        break;

    case show_offTime2:
        position = 2;
        disp = (off[2] * 100) + off[3];
        Print(disp);
        break;
    }

    PORTB &= ~(1 << PB2);//0
    PORTB &= ~(1 << PB3);//1
    PORTB &= ~(1 << PB4);//2
    PORTB &= ~(1 << PB5);//3

    blickCounter++;

    switch (set) {
    case set_first:
        if (i == 3 || i == 2) {
            if (blickCounter > 100) {
                PORTB |= (1 << (i + 2));
                if (blickCounter == 200) {
                    blickCounter = 0;
                }
            }
        } else {
            PORTB |= (1 << (i + 2));
        }
        break;
        // blinking minute
    case set_second:
        if (i == 1 || i == 0) {
            if (blickCounter > 100) {
                PORTB |= (1 << (i + 2));
                if (blickCounter == 200) {
                    blickCounter = 0;
                }
            }
        } else {
            PORTB |= (1 << (i + 2));
        }
        break;
    case set_both:
        if (blickCounter > 100) {
            PORTB |= (1 << (i + 2));
        }
        break;
    default:
        PORTB |= (1 << (i + 2));
    }

    if (blickCounter == 200) {
        blickCounter = 0;
    }

    // display number
    if (disp) {
        // show dot on the position i
        if (showDot && i == position) {
            SevenSegment(digits[i], 1);
        } else {
            SevenSegment(digits[i], 0);
        }
    }
    // display string
    if (dispc) {
        if (showDot && i == position) {
            SevenSegmentChar(digitsc[i], 1);
        } else {
            SevenSegmentChar(digitsc[i], 0);
        }
    }

    // reset counter
    if (i == DIGITS - 1) {
        i = 0;
    } else {
        i++;
    }
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

    // Fill with leading 0
    for(j = i; j < 4; j++) digits[j]=0;
}

void PrintChr(char *c) {
    unsigned int i = 0;
    unsigned int j = 3;

    for(i = 0; i < strlen(c); i++) {
        digitsc[i] = c[j];
        j--;
    }
}

/**
 * This function writes a digits given by n to the display
 * the decimal point is displayed if dp=1
 * Note: n must be less than 9
 */
void SevenSegment(uint8_t n, uint8_t dp) {
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
        // A Blank display
        SEVEN_SEGMENT_PORT=0b00000000;
        break;
    }

    if(dp) {
        // If decimal point should be displayed
        // Make 0th bit Low
        SEVEN_SEGMENT_PORT |= 0b00000001;
    }
}

void SevenSegmentChar(char ch, uint8_t dp) {
    switch (ch) {
    case ' ':
        SEVEN_SEGMENT_PORT=0b00000000;
        break;
    case 'O':
        SEVEN_SEGMENT_PORT=0b11111100;
        break;
    case '1':
        SEVEN_SEGMENT_PORT=0b01100000;
        break;
    case '2':
        SEVEN_SEGMENT_PORT=0b11011010;
        break;
    case '3':
        SEVEN_SEGMENT_PORT=0b11110010;
        break;
    case '4':
        SEVEN_SEGMENT_PORT=0b01100110;
        break;
    case '5':
        SEVEN_SEGMENT_PORT=0b10110110;
        break;
    case '6':
        SEVEN_SEGMENT_PORT=0b10111110;
        break;
    case '7':
        SEVEN_SEGMENT_PORT=0b11100000;
        break;
    case '8':
        SEVEN_SEGMENT_PORT=0b11111110;
        break;
    case '9':
        SEVEN_SEGMENT_PORT=0b11110110;
        break;
    case 'C':
        SEVEN_SEGMENT_PORT=0b10011100;
        break;
    case 'L':
        SEVEN_SEGMENT_PORT=0b00011100;
        break;
    case 'n':
        SEVEN_SEGMENT_PORT=0b00101010;
        break;
    case 'f':
        SEVEN_SEGMENT_PORT=0b10001110;
        break;
    case 'A':
        SEVEN_SEGMENT_PORT=0b11101110;
        break;
    case 'i':
        SEVEN_SEGMENT_PORT=0b00100000;
        break;
    case 'd':
        SEVEN_SEGMENT_PORT=0b01111010;
        break;
    case 'e':
        SEVEN_SEGMENT_PORT=0b11011110;
        break;
    case 'u':
        SEVEN_SEGMENT_PORT=0b00111000;
        break;
    case 'U':
        SEVEN_SEGMENT_PORT=0b01111100;
        break;
    case 'Y':
        SEVEN_SEGMENT_PORT=0b01100110;
        break;
    case 'G':
        SEVEN_SEGMENT_PORT=0b10111110;
        break;
    case 'N':
        SEVEN_SEGMENT_PORT=0b11101100;
        break;
    case 'E':
        SEVEN_SEGMENT_PORT=0b01111100;
        break;
    case 'S':
        SEVEN_SEGMENT_PORT=0b10110110;
        break;
    case '-':
        SEVEN_SEGMENT_PORT=0b00000010;
        break;
    }

    if(dp) {
        // If decimal point should be displayed
        // Make 0th bit Low
        SEVEN_SEGMENT_PORT |= 0b00000001;
    }
}

void portConfig(void) {
    // Port c as output 3 as input
    DDRC  = 0b0000011;
    PORTC = 0x00;

    // Port b 1, 0 as input
    DDRB  = 0b11111100;
    PORTB = 0x00;

    // Port D
    SEVEN_SEGMENT_DDR=0xff;

    // Turn off all segments
    SEVEN_SEGMENT_PORT=0xff;
}

unsigned int debounce(volatile uint8_t *port, uint8_t pin) {
    if (!(*port & (1 << pin))) {
        _delay_ms(180);
        return 1;
    }
    return 0;
}

void readDataFromEeprom() {
    unsigned int hour;
    unsigned int min;

    hour = eeprom_read_byte((uint8_t*)0);
    if (hour > 23) { hour = 0; }
    on[0] = hour;

    min = eeprom_read_byte((uint8_t*)1);
    if (min > 59) { min = 0; }
    on[1] = min;

    hour = eeprom_read_byte((uint8_t*)2);
    if (hour > 23) { hour = 0; }
    off[0] = hour;

    min = eeprom_read_byte((uint8_t*)3);
    if (min > 59) { min = 0; }
    off[1] = min;

    hour = eeprom_read_byte((uint8_t*)4);
    if (hour > 23) { hour = 0; }
    on[2] = hour;

    min = eeprom_read_byte((uint8_t*)5);
    if (min > 59) { min = 0; }
    on[3] = min;

    hour = eeprom_read_byte((uint8_t*)6);
    if (hour > 23) { hour = 0; }
    off[2] = hour;

    min = eeprom_read_byte((uint8_t*)7);
    if (min > 59) { min = 0; }
    off[3] = min;

    switchStatus = eeprom_read_byte((uint8_t*)STATUS_EEPROM_POS);
    if (switchStatus > 5) {
        switchStatus = 0;
    }

    dayStatus[0] = eeprom_read_byte((uint8_t*)STATUS_EEPROM_POS + 1);
    if (dayStatus[0] > 1) {
        dayStatus[0] = 0;
    }
    dayStatus[1] = eeprom_read_byte((uint8_t*)STATUS_EEPROM_POS + 2);
    if (dayStatus[1] > 1) {
        dayStatus[1] = 0;
    }
}
