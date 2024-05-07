#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define buffer_size 25
#define READ_END 0
#define WRITE_END 1
#define MAXLEN 512 

struct msg_buffer {
    long msg_type;
    int plane_no;
    int msg_text[MAXLEN];
};

int main() {
    struct msg_buffer message;
    key_t key;
    int msgid;
    char buf[MAXLEN];
    char o = 'A';

    system("touch m.txt");

    key = ftok("m.txt", o);
    if (key == -1) {
        printf("Error in creating unique key\n");
        exit(1);
    }

    int pid;
    int details[6];
    int ptype;
    int ano;
    char writeMsg[buffer_size];
    char readMsg[buffer_size];

    printf("Enter Plane ID:");
    scanf("%d", &pid);
    printf("Enter Type of Plane:");
    scanf("%d", &ptype);

    int aano;

    if (ptype == 1) {
        int seats;

        printf("Enter Number of Occupied Seats:");
        scanf("%d", &seats);

        char s[seats][5];
        char s1[seats][5];
        pid_t p;
        char lugwt[seats][2];
        char bwt[seats][3];
        int lugw[seats];
        int bw[seats];
        int fd[seats][2];

        for (int i = 0; i < seats; i++) {
            pipe(fd[i]);
            p = fork();

            if (p < 0) {
                printf("Error");
            } else if (p == 0) {
                int wt = 0;
                int bwt = 0;

                printf("Enter Weight of Your Luggage:");
                scanf("%d", &wt);
                printf("Enter Your Body Weight:");
                scanf("%d", &bwt);

                int j = 0;

                if (wt <= 9)
                    j = 1;
                else
                    j = 2;

                int twt = bwt * 100 + wt;

                sprintf(s[i], "%d", twt);
                close(fd[i][READ_END]);
                write(fd[i][WRITE_END], s[i], 5);
                close(fd[i][WRITE_END]);

                exit(0);
            } else {
                char ch[5];
                close(fd[i][WRITE_END]);
                read(fd[i][READ_END], readMsg, buffer_size);
                strcpy(ch, readMsg);
                close(fd[i][READ_END]);

                int k;
                for (k = 0; ch[k] != '\0'; ++k);

                if (ch[k - 2] == '0') {
                    lugwt[i][0] = ch[k - 1];
                } else {
                    lugwt[i][0] = ch[k - 2];
                    lugwt[i][1] = ch[k - 1];
                }

                lugw[i] = atoi(lugwt[i]);

                for (int j = 0; j < k - 2; j++) {
                    bwt[i][j] = ch[j];
                }

                bw[i] = atoi(bwt[i]);
            }
        }

        for (int i = 0; i < seats; i++) {
            wait(NULL);
        }

        int totalWeight = 0;

        for (int i = 0; i < seats; i++) {
            totalWeight = totalWeight + bw[i] + lugw[i];
        }

        totalWeight = totalWeight + 7 * 75;

        printf("Enter Airport Number for Departure: ");
        scanf("%d", &ano);

        printf("Enter Airport Number for Arrival: ");
        scanf("%d", &aano);

        details[0] = aano;
        details[1] = ano;
        details[2] = pid;
        details[3] = totalWeight;
        details[4] = ptype;
        details[5] = seats;
    } else {
        int citem;
        int avgwt;
        int totalWeight = 0;

        printf("Enter Number of Cargo Items: ");
        scanf("%d", &citem);
        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d", &avgwt);

        totalWeight = citem * avgwt + 4 * 75;

        printf("Enter Airport Number for Departure: ");
        scanf("%d", &ano);
        printf("Enter Airport Number for Arrival: ");
        scanf("%d", &aano);

        details[0] = aano;
        details[1] = ano;
        details[2] = pid;
        details[3] = totalWeight;
        details[4] = ptype;
        details[5] = -1;
    }

    for (int i = 0; i < 6; i++) {
        message.msg_text[i] = details[i];
    }

    message.msg_type = 100;
    message.plane_no = pid;

    msgid = msgget(key, 0644);
    if (msgid == -1) {
        printf("Error in creating message queue\n");
        exit(1);
    }

    if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
        printf("Error in sending message %s", buf);
        exit(1);
    }

    if (msgrcv(msgid, (void *)&message, sizeof(message) - sizeof(long), 200 + pid , 0) == -1) {
        printf("Error in receiving message\n");
        exit(1);
    }

    sleep(30); //Flight Duration

    message.msg_type = 400;

    if (msgsnd(msgid, (void *)&message, sizeof(message) - sizeof(long), 0) == -1) {
        printf("Error in sending message %s", buf);
        exit(1);
    }

    if (msgrcv(msgid, (void *)&message, sizeof(message) - sizeof(long), 300 + pid , 0) == -1) {
        printf("Error in receiving message\n");
        exit(1);
    }

    printf("Plane %d has successfully traveled from Airport %d to Airport %d!",pid ,ano ,aano);

    return 0;
}
