/*
	Author: Blake Johnson
	COSC 439
*/

#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdio.h>     /* for printf() and fprintf() */
#include <stdlib.h>    /* for atoi() and exit() */
#include <string.h>    /* for memset() */
#include <string.h>
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <unistd.h>     /* for close() */
#include <sys/mman.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */

//You should use the following definition for a client - to - server message :
typedef struct {

	enum { login, who, lookup, logout } requestType;     /* same size as an unsigned int */

	unsigned int playerDD;                       /* initiating player identifier */

	unsigned short tcpPort;                     /* listening port*/

} client2ServerMessage;                           /* an unsigned int is 32 bits = 4 bytes */

//You should use the following definition for a server - to - client message :
typedef struct {

	enum { ok, lookupSever, whoSever } responseType;          /* same size as an unsigned int */

	int availPlayers[50];                         /* list of player identifiers */

	unsigned int requestedID;                 /* requested client id */

	unsigned short tcpPort;                     /* requested port */

	unsigned long ipAddress;                     /* requested IP address */

	int size;

} serverMessage;

//You should use the following definition for a client - to - client message :
typedef struct {

	enum { play, acceptGame, decline, move} TCPrequestType;    /* same size as an unsigned int */

	unsigned int playerID;                 /* initiating player identifier */

	char move [5][7];                     /* destination column – 1, 2, 3, 4, 5, 6, or 7 */

} client2ClientMessage;                  /* an unsigned int is 32 bits = 4 bytes */

void DieWithError(char* errorMessage); /* External error handling function */

int main(int argc, char* argv[]) {

	client2ServerMessage* clientMessage;
	serverMessage* servermessage;
	client2ClientMessage* clientClientMessage;
	unsigned int myID;

	clientMessage = (client2ServerMessage*)mmap(NULL, sizeof(clientMessage), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	servermessage = (serverMessage*)mmap(NULL, sizeof(servermessage), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
	clientClientMessage = (client2ClientMessage*)mmap(NULL, sizeof(clientClientMessage), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);


	memset(clientMessage, 0, sizeof(clientMessage)); /* Zero out structure */
	memset(servermessage, 0, sizeof(servermessage)); /* Zero out structure */
	memset(clientClientMessage, 0, sizeof(clientClientMessage)); /* Zero out structure */
	int i, j, k, z, playerMove, win = 0;
	char playAgain[10];

	printf("  1\t  2\t  3\t  4\t  5\t  6\t  7\n");
	printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 7; j++) {
			clientClientMessage->move[i][j] = ' ';
			printf("| %c | \t", clientClientMessage->move[i][j]);

		}
		printf("\n");
		printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
	}


	printf("----------------------------------------------------------------\n");
	printf("\t\tWELCOME TO CONNECT4 WITH FRIENDS\n");
	printf("----------------------------------------------------------------\n");


	printf("Please enter a 4 digit playerID: ");
	scanf("%d", &clientMessage->playerDD);
	printf("Please enter a 4 digit TCPPort number: ");
	myID = clientMessage->playerDD;
	scanf("%d", &clientMessage->tcpPort);
	printf("%d\n", clientMessage->tcpPort);

	clientMessage->requestType = login;


	if (fork() > 0) {

		/***********************************************************************
		* **********************************************************************
									Variables
		*************************************************************************
		*************************************************************************/

		int UDPsock;                        /* Socket descriptor */
		struct sockaddr_in UDPServAddr; /* Echo server address */
		struct sockaddr_in UDPfromAddr;     /* Source address of echo */
		unsigned short UDPServPort;     /* Echo server port */
		char* UDPservIP;                    /* IP address of server */
		unsigned int UDPfromSize;           /* In-out of address size for recvfrom() */
		char type[10];


		int TCPsock;                        /* Socket descriptor */
		struct sockaddr_in TCPServAddr; /* Echo server address */
		unsigned short TCPServPort;     /* Echo server port */
		char* TCPservIP;                    /* Server IP address (dotted quad) */
		char* echoString;                /* String to send to echo server */
		unsigned int TCPfromSize;      /* Length of string to echo */



		/***********************************************************************
									UDP- LOGIN
		*************************************************************************/

		if ((argc < 2) || (argc > 3))    /* Test for correct number of arguments */
		{
			fprintf(stderr, "Usage: %s <Server IP>  [<Echo Port>]\n", argv[0]);
			exit(1);
		}
		UDPservIP = argv[1];           /* First arg: server IP address (dotted quad) */
		UDPServPort = atoi(argv[2]);  /* Use given port, if any */


		/* Create a datagram/UDP socket */
		if ((UDPsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
			DieWithError("socket() failed Client");

		/* Construct the server address structure */
		memset(&UDPServAddr, 0, sizeof(UDPServAddr));    /* Zero out structure */
		UDPServAddr.sin_family = AF_INET;                 /* Internet addr family */
		UDPServAddr.sin_addr.s_addr = inet_addr(UDPservIP);  /* Server IP address */
		UDPServAddr.sin_port = htons(UDPServPort);     /* Server port */

		/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
		if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
			DieWithError("sendto() sent a different number of bytes than expected Client");

		/* Recv a response */
		UDPfromSize = sizeof(UDPfromAddr);

		if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
			DieWithError("recvfrom() failed Client");

		printf("We are getting you logged in!\n");


		/***********************************************************************
								Other UDP Funtions
		*************************************************************************/
		while (1) {
			printf("What can I do for you [who, play, acceptGame, decline, move, logout]: ");
			scanf("%s", type);
			i, j = 0;
			k = 5;

			/***********************************************************************
							Printing a list of Avaiable players
			*************************************************************************/

			if (strcmp(type, "Who") == 0 || strcmp(type, "who") == 0) {
				clientMessage->requestType = who;

				/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
				if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
					DieWithError("sendto() sent a different number of bytes than expected Client");


				if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
					DieWithError("recvfrom() failed Client");

				printf("----------------------------------------------------------------\n");
				printf("\t\tAvailable player\n");
				printf("----------------------------------------------------------------\n");
				for (i = 0; i <= servermessage->size; i++) {
					printf("Player: %d\n", servermessage->availPlayers[i]);
				}
			}

			/***********************************************************************
							lookimg for Information in a player to play with
			*************************************************************************/
			else if (strcmp(type, "play") == 0 || strcmp(type, "Play") == 0) {
				z = 1;
				clientMessage->requestType = lookup;
				clientClientMessage->TCPrequestType = play;
				printf("Who would you like to connect with: ");
				scanf("%d", &clientMessage->playerDD);

				/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
				if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
					DieWithError("sendto() sent a different number of bytes than expected Client");


				if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
					DieWithError("recvfrom() failed Client");

				printf("----------------------------------------------------------------\n");
				printf("\t\tPlayer %d Informaion\n", clientMessage->playerDD);
				printf("----------------------------------------------------------------\n");
				printf("RequestID: %d\tTCPPort: %d\tIPAddress: %d\n", servermessage->requestedID, servermessage->tcpPort, servermessage->ipAddress);


				TCPServPort = servermessage->tcpPort;

				/* Create a reliable, stream socket using TCP */
				if ((TCPsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError("socket() failed");

				/* Construct the server address structure */
				memset(&TCPServAddr, 0, sizeof(TCPServAddr));     /* Zero out structure */
				TCPServAddr.sin_family = AF_INET;             /* Internet address family */
				TCPServAddr.sin_addr.s_addr = servermessage->ipAddress;   /* Server IP address */
				TCPServAddr.sin_port = htons(TCPServPort); /* Server port */

				/* Establish the connection to the echo server */
				if (connect(TCPsock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
					DieWithError("connect() failed");

				clientClientMessage->playerID = myID;

				/* Send the string to the server */
				if (sendto(TCPsock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) != sizeof(*clientClientMessage))
					DieWithError("send() sent a different number of bytes than expected");

			}

			/***********************************************************************
										Logging out
			*************************************************************************/
			else if (strcmp(type, "Logout") == 0 || strcmp(type, "logout") == 0) {
				clientMessage->requestType = logout;


				/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
				if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
					DieWithError("sendto() sent a different number of bytes than expected Client");

				printf("----------------------------------------------------------------\n");
				printf("\t\tThanks for playing!\n");
				printf("----------------------------------------------------------------\n");

				exit(0);
			} 
			else if (strcmp(type, "acceptGame") == 0 || strcmp(type, "AcceptGame") == 0 || strcmp(type, "acceptgame") == 0) {

				clientMessage->requestType = lookup;
				clientMessage->playerDD = clientClientMessage->playerID;
				z = 2;

				/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
				if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
					DieWithError("sendto() sent a different number of bytes than expected Client");


				if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
					DieWithError("recvfrom() failed Client");

				TCPServPort = servermessage->tcpPort;

				/* Create a reliable, stream socket using TCP */
				if ((TCPsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError("socket() failed");

				/* Construct the server address structure */
				memset(&TCPServAddr, 0, sizeof(TCPServAddr));     /* Zero out structure */
				TCPServAddr.sin_family = AF_INET;             /* Internet address family */
				TCPServAddr.sin_addr.s_addr = servermessage->ipAddress;   /* Server IP address */
				TCPServAddr.sin_port = htons(TCPServPort); /* Server port */

				/* Establish the connection to the echo server */
				if (connect(TCPsock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
					DieWithError("connect() failed");

				//clientClientMessage->playerID = clientMessage->playerDD;
				clientClientMessage->TCPrequestType = acceptGame;
				clientClientMessage->playerID = myID;

				/* Send the string to the server */
				if (sendto(TCPsock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) != sizeof(*clientClientMessage))
					DieWithError("send() sent a different number of bytes than expected");

			}
			else if (strcmp(type, "decline") == 0 || strcmp(type, "Decline") == 0) {

				clientMessage->requestType = lookup;
				clientMessage->playerDD = clientClientMessage->playerID;

				/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
				if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
					DieWithError("sendto() sent a different number of bytes than expected Client");


				if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
					DieWithError("recvfrom() failed Client");

				TCPServPort = servermessage->tcpPort;

				/* Create a reliable, stream socket using TCP */
				if ((TCPsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
					DieWithError("socket() failed");

				/* Construct the server address structure */
				memset(&TCPServAddr, 0, sizeof(TCPServAddr));     /* Zero out structure */
				TCPServAddr.sin_family = AF_INET;             /* Internet address family */
				TCPServAddr.sin_addr.s_addr = servermessage->ipAddress;   /* Server IP address */
				TCPServAddr.sin_port = htons(TCPServPort); /* Server port */

				/* Establish the connection to the echo server */
				if (connect(TCPsock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
					DieWithError("connect() failed");

				//clientClientMessage->playerID = clientMessage->playerDD;
				clientClientMessage->TCPrequestType = decline;
				clientClientMessage->playerID = myID;

				/* Send the string to the server */
				if (sendto(TCPsock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) != sizeof(*clientClientMessage))
					DieWithError("send() sent a different number of bytes than expected");
			}
			else if (strcmp(type, "move") == 0 || strcmp(type, "Move") == 0) {
				if (clientClientMessage->TCPrequestType == decline)
					continue;


				printf("Enter the collomn you want to place you piece at: ");
				scanf("%d", &playerMove);
				for (i = 0; i < 5; i++) {
					if (clientClientMessage->move[i][playerMove - 1] == ' ') {
						k = i;
					}
				}
				if (z % 2 == 0)
					clientClientMessage->move[k][playerMove - 1] = 'X';
				else
					clientClientMessage->move[k][playerMove - 1] = 'O';
				printf("  1\t  2\t  3\t  4\t  5\t  6\t  7\n");
				printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
				for (i = 0; i < 5; i++) {
					for (j = 0; j < 7; j++) {
						printf("| %c | \t", clientClientMessage->move[i][j]);

					}
					printf("\n");
					printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
				}

				win = 0;
				// horizontalCheck 
				for (j = 0; j < 5; j++) {
					for (i = 0; i < 5; i++) {
						if ((clientClientMessage->move[i][j] == 'X' && clientClientMessage->move[i][j + 1] == 'X' && clientClientMessage->move[i][j + 2] == 'X' && clientClientMessage->move[i][j + 3] == 'X')
							|| (clientClientMessage->move[i][j] == 'O' && clientClientMessage->move[i][j + 1] == 'O' && clientClientMessage->move[i][j + 2] == 'O' && clientClientMessage->move[i][j + 3] == 'O')) {
							win = 1;
						}
					}
				}

				// verticalCheck
				for (i = 0; i < 2; i++) {
					for (j = 0; j < 7; j++) {
						if ((clientClientMessage->move[i][j] == 'X' && clientClientMessage->move[i + 1][j] == 'X' && clientClientMessage->move[i + 2][j] == 'X' && clientClientMessage->move[i + 3][j] == 'X')
							|| (clientClientMessage->move[i][j] == 'O' && clientClientMessage->move[i + 1][j] == 'O' && clientClientMessage->move[i + 2][j] == 'O' && clientClientMessage->move[i + 3][j] == 'O')) {
							win = 1;
						}
					}
				}

				// ascendingDiagonalCheck 
				for (i = 4; i > 2; i--) {
					for (j = 0; j < 5; j++) {
						if ((clientClientMessage->move[i][j] == 'X' && clientClientMessage->move[i - 1][j + 1] == 'X' && clientClientMessage->move[i - 2][j + 2] == 'X' && clientClientMessage->move[i - 3][j + 3] == 'X')
							|| (clientClientMessage->move[i][j] == 'O' && clientClientMessage->move[i - 1][j + 1] == 'O' && clientClientMessage->move[i - 2][j + 2] == 'O' && clientClientMessage->move[i - 3][j + 3] == 'O'))
							win = 1;
					}
				}
				// descendingDiagonalCheck
				for (i = 0; i < 2; i++) {
					for (j = 0; j < 5; j++) {
						if ((clientClientMessage->move[i][j] == 'X' && clientClientMessage->move[i + 1][j + 1] == 'X' && clientClientMessage->move[i + 2][j + 2] == 'X' && clientClientMessage->move[i + 3][j + 3] == 'X')
							|| (clientClientMessage->move[i][j] == 'O' && clientClientMessage->move[i + 1][j + 1] == 'O' && clientClientMessage->move[i + 2][j + 2] == 'O' && clientClientMessage->move[i + 3][j + 3] == 'O'))
							win = 1;
					}
				}

				for (i = 0; i < 5; i++) {
					for (j = 0; j < 7; j++) {
						if (clientClientMessage->move[i][j] == ' ') {
							break;
						}
						win = 1;
					}
				}


				if (win) {
					printf("\n----------------------------------------------------------------\n");
					printf("\t\tPlayer %d Won\n", myID);
					printf("----------------------------------------------------------------\n");


				}
				else {

					clientClientMessage->TCPrequestType = move;

					clientMessage->requestType = lookup;
					clientMessage->playerDD = clientClientMessage->playerID;

					/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
					if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
						DieWithError("sendto() sent a different number of bytes than expected Client");


					if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
						DieWithError("recvfrom() failed Client");

					TCPServPort = servermessage->tcpPort;

					/* Create a reliable, stream socket using TCP */
					if ((TCPsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
						DieWithError("socket() failed");

					/* Construct the server address structure */
					memset(&TCPServAddr, 0, sizeof(TCPServAddr));     /* Zero out structure */
					TCPServAddr.sin_family = AF_INET;             /* Internet address family */
					TCPServAddr.sin_addr.s_addr = servermessage->ipAddress;   /* Server IP address */
					TCPServAddr.sin_port = htons(TCPServPort); /* Server port */

					/* Establish the connection to the echo server */
					if (connect(TCPsock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
						DieWithError("connect() failed");

					//clientClientMessage->playerID = clientMessage->playerDD;
					clientClientMessage->playerID = myID;

					/* Send the string to the server */
					if (sendto(TCPsock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) != sizeof(*clientClientMessage))
						DieWithError("send() sent a different number of bytes than expected");
				}
			}
			else {
				printf("who                   -- List of available players\n");
				printf("play                  -- Request to play with someone\n");
				printf("acceptGame/decline    -- Choose to accept/decline a game \n");
				printf("move                  -- Place a piece on the game board\n");
			}

			if (win) {
				printf("Would you like to play again [yes/no]: ");
				scanf("%s", &playAgain);
				win = 0;

				if (strcmp(playAgain, "Yes") == 0 || strcmp(playAgain, "yes") == 0) {
					printf("  1\t  2\t  3\t  4\t  5\t  6\t  7\n");
					printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
					for (i = 0; i < 5; i++) {
						for (j = 0; j < 7; j++) {
							clientClientMessage->move[i][j] = ' ';
							printf("| %c | \t", clientClientMessage->move[i][j]);

						}
						printf("\n");
						printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
					}

					clientMessage->requestType = lookup;
					clientMessage->playerDD = clientClientMessage->playerID;

					/* Send the Struct to the server  that holds message for encrpying/decrypting request ID and Translation tpye*/
					if (sendto(UDPsock, (void*)clientMessage, sizeof(*clientMessage), 0, (struct sockaddr*)&UDPServAddr, sizeof(UDPServAddr)) != sizeof(*clientMessage))
						DieWithError("sendto() sent a different number of bytes than expected Client");


					if (recvfrom(UDPsock, (void*)servermessage, sizeof(*servermessage), 0, (struct sockaddr*)&UDPfromAddr, &UDPfromSize) != sizeof(*servermessage))
						DieWithError("recvfrom() failed Client");

					TCPServPort = servermessage->tcpPort;

					/* Create a reliable, stream socket using TCP */
					if ((TCPsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
						DieWithError("socket() failed");

					/* Construct the server address structure */
					memset(&TCPServAddr, 0, sizeof(TCPServAddr));     /* Zero out structure */
					TCPServAddr.sin_family = AF_INET;             /* Internet address family */
					TCPServAddr.sin_addr.s_addr = servermessage->ipAddress;   /* Server IP address */
					TCPServAddr.sin_port = htons(TCPServPort); /* Server port */

					/* Establish the connection to the echo server */
					if (connect(TCPsock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
						DieWithError("connect() failed");

					//clientClientMessage->playerID = clientMessage->playerDD;
					clientClientMessage->TCPrequestType = play;
					clientClientMessage->playerID = myID;

					/* Send the string to the server */
					if (sendto(TCPsock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) != sizeof(*clientClientMessage))
						DieWithError("send() sent a different number of bytes than expected");

				}
				else {

					for (i = 0; i < 5; i++) {
						for (j = 0; j < 7; j++) {
							clientClientMessage->move[i][j] = ' ';
						}
					}
				}
			}
		}

		close(UDPsock);
		return 0;
	}
	else {

		int TCPservSock;                    /* Socket descriptor for server */
		int TCPclntSock;                    /* Socket descriptor for client */
		struct sockaddr_in TCPServAddr; /* Local address */
		struct sockaddr_in TCPClntAddr; /* Client address */
		unsigned short TCPServPort;     /* Server port */
		unsigned int TCPclntLen;            /* Length of client address data structure */

		TCPServPort = clientMessage->tcpPort;

		/* Create socket for incoming connections */
		if ((TCPservSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
			DieWithError("socket() failed");

		/* Construct local address structure */
		memset(&TCPServAddr, 0, sizeof(TCPServAddr));   /* Zero out structure */
		TCPServAddr.sin_family = AF_INET;                /* Internet address family */
		TCPServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
		TCPServAddr.sin_port = htons(TCPServPort);      /* Local port */

		/* Bind to the local address */
		if (bind(TCPservSock, (struct sockaddr*)&TCPServAddr, sizeof(TCPServAddr)) < 0)
			DieWithError("bind() failed");

		/* Mark the socket so it will listen for incoming connections */
		if (listen(TCPservSock, MAXPENDING) < 0)
			DieWithError("listen() failed");

		for (;;) /* Run forever */
		{
			/* Set the size of the in-out parameter */
			TCPclntLen = sizeof(TCPClntAddr);

			/* Wait for a client to connect */
			if ((TCPclntSock = accept(TCPservSock, (struct sockaddr*)&TCPClntAddr, &TCPclntLen)) < 0)
				DieWithError("accept() failed");

			/* Receive message from client */
			if (recvfrom(TCPclntSock, (void*)clientClientMessage, sizeof(*clientClientMessage), 0, (struct sockaddr*)&TCPClntAddr, &TCPclntLen) != sizeof(*clientClientMessage))
				DieWithError("recv() failed");

			if (clientClientMessage->TCPrequestType == play) {
				printf("\n----------------------------------------------------------------\n");
				printf("\tPlayer %d would like to play with you!\n", clientClientMessage->playerID);
				printf("----------------------------------------------------------------\n");
			}			
			else if (clientClientMessage->TCPrequestType == move) {
				printf("\n----------------------------------------------------------------\n");
				printf("\t\tIts your Move!\n");
				printf("----------------------------------------------------------------\n");

				printf("  1\t  2\t  3\t  4\t  5\t  6\t  7\n");
				printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");
				for (i = 0; i < 5; i++) {
					for (j = 0; j < 7; j++) {
						printf("| %c | \t", clientClientMessage->move[i][j]);
					}
					printf("\n");
					printf("_____\t_____\t_____\t_____\t_____\t_____\t_____\n");;
				}
			} 
			else if (clientClientMessage->TCPrequestType == acceptGame) {
				printf("\n----------------------------------------------------------------\n");
				printf("\tCongradulations you are now playing with user %d!\n", clientClientMessage->playerID);
				printf("----------------------------------------------------------------\n");
				clientClientMessage->TCPrequestType = acceptGame;
			}
			else if (clientClientMessage->TCPrequestType == decline) {
				printf("\n----------------------------------------------------------------\n");
				printf("\tUnfortuanlly player %d did not want to play!\n", clientClientMessage->playerID);
				printf("----------------------------------------------------------------\n");
			}
		}
		close(TCPclntSock);
	}
}