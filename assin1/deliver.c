/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define MAXBUFLEN 255

int main(int argc, char const *argv[]){
    if(argc != 3){
        fprintf(stderr,"lack for argument.\n");
        exit(1);
    }
    
    int sockfd;
    char buffer[MAXBUFLEN] = {0};
    
    struct sockaddr_in serv_addr;
    
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        perror("socket error");
        exit(1);
    }
    
    memset(& serv_addr, 0, sizeof(serv_addr));

    int PORT_num = atoi(argv[2]);
    
    //Filling in the server information
    serv_addr.sin_family    = AF_INET; // IPv4 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(PORT_num); 
    
    
    //get the user input
    char ftp_name[MAXBUFLEN];
    char filename[MAXBUFLEN];
    scanf("%s", ftp_name);
    scanf("%s", filename);
    
    // check the file exists or not
    int access_flag = access(filename, F_OK);
    FILE * file_pointer = NULL;
    file_pointer = fopen(filename, "r");
    
    if(access_flag == -1){
        perror("file does not exit");
        exit(1);
        
    } else {
        //if the file exist, send "ftp" to server
        if(sendto(sockfd, "ftp", strlen("ftp"), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            perror("send_to error.");
    }
    
    
    //rec from
    memset(buffer, 0, MAXBUFLEN);
    socklen_t serve_addr_size = sizeof(serv_addr);
    if((recvfrom(sockfd, buffer, MAXBUFLEN, 0, (struct sockaddr *) &serv_addr, &serve_addr_size)) == -1) {
        perror("rec from error");
        exit(1);
    }
    
    if(strcmp(buffer, "yes") == 0){
        fprintf(stdout, "A file transfer can start.\n");
    } else {
        perror("File transfered declined");
        exit(1);
    }
    
    close(sockfd);
    
    return 0;
    
}