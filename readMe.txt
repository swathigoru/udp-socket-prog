Author: Swathi Goru
Date: 10 Mar 2017

Instructions to run program 1:
Make sure assignheader.h is in the same folder as assign1client.c and assign1server.c

Instructions to run program 2:
Make sure assignheader.h and verification_database.txt are in the same folder as assign2client.c and assign2server.c

Command to compile the files:
Go to respective folders and enter the below commands.
	
	gcc -o assign1server assign1server.c
	gcc -o assign1client assign1client.c
	

To run the program, give the command as:
	
	./assign1server 5000 (for server program)
	./assign1client 127.0.0.1 5000 (for client program)
