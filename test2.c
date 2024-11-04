#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "aq.h"

AlarmQueue queue;
pthread_t normal_sender, alarm_sender, receiver;

void* normal_message_sender(void* arg) {
    // Send several normal messages first
    for (int i = 1; i <= 3; i++) {
        char* msg = malloc(20);
        sprintf(msg, "Normal%d", i);
        printf("NormalSender: Sending message: %s\n", msg);
        aq_send(queue, msg, AQ_NORMAL);
        usleep(50000);  // 50ms delay between sends
    }
    return NULL;
}

void* alarm_message_sender(void* arg) {
    // Wait for some normal messages to be queued
    usleep(100000);  // 100ms
    
    char* alarm_msg = strdup("Alarm1");
    printf("AlarmSender: Sending alarm message: %s\n", alarm_msg);
    aq_send(queue, alarm_msg, AQ_ALARM);
    
    return NULL;
}

void* message_receiver(void* arg) {
    // Wait to ensure messages are in queue
    usleep(200000);  // 200ms
    
    // Receive and track order of messages
    int msg_count = 0;
    while (msg_count < 4) {  // Expect 3 normal + 1 alarm
        void* msg;
        int kind = aq_recv(queue, &msg);
        if (kind == AQ_NO_MSG) break;
        
        printf("Receiver: Received message %d: %s, type: %d\n", 
               msg_count + 1, (char*)msg, kind);
        
        free(msg);
        msg_count++;
        usleep(50000);  // 50ms between receives
    }
    return NULL;
}

int main() {
    queue = aq_create();
    if (!queue) {
        printf("Failed to create alarm queue\n");
        return 1;
    }
    
    // Create threads
    pthread_create(&normal_sender, NULL, normal_message_sender, NULL);
    pthread_create(&alarm_sender, NULL, alarm_message_sender, NULL);
    pthread_create(&receiver, NULL, message_receiver, NULL);
    
    // Wait for all threads to complete
    pthread_join(normal_sender, NULL);
    pthread_join(alarm_sender, NULL);
    pthread_join(receiver, NULL);
    
    printf("Test completed.\n");
    return 0;
}