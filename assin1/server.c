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
    
    if(argc<2){
        //argv[0] is file name
        //argv[1] is port number, 
        fprintf(stderr, "Port Number not provided. Program terminated\n");
        exit(1);
    }
    
    int sockfd;
    char buffer[MAXBUFLEN] = {0};
    
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t client_len; // length of client info
    
    
    // start socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0){
        fprintf(stderr, "error opening socket.");
        exit(1);
    } //now socket has been opened
    
    memset(& serv_addr, 0, sizeof(serv_addr));
    memset(& serv_addr, 0, sizeof(cli_addr));
    int PORT_num = atoi(argv[1]);
    
    //Filling in the server information
    serv_addr.sin_family    = AF_INET; // IPv4 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(PORT_num); 
    
    //bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) {
        perror("bind failed");
        exit(1);
    }
    
    
    //recvfrom()
    int recv_flag = 1;
//    while(recv_flag == 1){
//        if(recvfrom(sockfd, buffer, MAXBUFLEN-1, 0, (struct sockaddr *)&cli_addr, &client_len) == -1 ){
//            perror("recvfrom error");
//            recv_flag = 0;
//            exit(1);
//        }
//    }
    
    if(recvfrom(sockfd, buffer, MAXBUFLEN-1, 0, (struct sockaddr *)&cli_addr, &client_len) == -1 ){
            perror("recvfrom error");
            exit(1);
        }

    
    //sendto()
    if(strcmp(buffer, "ftp") == 0){
        if(sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *) &cli_addr, sizeof(serv_addr)) == -1){
            perror("send to error");
            exit(1);
        } else{
            printf("success\n");
        }
    } else { // else if the message is not "ftp"
        if(sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *) &cli_addr, sizeof(serv_addr)) == -1){
            perror("send_to error");
            exit(1);
        } else {
            printf("success\n");
        }
    }
    
    close(sockfd);
    return 0;
    
}
 


