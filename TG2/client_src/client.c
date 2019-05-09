
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <signal.h>
#include "../constants.h"
#include "../types.h"
#include "../log.c"


#define ERR_ARGS 2
#define ERR_FIFO 3
#define SERVER_FIFO_ERROR 4
#define MESSAGE_SENT_ERROR 5

#define MAX_LINE_LENGTH 512
#define WIDTH_FIFO_NAME 16

typedef unsigned int u_int;

void print_usage(FILE *stream)
{
    fprintf(stream, "Usage: ./client <Account ID> <\"Password\"> <Operation Delay (ms)> <Operation Code> <\"Operation Arguments\">\n");
}

void send_message(tlv_request_t message){
    int fd;
   /* printf("msg: %d\n", message.length);
    printf("msg: %s\n", message.value.header.password);

    printf("msg: %ld\n", strlen(message.value.header.password));
    printf(" msg: %d\n", message.value.header.account_id);*/


    if ((fd = open(SERVER_FIFO_PATH, O_WRONLY | O_CREAT | O_APPEND, 0660)) < 0){
        printf("Failed to open server requests FIFO\n");
        exit(SERVER_FIFO_ERROR);
    } 
    else {
        printf("Open Successfully");
    }

    int write_result = write(fd, &message, sizeof(message));
    if (write_result < 0){
        printf("Error: Failed to send message to server FIFO\n");
        exit(MESSAGE_SENT_ERROR);
    }
    close(fd);
}

void alarm_hanlder(){
    printf(" timeout, no response from server ");
    //TODO write in log
    exit(0);
}
void setup_alarm(){
    signal(SIGALRM, alarm_hanlder);
}

void receive_message(char *strPid){

    int fd, fd_dummy;
    char line[MAX_LINE_LENGTH];
    int optype; 
    
    setup_alarm();
    alarm(FIFO_TIMEOUT_SECS);

    char fifo_name[USER_FIFO_PATH_LEN] = "\0";
    strcpy(fifo_name, USER_FIFO_PATH_PREFIX);
    strcat(fifo_name, strPid);
    if ((fd=open(fifo_name,O_RDONLY)) !=-1)
    printf("Reply FIFO openned in READONLY mode\n");
    if ((fd_dummy=open(fifo_name,O_WRONLY)) !=-1)
    printf("Reply FIFO openned in WRITEONLY mode\n");
    
    do {
      read(fd,&optype,sizeof(int));
      if (optype >=0 || optype < 4) {
        read(fd,line,MAX_LINE_LENGTH);
 printf("line %s\n",line);
 }
 } while (optype!=0);
    tlv_reply_t *message_received;
    // reconstruct_message(line, message_received);
   
    //int log_result = logReply(fifo_response, getpid(), *message_received); 
   


}



/*void setWidth(char *arg[], int width){
    
    strcat(account_ID, argv[1]);
    while((strlen(account_final) + strlen(account_ID)) <= WIDTH_ACCOUNT - 1){
        strcat(account_final, "0");
    }
    strcat(account_final, account_ID);
}*/

int main(int argc, char *argv[]) {

    if (argc != 6)
    {
        print_usage(stderr);
        exit(ERR_ARGS);
    }

    //************Prepare arguments*******************

    req_header_t header;
    req_create_account_t createAccount;
    req_transfer_t transfer;
    req_value_t value;
    tlv_request_t message_send;

    //PID
    header.pid = getpid();
    char strPid[WIDTH_ID];
    sprintf(strPid, "%d", getpid());

    //Account ID
    u_int account_number = atoi(argv[1]);
    if (account_number < 1 || account_number > MAX_BANK_ACCOUNTS)
    {
        printf("Error: Invalid Account Number. Use: 1 < Account Number < 4096\n");
        print_usage(stderr);
        exit(ERR_ARGS);
    }
    header.account_id = account_number;

    //Password
    if (strlen(argv[2]) < MIN_PASSWORD_LEN || strlen(argv[2]) > MAX_PASSWORD_LEN)
    {
        printf("Error: Invalid Password. Must have more than 8 and less than 20 characters.\n");
        print_usage(stderr);
        exit(ERR_ARGS);
    }
    strcpy(header.password, argv[2]);

    //Delay
    u_int delay = atoi(argv[3]);
    if (delay > MAX_OP_DELAY_MS || delay <= 0)
    {
        printf("Error: Invalid delay. Use delay between 0 and 99999.\n");
        print_usage(stderr);
        exit(ERR_ARGS);
    }
    header.op_delay_ms = delay;

    /*Operation Code
    * 0 - Account creation: "AccountID balance "password"" 
    * 1 - Balance consultation: no extra arguments
    * 2 - Between Accounts Transfer: "destinationAccountID value"
    * 3 - Server ShutDown: no extra arguments
    */
    int op_code = atoi(argv[4]);
    if (op_code < 0 || op_code >= __OP_MAX_NUMBER)
    {
        printf("Invalid Operation Code. Use 0, 1, 2 or 3.\n");
        print_usage(stderr);
        exit(ERR_ARGS);
    }
    //struct header is finished

    //Operation Arguments
    char op_arguments[MAX_LINE_LENGTH];
    char argList[3][MAX_LINE_LENGTH];
    memcpy(op_arguments, argv[5], strlen(argv[5]) + 1);
    char *ptr;
    ptr = strtok(op_arguments, " ");
    int count = 0;
    while (ptr != NULL)
    {
        strcpy(argList[count], ptr);
        ptr = strtok(NULL, " ");
        count++;
    }
    //operation arguments list is ready


    switch(op_code){
        case(OP_BALANCE):
            value.header = header;
            break;
        case(OP_SHUTDOWN):
            value.header = header;
            break;
        case(OP_CREATE_ACCOUNT):
          if (atoi(argList[0]) < 1 || atoi(argList[0]) > MAX_BANK_ACCOUNTS){
            printf("Invalid Destination Account ID. Use: 1 <= Account ID <= 4096\n");
            print_usage(stderr);
            exit(ERR_ARGS);
        }
          else if (atoi(argList[1]) < MIN_BALANCE || atoi(argList[1]) > MAX_BALANCE){
            printf("Invalid account balance. Use 1 <= Account Balance <= 1000000000\n");
            print_usage(stderr);
            exit(ERR_ARGS);
        }
          else if (strlen(argList[2]) < MIN_PASSWORD_LEN || strlen(argList[2]) > MAX_PASSWORD_LEN){
            printf("Error: Invalid Password. Must have more than 8 and less than 20 characters.\n");
            print_usage(stderr);
            exit(ERR_ARGS);
        }
          else{
            value.header = header;
            createAccount.account_id = atoi(argList[0]);
            createAccount.balance = atoi(argList[1]);
            strcpy(createAccount.password, argList[2]);
            value.create = createAccount;
        }//createAccount header struct is ready
        case(OP_TRANSFER):
          if (atoi(argList[0]) < 1 || atoi(argList[0]) > MAX_BANK_ACCOUNTS){
            printf("Invalid Destination Account ID. Use: 1 <= Account ID <= 4096\n");
            print_usage(stderr);
            exit(ERR_ARGS);
          }
          else if (atoi(argList[1]) < MIN_BALANCE || atoi(argList[1]) > MAX_BALANCE){
            printf("Invalid account balance. Use 1 <= Account Balance <= 1000000000\n");
            print_usage(stderr);
            exit(ERR_ARGS);
          }
        else{
            value.header = header;
            transfer.account_id = atoi(argList[0]);
            transfer.amount = atoi(argList[1]);
            value.transfer = transfer;
        }// transfer header struct ready
    }

    message_send.value = value;
    message_send.type = op_code;
    message_send.length = sizeof(message_send);
    //message TLV is ready to be sent!

    //*************************************************


    send_message(message_send);

return 0;
    receive_message(strPid);

    
}
