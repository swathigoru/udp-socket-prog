#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>
#include <sys/socket.h>	/* for socket(), sendto() and recvfrom() */
#include <arpa/inet.h>	/* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for memset() */
#include <unistd.h>		/*for close()*/


#define MAX_BUFFER_LENGTH               1024
#define MAX_PAYLOAD_LENGTH       		255

#define DATA_PACKET              		0xFFF1
#define ACK_PACKET               		0xFFF2
#define REJECT_PACKET            		0xFFF3
#define START_PACKET_ID          		0xFFFF
#define END_PACKET_ID            		0xFFFF

#define REJECT_OUT_OF_SEQ        		0xFFF4
#define REJECT_LENGTH_MISMATCH         	0xFFF5
#define REJECT_END_OF_PACKET_MISSING   	0xFFF6
#define REJECT_DUPLICATE_PACKING		0xFFF7

#define SUBSCRIBER_ACCESS_PERMISSION    0xFFF8
#define SUBSCRIBER_NOT_PAID				0xFFF9
#define SUBSCRIBER_NOT_EXIST			0xFFFA
#define SUBSCRIBER_OK					0xFFFB


struct data_pkt {
	uint16_t start_pkt_ID ;
	uint8_t client_ID;
	uint16_t data_flag;
	uint8_t segment_No;
	uint8_t length;
	char payload[MAX_PAYLOAD_LENGTH];
	uint16_t end_pkt_ID;
};

struct result {
	uint16_t start_pkt_ID;
	uint8_t client_ID;
	uint16_t pkt_flag;
};

struct ack_pkt {
	uint16_t start_pkt_ID;
	uint8_t client_ID;
	uint16_t ack_flag;
	uint8_t segment_No;
	uint16_t end_pkt_ID;
};

struct reject_pkt {
	uint16_t start_pkt_ID;
	uint8_t client_ID;
	uint16_t reject_flag;
	uint16_t reject_sub_code; 
	uint8_t segment_No;
	uint16_t end_pkt_ID;
};


struct pay_load {
	uint8_t technology;
	uint32_t subscriber_num;
};

struct access_pkt {
	uint16_t start_pkt_ID;
	uint8_t client_ID;
	uint16_t access_flag;
	uint8_t segment_No;
	uint8_t length;
	struct pay_load payload;
	uint16_t end_pkt_ID;
};

void DieWithError(char *errorMessage) /*External error handling function */
{
	perror(errorMessage);
	exit(1);
}