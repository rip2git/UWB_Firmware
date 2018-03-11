/*! ----------------------------------------------------------------------------
 * @file    deca_sleep.c
 * @brief   platform dependent sleep implementation
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_device_api.h"
#include "deca_sleep.h"
#include "port.h"



void deca_sleep(unsigned int time_10ms)
{
	sleep_10ms(time_10ms);
}


void sleep_10ms(unsigned int time_10ms)
{
    /* This assumes that the tick has a period of exactly one millisecond. See CLOCKS_PER_SEC define. */
    unsigned long end = portGetTickCount() + time_10ms;
    while ((signed long)(portGetTickCount() - end) <= 0)
        ;
}
