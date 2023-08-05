#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _DATE {
    int32_t  year;
    uint8_t  month;
    uint8_t  day;
    struct {
        uint8_t is_leap_month : 1;
        uint8_t is_holiday : 1;
    } ext;
} DATE;


/**
 * Get the day of the week
 * @param year a year
 * @param month a  month [1..12]
 * @param day a day [1..31]
 * @return [0..6] which means [Sun..Sat] or [Mon..Sun] depending on LV_CALENDAR_WEEK_STARTS_MONDAY
 */
uint8_t get_day_of_week(int32_t year, uint8_t month, uint8_t day);


/**
 * Get the number of days in a month
 * @param year a year
 * @param month a month. The range is basically [1..12]
 * @return [28..31]
 */
uint8_t get_month_days(int32_t year, uint8_t month);


/**
 * Tells whether a year is leap year or not
 * @param year a year
 * @return 0: not leap year; 1: leap year
 */
uint8_t is_leap_year(int32_t year);

/**
 * Get days elapsed from 1900/01/01
 */
int32_t days_from_1900(int32_t year, uint8_t month, uint8_t day);

/**
 * Get seconds elapsed from 1900/01/01 00:00:00
 */
int64_t seconds_from_1900(int32_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);


int get_lunar_date(int32_t year, uint8_t month, uint8_t day, DATE * lunar_date);

const char * get_lunar_month_str(uint8_t lunar_month, uint8_t is_leap_month);

const char * get_lunar_day_str(uint8_t lunar_day);

const char * get_week_str(uint8_t week);

const char * get_animal_str(int32_t lunar_year);

const char * get_year_ganzhi_str(int32_t lunar_year);

const char * get_month_ganzhi_str(int32_t lunar_year, uint8_t lunar_month);

const char * get_day_ganzhi_str(int32_t gre_year, uint8_t month, uint8_t day);


const char * get_festival(int32_t year, uint8_t month, uint8_t day);


const char * get_gre_festival(uint8_t month, uint8_t day);


const char * get_lunar_festival(DATE * lunar_date);


const char * get_week_festival(uint8_t month, uint8_t day, uint8_t days_of_month, uint8_t week_of_day1);

#ifdef __cplusplus
}
#endif