/*
	Author: Blake Johnson
	COSC 439
*/


#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <netinet/in.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <unistd.h>     /* for close() */

typedef struct {

	enum { login, who, lookup, logout } requestType;     /* same size as an unsigned int */

	unsigned int playerDD;                       /* initiating player identifier */

	unsigned short tcpPort;                     /* listening port*/

} client2ServerMessage;                           /* an unsigned int is 32 bits = 4 bytes */

//You should use the following definition for a server - to - client message :

typedef struct {

	enum { ok, lookupSever, whoSever} responseType;        /* same size as an unsigned int */

	int availPlayers[50];                         /* list of player identifiers */

	unsigned int requestedID;                 /* requested client id */

	unsigned short tcpPort;                     /* requested port */

	unsigned long ipAddress;                /* requested IP address */

	int size;

} serverMessage;

struct players {
	unsigned int playerDD;
	unsigned short tcpPort;
	unsigned long ipAddress;
};

void DieWithError(char* errorMessage); /* External error handling function */

int main(int argc, char* argv[])
{

	/***********************************************************************
	* **********************************************************************
							Variables
	*************************************************************************
	*************************************************************************/

	int sock;                        /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned short echoServPort;     /* Server port */
	unsigned int cliAddrLen;         /* Length of incoming message */

	client2ServerMessage clientMessage;
	serverMessage serverMessage;
	struct players players[50];
	char type[10];

	memset(&clientMessage, 0, sizeof(clientMessage)); /* Zero out structure */
	memset(&serverMessage, 0, sizeof(serverMessage)); /* Zero out structure */
	memset(&type, 0, sizeof(type));                   /* Zero out structure */


	/***********************************************************************
							Setting up server connection 
	*************************************************************************/

	if (argc != 2)         /* Test for correct number of parameters */
	{
		fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

	echoServPort = atoi(argv[1]);  /* First arg:  local port */

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed Server");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
	echoServAddr.sin_family = AF_INET;                /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort);      /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed Server");

	/***********************************************************************
								Waiting for something to do
	*************************************************************************/

	int i, j, k = 0;
	for (;;) /* Run forever */
	{

		cliAddrLen = sizeof(echoClntAddr);

		/* Block until receive message from a client */
		if (recvfrom(sock, (void*)&clientMessage, sizeof(clientMessage), 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen) < 0)
			DieWithError("recvfrom() failed Server");


		/***********************************************************************
									Logging in
		*************************************************************************/
		if (clientMessage.requestType == 0) {
			serverMessage.responseType = ok;
			serverMessage.size = i;
			serverMessage.requestedID = clientMessage.playerDD;
			serverMessage.tcpPort = clientMessage.tcpPort;
			serverMessage.availPlayers[i] = clientMessage.playerDD;
			serverMessage.ipAddress = echoClntAddr.sin_addr.s_addr;

			printf("Logging in player %d... \n", clientMessage.playerDD);
			players[i].playerDD = clientMessage.playerDD;
			players[i].tcpPort = clientMessage.tcpPort;
			players[i].ipAddress = echoClntAddr.sin_addr.s_addr;
			i = i+1;
			if (sendto(sock, (void*)&serverMessage, sizeof(serverMessage), 0, (struct sockaddr*)&echoClntAddr, sizeof(echoClntAddr)) != sizeof(serverMessage))
				DieWithError("sendto() sent a different number of bytes than expected Server");
		}


		/***********************************************************************
									Sending list of player
		*************************************************************************/
		else if (clientMessage.requestType == 1) {

			printf("Sending list of available player... \n");
			serverMessage.responseType = whoSever;
			if (sendto(sock, (void*)&serverMessage, sizeof(serverMessage), 0, (struct sockaddr*)&echoClntAddr, sizeof(echoClntAddr)) != sizeof(serverMessage))
				DieWithError("sendto() sent a different number of bytes than expected Server");

		}

		/***********************************************************************
									Looking up players informaion
		*************************************************************************/
		else if (clientMessage.requestType == 2) {
			serverMessage.responseType = lookupSever;
			printf("Sending information about player... \n");
			for (k = 0; k <i; k++) {
				if (players[k].playerDD == clientMessage.playerDD) {
					serverMessage.ipAddress = players[k].ipAddress;
					serverMessage.tcpPort = players[k].tcpPort;
					serverMessage.requestedID = players[k].playerDD;
					printf("RequestID: %d\tTCPPort: %d\tIPAddress: %d\n", serverMessage.requestedID, serverMessage.tcpPort, serverMessage.ipAddress);
					break;
				}
			}

			if (sendto(sock, (void*)&serverMessage, sizeof(serverMessage), 0, (struct sockaddr*)&echoClntAddr, sizeof(echoClntAddr)) != sizeof(serverMessage))
				DieWithError("sendto() sent a different number of bytes than expected Server");
		}    


		/***********************************************************************
									Logging out
		*************************************************************************/
		else if (clientMessage.requestType == 3) {
			serverMessage.responseType = ok;
			printf("Logging out player %d...\n", clientMessage.playerDD);
			for (k = 0; k <= i; k++) {
				if (players[k].playerDD == clientMessage.playerDD) {
					for (j = k; j < i; j++) {
						players[j] = players[j + 1];
						serverMessage.availPlayers[j] = serverMessage.availPlayers[j + 1];
					}
				}
			}
			if (i > 0) {
				i = i-1;
				serverMessage.size = i-1;
			}
		}
	}
	/* NOT REACHED */
}
