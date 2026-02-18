#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_server.h"
#include "dsp_filter.h"

/* 
 * Global ring buffer instance.
 * Shared between the Receiver thread and the Worker thread.
 */
ring_buffer_t telemetry_buffer;

/*
 * Receiver Thread: 
 * Listens for incoming UDP datagrams, parses the timestamp and value, 
 * and pushes the struct safely into the Ring Buffer.
 */
void *receiver_thread_func(void *arg) {
    // Extract socket file descriptor from argument pointer
    int sockfd = *(int *)arg;
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("[Receiver] Thread started. Listening on port %d...\n", PORT);

    while (1) {
        /*
         * recvfrom blocks until a packet arrives.
         * This avoids busy-waiting and saves CPU cycles.
         */
        int n = recvfrom(sockfd, (char *)buffer, sizeof(buffer) - 1, 
                         0, (struct sockaddr *)&client_addr, &addr_len);
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate the received string
            
            telemetry_data_t data;
            /* 
             * Parse string formatted as "timestamp:value".
             * Example: "1672531200000:0.435"
             */
            if (sscanf(buffer, "%lu:%lf", &data.timestamp, &data.value) == 2) {
                // Thread-safe insertion into the ring buffer
                ring_buffer_push(&telemetry_buffer, data);
            }
        }
    }
    
    return NULL;
}

/*
 * Worker Thread:
 * Pops telemetry data from the Ring Buffer, applies a moving average 
 * DSP filter, prints the signal, and logs it to a CSV file.
 */
void *worker_thread_func(void *arg) {
    // Suppress unused parameter warning
    (void)arg; 

    printf("[Worker]   Thread started. Waiting for data...\n");

    // Open CSV log file for writing
    FILE *log_file = fopen("telemetry_log.csv", "w");
    if (log_file != NULL) {
        fprintf(log_file, "Timestamp,Raw,Filtered\n");
        fflush(log_file); // Ensure header is written immediately
    } else {
        fprintf(stderr, "[Worker] Warning: Could not open telemetry_log.csv for writing.\n");
    }

    while (1) {
        /*
         * Extract data from the buffer.
         * If empty, this function blocks until the receiver signals.
         */
        telemetry_data_t data = ring_buffer_pop(&telemetry_buffer);
        
        // Apply the Digital Signal Processing filter (Moving Average)
        double filtered_value = apply_moving_average(data.value);
        
        // Print the original vs. filtered results
        printf("[DSP] TS: %13lu | Raw: %7.3f | Filtered: %7.3f\n", 
               data.timestamp, data.value, filtered_value);

        // Log the data to the CSV file
        if (log_file != NULL) {
            fprintf(log_file, "%lu,%.6f,%.6f\n", data.timestamp, data.value, filtered_value);
            // Flush periodically to avoid data loss if the server is stopped abruptly (e.g. Ctrl+C)
            fflush(log_file); 
        }
    }

    // In this infinite loop design, this is unreachable. 
    // Included for completeness/good practice.
    if (log_file != NULL) {
        fclose(log_file);
    }

    return NULL;
}

int main(void) {
    // 1. Initialize the thread-safe ring buffer
    ring_buffer_init(&telemetry_buffer);

    // 2. Initialize the UDP server
    int sockfd = udp_server_init(PORT);
    if (sockfd < 0) {
        fprintf(stderr, "Failed to initialize UDP socket.\n");
        return EXIT_FAILURE;
    }

    // Thread identifiers
    pthread_t receiver_tid;
    pthread_t worker_tid;

    /* 
     * 3. Launch threads.
     * We pass the address of sockfd to the receiver thread.
     */
    if (pthread_create(&receiver_tid, NULL, receiver_thread_func, &sockfd) != 0) {
        perror("Failed to create receiver thread");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (pthread_create(&worker_tid, NULL, worker_thread_func, NULL) != 0) {
        perror("Failed to create worker thread");
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* 
     * 4. Wait for threads to finish.
     * In this daemon-like design, they run indefinitely. 
     */
    pthread_join(receiver_tid, NULL);
    pthread_join(worker_tid, NULL);

    // Cleanup (Unreachable in this infinite loop design, but good practice)
    close(sockfd);
    return EXIT_SUCCESS;
}
