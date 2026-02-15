#ifndef DSP_FILTER_H
#define DSP_FILTER_H

#define FILTER_WINDOW_SIZE 5

/* 
 * Applies a Moving Average filter to smooth out noise from the incoming signal.
 * Parameter is passed by value.
 */
double apply_moving_average(double new_value);

#endif /* DSP_FILTER_H */
