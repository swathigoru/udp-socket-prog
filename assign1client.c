#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h>	/* for socket(), sendto() and recvfrom() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for memset() */
#include <unistd.h>		/*for close()*/
#include <sys/time.h>
#include <stdbool.h>
#include "assignheader.h"


int main (int argc, char * argv[])
{
	int sock; /*Socket descriptor */
	struct sockaddr_in srv_addr; /* server address */
	struct sockaddr_in clnt_addr; /* source address */
	unsigned short srv_port; /*server port*/
	unsigned int from_size; /*In-out of address size for recvfrom() */
	char *serv_IP; /* IP address of server */
	int resp_string_len; 	/* Length of received response */
	int testcase = 0;
	bool sendtwo = false;
	int num_retries;
	bool not_exit = true;
	int segment_number = 1;

	char *recv_buf = NULL;
    recv_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	char *send_buf = NULL;
    send_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);
	
	if ( (argc < 2 ) || (argc > 4) )	/*Test for correct number of arguments */
	{
		fprintf(stderr, "Usage: %s <Server IP> [<Port No>] ", argv[0]);
		exit(1);
	}

	serv_IP = argv[1];	/* first arg: server IP address */
	

	if (argc == 3)
		srv_port = atoi(argv[2]); /* Use given port, if any */
	else
		srv_port = 5000; 	/*giving a default port number as 5000 */

    /*Create UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		printf("Socket failed");
		//DieWithError("socket() failed");

	/*socket created*/

	/* construct the server address structure */
	memset(&srv_addr, 0, sizeof(srv_addr));	/*Zero out structure */
	srv_addr.sin_family = AF_INET;	/*Internet addr family */
	srv_addr.sin_addr.s_addr = inet_addr(serv_IP); 	/* Server IP address */
	srv_addr.sin_port = htons(srv_port);	/*Server port */

	struct timeval timeout={3,0}; //set timeout for 3 seconds

	/* set receive UDP message timeout */
	setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    int choice;
    char *str = "this is test data";
    struct data_pkt *my_data = NULL;

    while(not_exit) {
    	choice = 0;
    	sleep(2); // Giving a chance for the user to see the output
    	fprintf(stderr,"\n ====================== MENU ====================== \n");
    	fprintf(stderr,"    1. Send 5 packets and recieve ACK\n");
    	fprintf(stderr,"    2. Send packet to timeout\n");
    	fprintf(stderr,"    3. Send packet that is out of sequence\n");
    	fprintf(stderr,"    4. Send packet with different payload compared to the length \n");
    	fprintf(stderr,"    5. Send packet without End of packet identifier\n");
    	fprintf(stderr,"    6. Send a duplicate packet\n");
    	fprintf(stderr,"    7. EXIT\n\n");
    	fprintf(stderr,"    Please enter your choice:");
    	scanf("%d", &choice);

    	if (choice < 1 || choice > 7) {
    		fprintf(stderr,"\n************* Please enter a choice between 1-7 **************\n");
    		continue;
    	}

    	if (choice == 7) {
    		not_exit = false;
    		continue;
    	}
		
		if (segment_number == 255) {
			segment_number = 1;
		}

		
		int count = 0;
loop:
		memset(send_buf, 0, MAX_BUFFER_LENGTH);
		/*****   BUILDING PACKETS BASED ON TEST CASES  ******/
		if ((choice == 1) && (count < 5)) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->length = strlen(str);
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = END_PACKET_ID;
			my_data->segment_No = segment_number;
			send_buf = (char *) my_data;
		} else if (choice == 2) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->length = strlen(str);
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = END_PACKET_ID;
			my_data->segment_No = 0;	
			send_buf = (char *) my_data;
		} else if (choice == 3) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->length = strlen(str);
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = END_PACKET_ID;
			my_data->segment_No = segment_number + 2;
			send_buf = (char *) my_data;
		} else if (choice == 4) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->segment_No = segment_number;
			my_data->length = 1;
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = END_PACKET_ID;
			send_buf = (char *) my_data; 
		} else if (choice == 5) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->segment_No = segment_number;
			my_data->length = strlen(str);
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = 0x0001;
			send_buf = (char *) my_data; 
		} else if (choice == 6) {
			my_data = malloc(sizeof(struct data_pkt));
			my_data->start_pkt_ID = START_PACKET_ID;
			my_data->client_ID = 1;
			my_data->data_flag = DATA_PACKET;
			my_data->segment_No = segment_number - 1;
			my_data->length = strlen(str);
			strcpy(my_data->payload, str);
			my_data->end_pkt_ID = END_PACKET_ID;
			send_buf = (char *) my_data; 
		}

	    memset(recv_buf, 0, MAX_BUFFER_LENGTH);
		num_retries = 0;
		while (num_retries < 3) {
            fprintf(stderr,"\nSending packet with segment_number:%d to server", my_data->segment_No);
			if(sendto(sock, (char *)send_buf, MAX_BUFFER_LENGTH, 0, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) != MAX_BUFFER_LENGTH)
				DieWithError("sendto() failed");
			
			/* Receive a response */
			from_size = sizeof(clnt_addr);
		    int resp_string_len = recvfrom(sock, recv_buf, MAX_BUFFER_LENGTH,0, (struct sockaddr *)&clnt_addr, &from_size);
		    if (resp_string_len < 0) {
		    	fprintf(stderr, "\nNo ACK from server. Resending packet.\n");
		    	num_retries++;
		    } else if (resp_string_len != MAX_BUFFER_LENGTH) {
				DieWithError("recvfrom() failed");
		    } else {
		    	break;
		    }
		}

		if (num_retries >= 3) 
		{
		    fprintf(stderr, "\nError(Timed out) :- Server does not respond\n");
			        continue;
		}

		sleep(1); // Giving a chance for the user to see the output
		if (srv_addr.sin_addr.s_addr != clnt_addr.sin_addr.s_addr)
		{
			fprintf(stderr, "Error: recevied a packet from unknown source.\n");
			exit(1);
		}


	    struct result *result_structure = (struct result *) recv_buf;

	    if (!((result_structure->pkt_flag == ACK_PACKET) || (result_structure->pkt_flag == REJECT_PACKET)))
	    {
			fprintf(stderr, "Error: recevied unknown packet format.\n");
			exit(1);
	    }
	    
	    if (result_structure->pkt_flag == REJECT_PACKET) 
	    {
	    	struct reject_pkt *reject = (struct reject_pkt *) recv_buf;
	    	switch (reject->reject_sub_code) {
	    		case REJECT_OUT_OF_SEQ:
	    		    fprintf(stderr, "\nError(PKT REJECTED): Sent packet with segment_number:%d is out of sequence.\n", reject->segment_No);
	    		    break;
	    		case REJECT_LENGTH_MISMATCH:
	    		    fprintf(stderr, "\nError(PKT REJECTED): Sent packet's payload and length did not match for segment_number:%d.\n", reject->segment_No);
	    		    break;
	    		case REJECT_END_OF_PACKET_MISSING:
	    		    fprintf(stderr, "\nError(PKT REJECTED): Sent packet's end of packet missing for segment_number:%d.\n", reject->segment_No);
	    		    break;
	    		case REJECT_DUPLICATE_PACKING:
	    		    fprintf(stderr, "\nError(PKT REJECTED): Sent packet with segment_number:%d is a duplicate packet.\n", reject->segment_No);
	    		    break;
	    		default:
	    			fprintf(stderr, "\nError(PKT REJECTED): Unknown Error for Packet with segment_number:%d.\n", reject->segment_No);
	    	}
	    } else if(result_structure->pkt_flag == ACK_PACKET) {
	    	struct ack_pkt *ack = (struct ack_pkt *) recv_buf;
	    	fprintf(stderr,"\nRecieved ACK for segment_number:%d  from server\n", ack->segment_No);
	    	segment_number++;
	    }

	    if ((choice == 1) && (count < 4)) {
	    	count++;
	    	goto loop;
	    }
 	}
	close(sock);
	exit(0);

}