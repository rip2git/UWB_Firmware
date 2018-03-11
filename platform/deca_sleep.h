/*! ----------------------------------------------------------------------------
 * @file    sleep.h
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

#ifndef _SLEEP_H_
#define _SLEEP_H_

#ifdef __cplusplus
extern "C" {
#endif


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: sleep_ms()
 *
 * Wait for a given amount of time.
 * /!\ This implementation is designed for a single threaded application and is blocking.
 *
 * param  time_ms  time to wait in milliseconds
 */
void sleep_10ms(unsigned int time_10ms);

#ifdef __cplusplus
}
#endif

#endif /* _SLEEP_H_ */
