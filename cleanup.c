#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>

#define MAXLEN 50

struct msg_buffer {
    long msg_type;
    int plane_no;
    int msg_text[MAXLEN];
};

int main(){
    key_t key;
    int msgid;
    char buf[MAXLEN];
    char o = 'A';
    key = ftok("m.txt", o);
    if (key == -1) {
        printf("Error in creating unique key\n");
        exit(1);
    }

    struct msg_buffer cleanup;

    msgid = msgget(key, 0644);
    if (msgid == -1) {
        printf("Error in creating message queue\n");
        exit(1);
    }

    char choice;
    cleanup.msg_type = 900;

    while(1){
        printf("Do you want the Air Traffic Control System to terminate ? (Enter Y/y for Yes and N/n for No)");
        scanf("%c",&choice);

        if(choice == 'N' || choice == 'n'){
            cleanup.msg_text[0] = 1;
            if (msgsnd(msgid, (void *)&cleanup, sizeof(cleanup) - sizeof(long), 0) == -1) {
            printf("Error in sending message %s", buf);
            exit(1);
            }
        }else if(choice == 'Y' || choice == 'y'){
            cleanup.msg_text[0] = 0;
            if (msgsnd(msgid, (void *)&cleanup, sizeof(cleanup) - sizeof(long), 0) == -1) {
            printf("Error in sending message %s", buf);
            exit(1);
            }
            break;
        }else{
            printf("Invalid input.");
        }
    }

    return 0;
}