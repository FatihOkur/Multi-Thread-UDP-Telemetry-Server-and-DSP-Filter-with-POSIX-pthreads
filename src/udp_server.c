#include "udp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * Initializes the ring buffer, mutex, and condition variables.
 * This must be called before any thread attempts to use the buffer.
 */
void ring_buffer_init(ring_buffer_t *rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    
    // Initialize synchronization primitives
    pthread_mutex_init(&rb->mutex, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
    pthread_cond_init(&rb->not_full, NULL);
}

/*
 * Pushes data into the ring buffer. 
 * If the buffer is full, the calling thread (Receiver) will block 
 * on the 'not_full' condition variable until space is freed.
 */
void ring_buffer_push(ring_buffer_t *rb, telemetry_data_t data) {
    pthread_mutex_lock(&rb->mutex);

    /* 
     * Wait while the buffer is full. 
     * Using a while loop handles spurious wakeups.
     */
    while (rb->count == BUFFER_SIZE) {
        pthread_cond_wait(&rb->not_full, &rb->mutex);
    }

    // Insert data and update head pointer with wrap-around
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % BUFFER_SIZE;
    rb->count++;

    /* Signal the Worker thread that data is now available */
    pthread_cond_signal(&rb->not_empty);
    
    pthread_mutex_unlock(&rb->mutex);
}

/*
 * Pops data from the ring buffer. 
 * If the buffer is empty, the calling thread (Worker) will block 
 * on the 'not_empty' condition variable until data arrives.
 */
telemetry_data_t ring_buffer_pop(ring_buffer_t *rb) {
    telemetry_data_t data;
    
    pthread_mutex_lock(&rb->mutex);

    /* 
     * Wait while the buffer is empty.
     * Prevents reading invalid or stale data.
     */
    while (rb->count == 0) {
        pthread_cond_wait(&rb->not_empty, &rb->mutex);
    }

    // Retrieve data and update tail pointer with wrap-around
    data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % BUFFER_SIZE;
    rb->count--;

    /* Signal the Receiver thread that space is now available */
    pthread_cond_signal(&rb->not_full);

    pthread_mutex_unlock(&rb->mutex);
    
    return data;
}

/*
 * Creates and binds a UDP socket to the given port.
 * Returns the socket file descriptor on success, -1 on failure.
 */
int udp_server_init(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    
    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port);       // Convert to network byte order

    // Bind socket to the specified port
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
