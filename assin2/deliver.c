#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>     
#include <netdb.h>  
#include <netinet/in.h>        
#include <arpa/inet.h>      
#include <unistd.h>        
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 256

struct packet {
    unsigned int total_frag;       
    unsigned int frag_no;           
    unsigned int size;             
    char *filename;
    char filedata[1000];
	struct packet *next;
};

struct packet *get_packet_list(char *filename){
    FILE *file;
    int file_bytes, total_frag, size, frag_no;
    char data[1000];
    struct packet *head, *prev_packet;

    file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    file_bytes = ftell(file);
    fseek(file, 0, SEEK_SET);

	if(file_bytes % 1000 == 0)
		total_frag = file_bytes / 1000;
	else
		total_frag = (file_bytes / 1000) + 1;

    for (frag_no = 1; frag_no <= total_frag; frag_no++) {
        struct packet *new_packet = malloc(sizeof(struct packet));
        if(frag_no == 1){
            head = new_packet;
        }
        else{
            prev_packet->next = new_packet;
        }

        size = fread(data, 1, 1000, file);
		
        new_packet->total_frag = total_frag;
        new_packet->frag_no = frag_no;
        new_packet->size = size;
        new_packet->filename = filename;
        memcpy(new_packet->filedata, data, size);
        new_packet->next = NULL;
		
        prev_packet = new_packet;
    }

    fclose(file);
    return head;
}

char *get_packet_msg(struct packet *packet, int *msg_size){
    int s1 = snprintf(NULL, 0, "%d", packet->total_frag);
    int s2 = snprintf(NULL, 0, "%d", packet->frag_no);
    int s3 = snprintf(NULL, 0, "%d", packet->size);
    int s4 = strlen(packet->filename);
    int s5 = packet->size;
    int total_size = s1 + s2 + s3 + s4 + s5;

    char *msg = malloc((total_size + 4)*sizeof(char));

    int header_size = sprintf(msg, "%d:%d:%d:%s:",
                                packet->total_frag,
                                packet->frag_no,
                                packet->size,
                                packet->filename);

    memcpy(&msg[header_size], packet->filedata, packet->size);

    *msg_size = header_size + packet->size;
    return msg;
}

void free_packet_list(struct packet *head){
    struct packet *curr_packet = head;
    struct packet *prev_packet = head;
    while (curr_packet->next != NULL) {
        curr_packet = curr_packet->next;
        free(prev_packet);
        prev_packet = curr_packet;
    }
}

int main(int argc, char **argv)
{
    
    if (argc < 3) {
       printf("No server address or port number provided.\n");
       exit(1);
    }
	
	int sock_fd, recv_n, send_n;
	int port = atoi(argv[2]);
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char file_name[BUFFER_SIZE];
	
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0){
        printf("ERROR opening socket.\n");
		exit(1);
	}
    
    serv_addr.sin_family = AF_INET;
    if(inet_aton(argv[1], &serv_addr.sin_addr) == 0){
        printf("ERROR obtaining server address.\n");
        exit(1);
    }
    serv_addr.sin_port = htons(port);
	bzero(&(serv_addr.sin_zero), 8);
	
    printf("Please input a message in the form of ftp <filename>: ");

    scanf("%s %s", buffer, file_name);

    if(strcmp(buffer, "ftp") != 0){
        printf("Invalid command: %s\n", buffer);
        exit(1);
    }
   
    if(access(file_name, F_OK) == -1) {
		printf("File \"%s\" doesn't exist.\n", file_name);
        exit(1);
    }   
    
    socklen_t addr_len = sizeof(serv_addr);
	
	clock_t start, end;
	start = clock();
    send_n = sendto(sock_fd, "ftp", strlen("ftp"), 0, (struct sockaddr *)&serv_addr, addr_len);
    if(send_n < 0){
		printf("ERROR: Sending failed.\n");
        exit(1);
	}
	
	bzero(buffer, BUFFER_SIZE);
	recv_n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
	end = clock();
	printf("RTT: %f\n", ((double) (end - start) / CLOCKS_PER_SEC));
	if(recv_n < 0){
		printf("ERROR: Receiving failed.\n");
        exit(1);
	}
	if(strcmp(buffer, "yes") == 0)
		printf("A file transfer can start.\n");
	else 
		exit(1);
	
	struct packet *head = get_packet_list(file_name);
	
	int msg_size = 0;
	bzero(buffer, BUFFER_SIZE);
	struct packet *curr_packet = head;
	while(curr_packet != NULL){
		char *msg = get_packet_msg(curr_packet, &msg_size);
		if(sendto(sock_fd, msg, msg_size, 0, (struct sockaddr *)&serv_addr, addr_len) == -1)
			printf("Error sending packet %d\n", curr_packet->frag_no);
		if(recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &serv_addr, &addr_len) == -1)
			printf("Error receiving ACK for packet %d\n", curr_packet->frag_no);
		
		if (strcmp(buffer, "NACK") == 0) {
            printf("NACK! Resend packet %d\n", curr_packet->frag_no);
            continue;
		}
		
		printf("Packet #%d Sent.\n", curr_packet->frag_no);

		curr_packet = curr_packet->next;
		free(msg);
    }
	
    free_packet_list(head);
	close(sock_fd);
	return 0;
	
}