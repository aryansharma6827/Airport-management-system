#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_FLIGHTS 10
#define MAX_RUNWAYS 10
#define MAXLEN 512

pthread_mutex_t runway_mutex[MAX_RUNWAYS];
pthread_mutex_t backup_runway_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;

struct Runway {
    int loadCap;
};

struct msg_buffer {
    long msg_type;
    int plane_no;
    int msg_text[MAXLEN];
};

struct details {
    int aair;
    int dair;
    int pid;
    int totalWeight;
    int ptype;
    int seats;
};


int AirportNo;
int No_of_Runways;
int msgid;
struct Runway R[MAX_RUNWAYS];

void* takeoff(void* arg) {
    struct msg_buffer message = *(struct msg_buffer*)arg;
    struct details tmp;
    
    tmp.aair = message.msg_text[0];
    tmp.dair = message.msg_text[1];
    tmp.pid = message.msg_text[2];
    tmp.totalWeight = message.msg_text[3];
    tmp.ptype = message.msg_text[4];
    tmp.seats = message.msg_text[5];

    if (tmp.totalWeight > 12000) {
        if (tmp.totalWeight > 15000) {
            printf("Sorry, the plane is beyond the allowable weight limit. Departure aborted.\n");
            return NULL;
        }
        // Lock the mutex for the backup runway
        pthread_mutex_lock(&backup_runway_mutex);

        sleep(2); // Landing time
        // Send message to the message queue
        message.msg_type = 160 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Deboarding/unloading
        printf("Plane %d has completed boarding/loading and taken off from the Backup Runway of Airport No. %d .\n", tmp.pid,AirportNo);
        
        // Unlock the mutex for the backup runway
        pthread_mutex_unlock(&backup_runway_mutex);
        return NULL;
    }
    int closest = 20000;
    int index = -1;
    sem_wait(&semaphore); // Wait for the semaphore
    int i;
    for (i = 0; i < No_of_Runways; i++) {
        if (pthread_mutex_trylock(&runway_mutex[i]) == 0) { // Try to lock the runway
            int m = R[i].loadCap - tmp.totalWeight;
            if (m < closest && m >= 0) {
                closest = m;
                index = i;
            }
        }
    }
    if (index != -1) { // If a suitable runway is found
        sleep(2); // Takeoff time
        // Send message to the message queue
        message.msg_type = 160 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Boarding/Loading
        printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d .\n",tmp.pid, index + 1, AirportNo);
        pthread_mutex_unlock(&runway_mutex[i]); // Unlock the runway
    } else {
        // Lock the mutex for the backup runway
        pthread_mutex_lock(&backup_runway_mutex);

        sleep(2); // Takeoff time
        // Send message to the message queue
        message.msg_type = 160 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Boarding/Loading
        printf("Plane %d has completed boarding/loading and taken off from the Backup Runway of Airport No. %d .\n",tmp.pid, AirportNo);
        
        // Unlock the mutex for the backup runway
        pthread_mutex_unlock(&backup_runway_mutex);
        pthread_mutex_unlock(&runway_mutex[i]); // Unlock the runway
    }
    sem_post(&semaphore); // Signal semaphore
    return NULL;
}


void* land(void* arg) {
    struct msg_buffer message = *(struct msg_buffer*)arg;
    struct details tmp;
    
    tmp.aair = message.msg_text[0];
    tmp.dair = message.msg_text[1];
    tmp.pid = message.msg_text[2];
    tmp.totalWeight = message.msg_text[3];
    tmp.ptype = message.msg_text[4];
    tmp.seats = message.msg_text[5];

    if (tmp.totalWeight > 12000) {
        if (tmp.totalWeight > 15000) {
            printf("Sorry, the plane is beyond the allowable weight limit. Arrival aborted.\n");
            return NULL;
        }
        // Lock the mutex for the backup runway
        pthread_mutex_lock(&backup_runway_mutex);

        sleep(2); // Landing time
        // Send message to the message queue
        message.msg_type = 180 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Deboarding/unloading
        printf("Plane %d has landed on the Backup Runway of Airport No. %d and has completed deboarding/unloading.\n", tmp.pid,AirportNo);
        
        // Unlock the mutex for the backup runway
        pthread_mutex_unlock(&backup_runway_mutex);
        return NULL;
    }
    int closest = 20000;
    int index = -1;
    sem_wait(&semaphore); // Wait for the semaphore
    int i;
    for (i = 0; i < No_of_Runways; i++) {
        if (pthread_mutex_trylock(&runway_mutex[i]) == 0) { // Try to lock the runway
            int m = R[i].loadCap - tmp.totalWeight;
            if (m < closest && m >= 0) {
                closest = m;
                index = i;
            }
        }
    }
    if (index != -1) { // If a suitable runway is found
        sleep(2); // Landing time
        // Send message to the message queue
        message.msg_type = 180 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Deboarding/unloading
        printf("Plane %d has landed on Runway No. %d of Airport No. %d and has completed deboarding/unloading.\n",tmp.pid, index + 1, AirportNo);
        pthread_mutex_unlock(&runway_mutex[i]); // Unlock the runway
    } else {
        // Lock the mutex for the backup runway
        pthread_mutex_lock(&backup_runway_mutex);

        sleep(2); // Landing time
        // Send message to the message queue
        message.msg_type = 180 + AirportNo;
        if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
            printf("Error in sending message");
            exit(1);
        }
        sleep(3); // Deboarding/unloading
        printf("Plane %d has landed on the Backup Runway of Airport No. %d and has completed deboarding/unloading.\n",tmp.pid, AirportNo);
        
        // Unlock the mutex for the backup runway
        pthread_mutex_unlock(&backup_runway_mutex);
        pthread_mutex_unlock(&runway_mutex[i]); // Unlock the runway
    }
    sem_post(&semaphore); // Signal semaphore
    return NULL;
}

int main() {
    
    //The input taken from user
    printf("Enter Airport Number: ");
    scanf("%d", &AirportNo);

    printf("Enter number of Runways: ");
    scanf("%d", &No_of_Runways);

    printf("Enter loadCapacity of Runways (give as a space separated list in a single line): \n");
    for (int i = 0; i < No_of_Runways; i++) {
        scanf("%d", &R[i].loadCap);
    }

    for (int i = 0; i < MAX_RUNWAYS; i++) {
        if (pthread_mutex_init(&runway_mutex[i], NULL) != 0) {
            perror("Mutex initialization failed");
            exit(1);
        }
    }

    if (sem_init(&semaphore, 0, 1) != 0) {
        perror("Semaphore initialization failed");
        exit(1);
    }

    struct msg_buffer message;
    key_t key;
    int msgid;

    system("touch m.txt");

    key = ftok("m.txt", 'A');
    if (key == -1){
        printf("error in creating unique key\n");
        exit(1);
    }

    msgid = msgget(key, 0644);    
    if (msgid == -1){
        printf("error in creating message queue\n");
        exit(1);
    }

    int weight;

    pthread_t tid[MAX_FLIGHTS];
    int i;


    for (i = 0; i < MAX_FLIGHTS; i++) {
    pid_t p = fork();

    if (p < 0) {
        perror("Error in fork.");
    } else if (p == 0) {
        if (msgrcv(msgid, (void *)&message, sizeof(message), 120 + AirportNo, 0) == -1) {
            printf("error in receiving message\n");
            exit(1);
        }
        if (pthread_create(&tid[i], NULL, takeoff, (void *)&message) != 0) {
            perror("Thread creation failed");
            exit(1);
        }
    } else {
        if (msgrcv(msgid, (void *)&message, sizeof(message), 140 + AirportNo, 0) == -1) {
            printf("error in receiving message\n");
            exit(1);
        }
        if (pthread_create(&tid[i], NULL, land, (void *)&message) != 0) {
            perror("Thread creation failed");
            exit(1);
        }
    }
}



    for (int j = 0; j <= i; j++) {
        if (pthread_join(tid[j], NULL) != 0) {
            perror("Thread joining failed");
            exit(1);
        }
    }

    for (int i = 0; i < MAX_RUNWAYS; i++) {
        if (pthread_mutex_destroy(&runway_mutex[i]) != 0) {
            perror("Mutex destruction failed");
            exit(1);
        }
    }

    if (sem_destroy(&semaphore) != 0) {
        perror("Semaphore destruction failed");
        exit(1);
    }

    return 0;
}
