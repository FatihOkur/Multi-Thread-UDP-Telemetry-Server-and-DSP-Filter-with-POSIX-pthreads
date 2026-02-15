#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <stdint.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define PORT 8080

/* Structure to hold the received telemetry data */
typedef struct {
    double value;
    uint64_t timestamp;
} telemetry_data_t;

/* 
 * Thread-safe Ring Buffer Structure 
 * Employs a mutex for mutual exclusion and two condition variables 
 * to handle empty/full states safely, avoiding busy-waiting.
 */
typedef struct {
    telemetry_data_t buffer[BUFFER_SIZE];
    int head;       /* Write index */
    int tail;       /* Read index */
    int count;      /* Current number of elements in the buffer */
    
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ring_buffer_t;

/* Function prototypes using standard C pointers (no C++ references) */
void ring_buffer_init(ring_buffer_t *rb);
void ring_buffer_push(ring_buffer_t *rb, telemetry_data_t data);
telemetry_data_t ring_buffer_pop(ring_buffer_t *rb);

int udp_server_init(int port);

#endif /* UDP_SERVER_H */
