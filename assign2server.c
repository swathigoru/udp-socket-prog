#include <stdio.h>	/* for printf() and fprintf() */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "assignheader.h"

int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in srv_addr;
	struct sockaddr_in clnt_addr;
	unsigned int clnt_addr_len;
	unsigned short srv_port;
	int recv_msg_size;
	char line[MAX_BUFFER_LENGTH];
	struct access_pkt *result = NULL;
	const char delimiter[2] = ":";

    /* Intitalize recieve and send buffers */ 
	char *recv_buf = NULL;
    recv_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	char *send_buf = NULL;
    send_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	srv_port = atoi(argv[1]); //converting string to integer

	/*creating socket*/
	if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* construct local address structure */

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	srv_addr.sin_port = htons(srv_port);

	/*bind to the local address*/
	if (bind(sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) < 0)
		DieWithError("bind() failed");

	for (;;) {/*infinite loop*/
		
		clnt_addr_len = sizeof(clnt_addr);
		memset(recv_buf, 0, MAX_BUFFER_LENGTH);

		if((recv_msg_size = recvfrom(sock, recv_buf, MAX_BUFFER_LENGTH, 0, (struct sockaddr *) &clnt_addr, &clnt_addr_len)) < 0) 
			DieWithError("recvfrom() failed");
        struct access_pkt *access = (struct access_pkt *) recv_buf;
        if (access->access_flag != SUBSCRIBER_ACCESS_PERMISSION) {
        	fprintf(stderr,"\n Incorrect packet format.\n");
        	continue;
        }

        /* construct string from the subscriber number adding '-' where ever necessary */
        uint32_t number = access->payload.subscriber_num;
        char buf[13];
        buf[12] ='\0';
        for(int i = 11; i >= 0; i--) {
        	if ((i == 7) || (i == 3)) { // We need '-' at 3 and 7 index
        		buf[i] = '-';
        		continue;
        	}
        	buf[i] = number%10 +'0';
        	number = number/10;
        }
 

        fprintf(stderr,"\n Recieved packet from client for subscriber number :%s", buf);
        FILE *file = fopen("verification_database.txt", "r" ); //Open verification_database file
        char line[MAX_BUFFER_LENGTH];
        int paid = -1; // Default is -1 which indicates the subscriber doesnt exist
    	while (fgets(line, sizeof(line), file)) { // Read each line from the file
    		char *token;
    		token = strtok(line, delimiter); //Split the line with delimiter':'
            if(!(strcmp(token,buf))) {
    			token = strtok(NULL, delimiter);
    			token = strtok(NULL, delimiter); // According to file, the 3rd token is the paid or unpaid field
    			paid = atoi(token);
    		}
    	}
 		fclose(file);

 		/* construct the result packet */
        result = malloc(sizeof(struct access_pkt));
        result->start_pkt_ID = START_PACKET_ID;
        result->client_ID = access->client_ID;
        result->segment_No = access->segment_No;
        result->length = access->length;
        result->payload.technology = access->payload.technology;
        result->payload.subscriber_num = access->payload.subscriber_num;
        result->end_pkt_ID = END_PACKET_ID;

        if (paid == -1) { 
        	result->access_flag = SUBSCRIBER_NOT_EXIST;
        } else if (paid == 0){
        	result->access_flag = SUBSCRIBER_NOT_PAID;
        } else if (paid == 1) {
        	result->access_flag = SUBSCRIBER_OK;
        }

        send_buf = (char *) result;
		if(sendto(sock, send_buf, MAX_BUFFER_LENGTH, 0, (struct sockaddr *) &clnt_addr, sizeof(clnt_addr)) != MAX_BUFFER_LENGTH)
			DieWithError("sendto() sent a different number of bytes than expected");

		fprintf(stderr,"\n Sent packet to client for subscriber number :%s\n", buf);

		if (result != NULL) {
			free(result);
			result = NULL;
		}
	}

	if(send_buf != NULL) {
		free(send_buf);
		send_buf = NULL;
	}

	if (recv_buf != NULL) {
		free(recv_buf);
		recv_buf = NULL;
	}

}
