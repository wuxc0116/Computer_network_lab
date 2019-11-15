#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 256
#define MAXDATASIZE 1100

struct packet {
    unsigned int total_frag;       
    unsigned int frag_no;           
    unsigned int size;             
    char *filename;
    char filedata[1000];
	struct packet *next;
};

struct packet* get_packet(char *packet_str){
	struct packet *new_packet;
    char *total_frag_str, *frag_no_str, *size_str, *filename, *filedata;
    int total_frag, frag_no, size;

    total_frag_str = strtok(packet_str, ":");
    frag_no_str = strtok(NULL, ":");
    size_str = strtok(NULL, ":");
    filename = strtok(NULL, ":");

    total_frag = atoi(total_frag_str);
    frag_no = atoi(frag_no_str);
    size = atoi(size_str);

    int header_size = strlen(total_frag_str) + strlen(frag_no_str) + strlen(size_str) + strlen(filename) + 4;

    filedata = malloc(size * sizeof(char));
    memcpy(filedata, &packet_str[header_size], size);


    new_packet = (struct packet *)malloc(sizeof(struct packet));
    new_packet->total_frag = total_frag;
    new_packet->frag_no = frag_no;
    new_packet->size = size;
    new_packet->filename = filename;
    memcpy(new_packet->filedata, filedata, size);
    if(size < 1000){
        new_packet->filedata[size] = '\0';
    }

    return new_packet;
}


int main(int argc, char ** argv){
	if(argc < 2){
		printf("ERROR: No port provided.\n");
		exit(1);
	}
	int port = atoi(argv[1]);
	int sock_fd, addr_len, recv_n, send_n;
	struct sockaddr_in serv_addr, cli_addr;
    char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);
   
    sock_fd	= socket(AF_INET, SOCK_DGRAM, 0);
	if(sock_fd < 0){
		printf("ERROR: Opening socket failed.\n");
        exit(1);
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);
	
	if(bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0){
		printf("ERROR: Binding failed.\n");
        exit(1);
	}
	
	addr_len = sizeof(cli_addr);
	
	recv_n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cli_addr, &addr_len);
	if(recv_n < 0){
		printf("ERROR: Receiving failed.\n");
        exit(1);
	}
	
	if(strcmp(buffer, "ftp") == 0)
		send_n = sendto(sock_fd, "yes", strlen("yes"), 0, (struct sockaddr *)&cli_addr, addr_len);
	else
		send_n = sendto(sock_fd, "no", strlen("no"), 0, (struct sockaddr *)&cli_addr, addr_len);
	if(send_n < 0){
		printf("ERROR: Sending failed.\n");
        exit(1);
	}
    
	FILE *file;
    int flag = 1;
	char message[MAXDATASIZE];
    while(flag){
        recv_n = recvfrom(sock_fd, message, MAXDATASIZE - 1, 0, (struct sockaddr *)&cli_addr, &addr_len);
        message[recv_n] = '\0';

        struct packet *curr_packet = get_packet(message);
        int total_frag = curr_packet->total_frag;
        int frag_no = curr_packet->frag_no;
        int size = curr_packet->size;
        char * filename = curr_packet->filename;
        strcat(filename, " copy");
        char * filedata = curr_packet->filedata;
        printf("Packet #%d Recieved...\n", frag_no);

        if(frag_no == 1){
            file = fopen(filename, "wb");
        }
        fwrite(filedata, 1, size, file);

        if(frag_no == total_frag){
                flag = 0;
        }

        send_n = sendto(sock_fd, "ACK", strlen("ACK"), 0, (struct sockaddr *)&cli_addr, addr_len);

        free(curr_packet);
    }
    fclose(file);

	close(sock_fd);
	
	return 0;
}