#include "dsp_filter.h"

/*
 * Static variables maintain the state of the filter across multiple function calls.
 * This is an implementation of a Simple Moving Average (SMA/FIR) filter.
 */
static double window[FILTER_WINDOW_SIZE] = {0};
static int index = 0;
static double sum = 0;
static int count = 0;

double apply_moving_average(double new_value) {
    /* 
     * Subtract the oldest value from the running sum.
     * If the buffer isn't full yet, window[index] will just be 0.
     */
    sum -= window[index];
    
    // Add the new value to the array and the running sum
    window[index] = new_value;
    sum += new_value;
    
    // Advance the index, wrapping around to create a circular buffer effect
    index = (index + 1) % FILTER_WINDOW_SIZE;
    
    // Keep track of how many samples we've processed to avoid dividing by 0 or 
    // a number larger than the actual samples available during initial startup.
    if (count < FILTER_WINDOW_SIZE) {
        count++;
    }
    
    // Return the averaged value
    return sum / count;
}
