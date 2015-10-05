#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <string.h>
#include "twimaster.h"
#include "rtc8563.h"
#include "adc.h"
#include "fifo.h"
//#include "uart.h"

#define SEVEN_SEGMENT_PORT PORTD
#define SEVEN_SEGMENT_DDR DDRD

#define true 1
#define false 0

#define SWITCH_COUNT 12
#define DIGITS 4
#define STATUS_EEPROM_POS 128
#define ADCCHANNEL 6

volatile unsigned int i = 0;

volatile unsigned char showDot;
volatile unsigned char show = 0;
volatile unsigned int set = 0;
volatile unsigned char blickCounter = 0;
volatile unsigned int tempCounter = 0;
volatile unsigned int temperature = 0;

volatile char digitsc[DIGITS];

volatile uint8_t on[SWITCH_COUNT];
volatile uint8_t co2[4];

volatile uint8_t switchStatus = 0;
volatile unsigned int changeStateCounter = 0;
volatile unsigned int dayStatus[6];
volatile unsigned int daySt = 0;

enum {
    d_day = 0,
    d_night,
    d_off
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
    show_onTime2,
    show_onTime3,
    show_onTime4,
    show_onTime5,
    show_onTime6,
    show_co2on,
    show_co2off,
    show_end
};

// method forward declaration
unsigned int debounce(volatile uint8_t *port, uint8_t pin);
inline unsigned char bitIsSet(unsigned char byte, unsigned int bit);
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

    // termistor on ADC channel 2
    ADC_channel(ADCCHANNEL);

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
                            year = 0;
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
                        } else if (dayStatus[0] == d_night) {
                            dayStatus[0] = d_off;
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
                        eeprom_write_byte ((uint8_t*)2, on[2]);
                    }
                    if(set == set_second) {
                        on[3]++;
                        if (on[3] > 59) {
                            on[3] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)3, on[3]);
                    }
                    if(set == set_both) {
                        if (dayStatus[1] == d_day) {
                            dayStatus[1] = d_night;
                        } else if (dayStatus[1] == d_night) {
                            dayStatus[1] = d_off;
                        } else {
                            dayStatus[1] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 2, (uint8_t)dayStatus[1]);
                    }
                    break;

                case show_onTime3:
                    if(set == set_first) {
                        on[4]++;
                        if (on[4] > 23) {
                            on[4] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)4, on[4]);
                    }
                    if(set == set_second) {
                        on[5]++;
                        if (on[5] > 59) {
                            on[5] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)5, on[5]);
                    }
                    if(set == set_both) {
                        if (dayStatus[2] == d_day) {
                            dayStatus[2] = d_night;
                        } else if (dayStatus[2] == d_night) {
                            dayStatus[2] = d_off;
                        } else {
                            dayStatus[2] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 3, (uint8_t)dayStatus[2]);
                    }
                    break;

                case show_onTime4:
                    if(set == set_first) {
                        on[6]++;
                        if (on[6] > 23) {
                            on[6] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)6, on[6]);
                    }
                    if(set == set_second) {
                        on[7]++;
                        if (on[7] > 59) {
                            on[7] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)7, on[7]);
                    }
                    if(set == set_both) {
                        if (dayStatus[3] == d_day) {
                            dayStatus[3] = d_night;
                        } else if (dayStatus[3] == d_night) {
                            dayStatus[3] = d_off;
                        } else {
                            dayStatus[3] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 4, (uint8_t)dayStatus[3]);
                    }
                    break;

                case show_onTime5:
                    if(set == set_first) {
                        on[8]++;
                        if (on[8] > 23) {
                            on[8] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)8, on[8]);
                    }
                    if(set == set_second) {
                        on[9]++;
                        if (on[9] > 59) {
                            on[9] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)9, on[9]);
                    }
                    if(set == set_both) {
                        if (dayStatus[4] == d_day) {
                            dayStatus[4] = d_night;
                        } else if (dayStatus[4] == d_night) {
                            dayStatus[4] = d_off;
                        } else {
                            dayStatus[4] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 5, (uint8_t)dayStatus[4]);
                    }
                    break;

                case show_onTime6:
                    if(set == set_first) {
                        on[10]++;
                        if (on[10] > 23) {
                            on[10] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)10, on[10]);
                    }
                    if(set == set_second) {
                        on[11]++;
                        if (on[11] > 59) {
                            on[11] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)11, on[11]);
                    }
                    if(set == set_both) {
                        if (dayStatus[5] == d_day) {
                            dayStatus[5] = d_night;
                        } else if (dayStatus[5] == d_night) {
                            dayStatus[5] = d_off;
                        } else {
                            dayStatus[5] = d_day;
                        }
                        eeprom_write_byte ((uint8_t*)STATUS_EEPROM_POS + 6, (uint8_t)dayStatus[5]);
                    }
                    break;

                case show_co2on:
                    if(set == set_first) {
                        co2[0]++;
                        if (co2[0] > 23) {
                            co2[0] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)12, co2[0]);
                    }
                    if(set == set_second) {
                        co2[1]++;
                        if (co2[1] > 59) {
                            co2[1] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)13, co2[1]);
                    }
                    break;

                case show_co2off:
                    if(set == set_first) {
                        co2[2]++;
                        if (co2[2] > 23) {
                            co2[2] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)14, co2[2]);
                    }
                    if(set == set_second) {
                        co2[3]++;
                        if (co2[3] > 59) {
                            co2[3] = 0;
                        }
                        eeprom_write_byte ((uint8_t*)15, co2[3]);
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
            case show_co2on:
            case show_co2off:
                set++;
                if (set == set_both) {
                    set = set_none;
                }
                readDate();
                break;

            case show_onTime1:
            case show_onTime2:
            case show_onTime3:
            case show_onTime4:
            case show_onTime5:
            case show_onTime6:
                set++;
                if (set == set_end) {
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

    tempCounter++;
    if (tempCounter == 500) {
        temperature = getTemperature();
        tempCounter = 0;
    }

    // timer switch
    if (switchStatus == switch_auto) {
        int t = (hour * 60) + minute;
        unsigned int i, j;

        j = 0;
        for (i = 0; i < 11; i = i + 2) {
            int on_;
            if (i == 0) {
                daySt = dayStatus[(SWITCH_COUNT / 2) - 1];
            } else {
                on_ = (on[i] * 60) + on[i + 1];
                if (t > on_) {
                    daySt = dayStatus[j];
                }
            }
            j++;
        }

        // lights
        if (daySt == d_day) { // day
            PORTC |= (1 << PC1);
            PORTC |= (1 << PC0);
        } else if (daySt == d_night) { // night
            PORTC |= (1 << PC1);
            PORTC &= ~(1 << PC0);
        } else { // off
            PORTC &= ~(1 << PC1);
            PORTC &= ~(1 << PC0);
        }

        // CO2
        int co2on  = (co2[0] * 60) + co2[1];
        int co2off = (co2[2] * 60) + co2[3];
        if (co2on <= t && t < co2off) {
            PORTC |= (1 << PC2);
        } else {
            PORTC &= ~(1 << PC2);
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
    changeStateCounter++;
    if (show == show_time) {
        if (changeStateCounter == 400) {
            show = show_date;
            set = set_none;
            changeStateCounter = 0;
        }
    } else if (show == show_date) {
        if (changeStateCounter == 100) {
            show = show_temperature;
            set = set_none;
            changeStateCounter = 0;
        }
    } else {
        if (changeStateCounter == 50) {
            show = show_time;
            set = set_none;
            changeStateCounter = 0;
        }
    }
}

/**
 * Timer Overflow for update display
 */
ISR(TIMER0_OVF_vect) {

    unsigned char position = 0b0100;
    char s[4];

    //show = show_temperature;

    switch (show) {
    case show_time:
        sprintf (s, "%02d%02d", hour, minute);
        position = 0b0100;
        PrintChr(s);
        break;

    case show_switch:
        position = 0b0000;
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
        sprintf (s, "%02d%02d", day, month);
        showDot = true;
        position = 0b0100;
        PrintChr(s);
        break;

    case show_temperature:
        if (temperature == 0) {
            sprintf (s, "---C");
        } else {
            sprintf (s, "%03dC", temperature);
        }
        showDot = true;
        position = 0b0100;
        PrintChr(s);
        break;

    case show_year:
        sprintf (s, "20%02d", year);
        showDot = false;
        position = 0b0000;
        PrintChr(s);
        break;

    case show_onTime1:
        showDot = true;
        position = 0b1000;
        if (set == set_both) {
            if (dayStatus[0] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[0] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[0], on[1]);
            PrintChr(s);
        }
        break;

    case show_onTime2:
        showDot = true;
        position = 0b0100;
        if (set == set_both) {
            if (dayStatus[1] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[1] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[2], on[3]);
            PrintChr(s);
        }
        break;

    case show_onTime3:
        showDot = true;
        position = 0b0010;
        if (set == set_both) {
            if (dayStatus[2] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[2] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[4], on[5]);
            PrintChr(s);
        }
        break;

    case show_onTime4:
        showDot = true;
        position = 0b0000;
        if (set == set_both) {
            if (dayStatus[3] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[3] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[6], on[7]);
            PrintChr(s);
        }
        break;

    case show_onTime5:
        showDot = true;
        position = 0b1111;
        if (set == set_both) {
            if (dayStatus[4] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[4] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[8], on[9]);
            PrintChr(s);
        }
        break;

    case show_onTime6:
        showDot = true;
        position = 0b0111;
        if (set == set_both) {
            if (dayStatus[5] == d_day) {
                PrintChr("S-dA");
            } else if (dayStatus[5] == d_night) {
                PrintChr("S-ni");
            } else {
                PrintChr("S-Of");
            }
        } else {
            sprintf (s, "%02d%02d", on[10], on[11]);
            PrintChr(s);
        }
        break;

    case show_co2on:
        if (set == set_first || set == set_second) {
            sprintf (s, "%02d%02d", co2[0], co2[1]);
            PrintChr(s);
            showDot = true;
            position = 0b0100;
        } else {
            showDot = true;
            position = 0b0010;
            PrintChr("CO2O");
        }
        break;

    case show_co2off:
        showDot = true;
        if (set == set_first || set == set_second) {
            sprintf (s, "%02d%02d", co2[2], co2[3]);
            position = 0b0100;
            PrintChr(s);
        } else {
            position = 0b0010;
            PrintChr("CO2F");
        }
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

    // display string
    if (showDot && bitIsSet(position, i)) {
        SevenSegmentChar(digitsc[i], 1);
    } else {
        SevenSegmentChar(digitsc[i], 0);
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

void PrintChr(char c[]) {
    unsigned int i = 0;
    unsigned int j = 3;

    for(i = 0; i < 4; i++) {
        digitsc[i] = c[j];
        j--;
    }
}

/**
 * This function writes a char given by n to the display
 * the decimal point is displayed if dp=1
 */
void SevenSegmentChar(char ch, uint8_t dp) {
    switch (ch) {
    case ' ':
        SEVEN_SEGMENT_PORT=0b00000000;
        break;
    case '0':
        SEVEN_SEGMENT_PORT=0b11111100;
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
    case 'A':
        SEVEN_SEGMENT_PORT=0b11101110;
        break;
    case 'C':
        SEVEN_SEGMENT_PORT=0b10011100;
        break;
    case 'F':
        SEVEN_SEGMENT_PORT=0b10001110;
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
    DDRC  = 0b0000111;
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

inline unsigned char bitIsSet(unsigned char byte, unsigned int bit)  {
    return byte & (1 << bit);
}

void readDataFromEeprom() {
    unsigned int hour;
    unsigned int min;

    // sequence 1
    hour = eeprom_read_byte((uint8_t*)0);
    if (hour > 23) { hour = 0; }
    on[0] = hour;

    min = eeprom_read_byte((uint8_t*)1);
    if (min > 59) { min = 0; }
    on[1] = min;

    // sequence 2
    hour = eeprom_read_byte((uint8_t*)2);
    if (hour > 23) { hour = 0; }
    on[2] = hour;

    min = eeprom_read_byte((uint8_t*)3);
    if (min > 59) { min = 0; }
    on[3] = min;

    // sequence 3
    hour = eeprom_read_byte((uint8_t*)4);
    if (hour > 23) { hour = 0; }
    on[4] = hour;

    min = eeprom_read_byte((uint8_t*)5);
    if (min > 59) { min = 0; }
    on[5] = min;

    // sequence 4
    hour = eeprom_read_byte((uint8_t*)6);
    if (hour > 23) { hour = 0; }
    on[6] = hour;

    min = eeprom_read_byte((uint8_t*)7);
    if (min > 59) { min = 0; }
    on[7] = min;

    // sequence 5
    hour = eeprom_read_byte((uint8_t*)8);
    if (hour > 23) { hour = 0; }
    on[8] = hour;

    min = eeprom_read_byte((uint8_t*)9);
    if (min > 59) { min = 0; }
    on[9] = min;

    // sequence 6
    hour = eeprom_read_byte((uint8_t*)10);
    if (hour > 23) { hour = 0; }
    on[10] = hour;

    min = eeprom_read_byte((uint8_t*)11);
    if (min > 59) { min = 0; }
    on[11] = min;

    // co2 on
    hour = eeprom_read_byte((uint8_t*)12);
    if (hour > 23) { hour = 0; }
    co2[0] = hour;

    min = eeprom_read_byte((uint8_t*)13);
    if (min > 59) { min = 0; }
    co2[1] = min;

    // co2 off
    hour = eeprom_read_byte((uint8_t*)14);
    if (hour > 23) { hour = 0; }
    co2[2] = hour;

    min = eeprom_read_byte((uint8_t*)15);
    if (min > 59) { min = 0; }
    co2[3] = min;


    // save switch and day status
    __EEGET(switchStatus, (uint8_t*)STATUS_EEPROM_POS);
    if (switchStatus > 5) {
        switchStatus = 0;
    }

    __EEGET(dayStatus[0], (uint8_t*)STATUS_EEPROM_POS + 1);
    if (dayStatus[0] > 2) {
        dayStatus[0] = 0;
    }

    __EEGET(dayStatus[1], (uint8_t*)STATUS_EEPROM_POS + 2);
    if (dayStatus[1] > 2) {
        dayStatus[1] = 0;
    }

    __EEGET(dayStatus[2], (uint8_t*)STATUS_EEPROM_POS + 3);
    if (dayStatus[2] > 2) {
        dayStatus[2] = 0;
    }

    __EEGET(dayStatus[3], (uint8_t*)STATUS_EEPROM_POS + 4);
    if (dayStatus[3] > 2) {
        dayStatus[3] = 0;
    }

    __EEGET(dayStatus[4], (uint8_t*)STATUS_EEPROM_POS + 5);
    if (dayStatus[4] > 2) {
        dayStatus[4] = 0;
    }

    __EEGET(dayStatus[5], (uint8_t*)STATUS_EEPROM_POS + 6);
    if (dayStatus[5] > 2) {
        dayStatus[5] = 0;
    }
}
