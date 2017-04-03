#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h>	/* for socket(), sendto() and recvfrom() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for memset() */
#include <unistd.h>		/*for close()*/
#include <sys/time.h>
#include <stdbool.h>
#include "assignheader.h"	/*custom header file*/


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
	uint32_t sub_num;

	char *recv_buf = NULL;
    recv_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);

	char *send_buf = NULL;
    send_buf = (char *)malloc (sizeof(char) * MAX_BUFFER_LENGTH);
	
	if ( (argc < 2 ) || (argc > 4) ) {	/*Test for correct number of arguments */
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
		printf("Socket creation failed");
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
    struct access_pkt *access = NULL;

    while(not_exit) {
    	choice = 0;
    	sleep(2); // Giving a chance for the user to see the output
    	fprintf(stderr,"\n ====================== MENU ====================== \n");
    	fprintf(stderr,"    1. Send right subscriber packet\n");
    	fprintf(stderr,"    2. Send subscriber not paid packet\n");
    	fprintf(stderr,"    3. Send subscriber not exist packet\n");
    	fprintf(stderr,"    4. Check access for a subscriber\n");
    	fprintf(stderr,"    5. EXIT\n\n");
    	fprintf(stderr,"    Please enter your choice:");
    	scanf("%d", &choice);

    	if (choice < 1 || choice > 5) {
    		fprintf(stderr,"\n************* Please enter a choice between 1-4 **************\n");
    		continue;
    	}

    	if (choice == 5) {
    		fprintf(stderr,"\n\nExiting......");
    		not_exit = false;
    		continue;
    	}

    	if (choice == 4) {
    		fprintf(stderr," \n Please enter the subscriber number to check access :");
    		scanf("%u", &sub_num);
    	}
		
		int count = 0;
		memset(send_buf, 0, MAX_BUFFER_LENGTH);
		/*****   BUILDING PACKETS BASED ON TEST CASES  ******/
		access = malloc(sizeof(struct access_pkt));
		access->start_pkt_ID = START_PACKET_ID;
		access->client_ID = 1;
		access->access_flag = SUBSCRIBER_ACCESS_PERMISSION;
		access->segment_No = 1;
		access->length = sizeof(struct pay_load);
		access->payload.technology = 2;

		if (choice == 1) {
			access->payload.subscriber_num = 4085546805; // Sending subscriber number with status paid.
		} else if (choice == 2) {
			access->payload.subscriber_num = 4086668821;  // Sending subscriber number with status NOT-paid.
		} else if (choice == 3) {
			access->payload.subscriber_num = 4085555555; // Sending subscriber number that doesnot exist
		} else if (choice == 4) {
			access->payload.subscriber_num = sub_num; // Sending subscriber number from user input
		}

		access->end_pkt_ID = END_PACKET_ID;
		send_buf = (char *) access;

	    memset(recv_buf, 0, MAX_BUFFER_LENGTH);
		num_retries = 0;
		while (num_retries < 3) {
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

		if (num_retries >= 3) {
		    fprintf(stderr, "\nError(Timed out) :- Server does not respond\n");
			        continue;
		}

		sleep(1); // Giving a chance for the user to see the output
		if (srv_addr.sin_addr.s_addr != clnt_addr.sin_addr.s_addr) {
			fprintf(stderr, "Error: recevied a packet from unknown source.\n");
			exit(1);
		}


	    struct access_pkt *result = (struct access_pkt *) recv_buf;
	    // Print the result based on the response from the server
	    switch (result->access_flag) {
	    	case SUBSCRIBER_NOT_PAID:
	    		fprintf(stderr, "\n Error: Subscriber has not paid the bill.\n");
	    		break;
	    	case SUBSCRIBER_NOT_EXIST:
	    		fprintf(stderr, "\n Error: Subscriber does not exist\n");
	    		break;
	    	case SUBSCRIBER_OK:
	    		fprintf(stderr, "\n Success: Subscriber is allowed access\n");
	    		break;
	    }

	    if(access != NULL) {
	    	free(access);
	    	access = NULL;
	    }

 	}
	close(sock);
	exit(0);

}