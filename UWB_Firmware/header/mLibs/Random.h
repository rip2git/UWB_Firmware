#ifndef RANDOM_H
#define RANDOM_H


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Random_Initialization
 *
 * @brief: Initializes the seed of the random generator (using ADC1, which turns off before return from this fn)
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void Random_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Random_GetNumber
 *
 * @brief: Used to get the number within the established limits
 *
 * NOTE: 
 *
 * input parameters
 * @param min - minimum of the returned number (included)
 * @param max - maximum of the returned number (included)
 *
 * output parameters
 *
 * return value is the random number
*/
extern unsigned long Random_GetNumber(unsigned long min, unsigned long max);


#endif