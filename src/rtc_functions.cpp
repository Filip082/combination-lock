#include "rtc_functions.h"

int8_t getMonthValue(char* m) {
    if (strcmp(m, "Jan") == 0) return 1;
    if (strcmp(m, "Feb") == 0) return 2;
    if (strcmp(m, "Mar") == 0) return 3;
    if (strcmp(m, "Apr") == 0) return 4;
    if (strcmp(m, "May") == 0) return 5;
    if (strcmp(m, "Jun") == 0) return 6;
    if (strcmp(m, "Jul") == 0) return 7;
    if (strcmp(m, "Aug") == 0) return 8;
    if (strcmp(m, "Sep") == 0) return 9;
    if (strcmp(m, "Oct") == 0) return 10;
    if (strcmp(m, "Nov") == 0) return 11;
    if (strcmp(m, "Dec") == 0) return 12;
    return 0;
}

// Helper function: Calculate Day of the Week (0 = Sunday)
int8_t get_dotw(int d, int m, int y) {
    if (m < 3) { m += 12; y -= 1; }
    int k = y % 100;
    int j = y / 100;
    int h = (d + 13 * (m + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (h + 5) % 7; 
}

// Initialize RTC with compile time
void RTC_Init_dateTime() {
    // Start the Pico hardware RTC
    rtc_init(); 

    int16_t year;
    int8_t day, hour, min, sec;
    char month_str[4];
    
    // Parse the compiler macros
    sscanf(__DATE__, "%s %d %d", month_str, &day, &year);
    sscanf(__TIME__, "%d:%d:%d", &hour, &min, &sec);
    
    // Populate the Pico's native datetime structure
    datetime_t t = {
        .year  = year,
        .month = getMonthValue(month_str),
        .day   = day,
        .dotw  = get_dotw(day, getMonthValue(month_str), year),
        .hour  = hour,
        .min   = min,
        .sec   = sec
    };

    // Set the time directly (replaces your RTC_setDateTime function)
    rtc_set_datetime(&t);
}

void RTC_Init_Interrupt(rtc_callback_t callback) {
    datetime_t t = {
        .year  = -1,
        .month = -1,
        .day   = -1,
        .dotw  = -1,
        .hour  = -1,
        .min   = -1,
        .sec   = 0
    };

    rtc_set_alarm(&t, callback);
    rtc_enable_alarm();
}
