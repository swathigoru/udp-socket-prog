#include <stdio.h>
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
	int segment_array[256]; /*using segment number as an index*/
	struct reject_pkt *reject = NULL;
	struct ack_pkt *ack = NULL;

    /* Intitalize recieve and send buffers */ 
	char *recv_buf = NULL;
    recv_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	char *send_buf = NULL;
    send_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	srv_port = atoi(argv[1]);

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
		DieWithError("bind() failed;");

	memset(segment_array, 0, sizeof(segment_array));

	for (;;) /*infinite loop*/
	{
		clnt_addr_len = sizeof(clnt_addr);
		memset(recv_buf, 0, MAX_BUFFER_LENGTH);

		if((recv_msg_size = recvfrom(sock, recv_buf, MAX_BUFFER_LENGTH, 0, (struct sockaddr *) &clnt_addr, &clnt_addr_len)) < 0) 
			DieWithError("recvfrom() failed");

        struct data_pkt *data = (struct data_pkt *) recv_buf;
        bool is_rejected = false;
        int reject_code = 0;

        fprintf(stderr,"\n Recieved packet with segment_number:%d from client", data->segment_No);
        if (data->segment_No == 0) 
        {
        	continue;
        }


		if ((data->segment_No != 1) && (segment_array[data->segment_No - 1] == 0)) 
		{ // Out of order packets
			is_rejected = true;
			reject_code = REJECT_OUT_OF_SEQ;
		} else if (data->length != strlen(data->payload)) 
		{
			is_rejected = true;
			reject_code = REJECT_LENGTH_MISMATCH;
		} else if(data->end_pkt_ID != END_PACKET_ID) 
		{
			is_rejected = true;
			reject_code = REJECT_END_OF_PACKET_MISSING;
		} else if (segment_array[data->segment_No] != 0) {
			is_rejected = true;
			reject_code = REJECT_DUPLICATE_PACKING;
		}

		memset(send_buf, 0, MAX_BUFFER_LENGTH);
		if(is_rejected) {
			reject  = malloc(sizeof(struct reject_pkt));
			reject->start_pkt_ID = START_PACKET_ID;
			reject->client_ID = data->client_ID;
			reject->reject_flag = REJECT_PACKET;
			reject->reject_sub_code = reject_code;
			reject->segment_No = data->segment_No;
			reject->end_pkt_ID = END_PACKET_ID;
			send_buf = (char *) reject;
		} else {
			ack = malloc(sizeof(struct ack_pkt));
        	ack->start_pkt_ID = START_PACKET_ID;
			ack->client_ID = data->client_ID;
			ack->ack_flag = ACK_PACKET;
			ack->segment_No = data->segment_No;
			ack->end_pkt_ID = END_PACKET_ID;
			send_buf = (char *) ack;
			segment_array[data->segment_No] = 1;
		}
        
		fprintf(stderr,"\n Server sending ACK/REJECT packet to client for segment_number:%d \n", data->segment_No);
		if(sendto(sock, send_buf, MAX_BUFFER_LENGTH, 0, (struct sockaddr *) &clnt_addr, sizeof(clnt_addr)) != MAX_BUFFER_LENGTH)
			DieWithError("sendto() sent a different number of bytes than expected");

		if (data->segment_No == 255) {
			memset(segment_array, 0, sizeof(segment_array));
		}

		if(reject != NULL) {
			free(reject);
			reject = NULL;
		}

		if( ack != NULL) {
			free(ack);
			ack = NULL;
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