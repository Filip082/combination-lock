#pragma once
#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "hardware/rtc.h"
#include "pico/util/datetime.h"


int8_t getMonthValue(char* m);
int8_t get_dotw(int d, int m, int y); // Helper function: Calculate Day of the Week (0 = Sunday)
void RTC_Init_dateTime(const char* date, const char* time); // Initialize RTC with compile time
void RTC_Init_Interrupt(rtc_callback_t callback);
void rtc_set_datetime(const char* date, const char* time);
