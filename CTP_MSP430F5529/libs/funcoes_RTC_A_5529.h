/*
 * funcoes_RTC_A_5529.h
 *
 *  Created on: Mar 29, 2023
 *      Author: guiwz
 */

#ifndef LIBS_FUNCOES_RTC_A_5529_H_
#define LIBS_FUNCOES_RTC_A_5529_H_


//*****************************************************************************
//
//! \brief Used in the RTC_A_initCalendar() function as the CalendarTime
//! parameter.
//
//*****************************************************************************
typedef struct Calendar {
    //! Seconds of minute between 0-59
    uint8_t Seconds;
    //! Minutes of hour between 0-59
    uint8_t Minutes;
    //! Hour of day between 0-23
    uint8_t Hours;
    //! Day of week between 0-6
    uint8_t DayOfWeek;
    //! Day of month between 1-31
    uint8_t DayOfMonth;
    //! Month between 1-12
    uint8_t Month;
    //! Year between 0-4095
    uint16_t Year;
} Calendar;




#endif /* LIBS_FUNCOES_RTC_A_5529_H_ */
