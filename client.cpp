#include<iostream>
#include<string.h>
#include<thread>
#include<stdlib.h>
#include<stdio.h> 
#include<sys/ipc.h> 
#include<sys/msg.h>
#include<unistd.h> 

enum status_code { freee = 1, locked, unlocked, valid, invalid, delay_write,old };

struct mesg_buffer { 
    long mesg_type;
    int pid = getpid();
    int choice;
    int dev_no;
    int blk_no;
} message; 

int main() {
    key_t key; 
    int msgid; 
    // ftok to generate unique key 
    key = ftok("progfile", 65); 
  
    // msgget creates a message queue 
    // and returns identifier 
    msgid = msgget(key, 0666 | IPC_CREAT); 
    message.mesg_type = 1; 
    
    
    while(message.choice!=3){
        std::cout<<"\n1. Process needs a block \n2. Process wants to release a block\n3. Terminate the process\nEnter your choice: ";
        std::cin>>message.choice;
        switch(message.choice){
            case 1:
                std::cout<<"Enter Device number: ";
                std::cin>>message.dev_no;
                std::cout<<"Enter Block number: ";
                std::cin>>message.blk_no;
                break;
            case 2:
                std::cout<<"\nEnter device number and block number you want to release ";
                std::cin>>message.dev_no>>message.blk_no;
                break;
            case 3:
                break;
            default:
                std::cout<<"\nYou entered the wrong choice ";
                break;
        }
    
  
        // msgsnd to send message 
        msgsnd(msgid, &message, sizeof(message), 0);
        

    }

    return 0;
  
}