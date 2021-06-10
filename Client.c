//You should use the following definition for a client - to - server message :
typedef struct {

	enum { login, who, lookup, logout } requestType;     /* same size as an unsigned int */

	unsigned int playerDD;                       /* initiating player identifier */

	unsigned short tcpPort;                     /* listening port*/

} client2ServerMessage;                           /* an unsigned int is 32 bits = 4 bytes */

//You should use the following definition for a server - to - client message :
typedef struct {

	enum { ok, lookup, who } responseType;        /* same size as an unsigned int */

	int availPlayers[10];                         /* list of player identifiers */

	unsigned int requestedID;                 /* requested client id */

	unsigned short tcpPort;                     /* requested port */

	unsigned int ipAddress;                     /* requested IP address */

} serverMessage;

//You should use the following definition for a client - to - client message :
typedef struct {

	enum { play, accept, decline, move } requestType;    /* same size as an unsigned int */

	unsigned int playerID;                 /* initiating player identifier */

	unsigned int move;                     /* destination column – 1, 2, 3, 4, 5, 6, or 7 */

} client2ClientMessage;                  /* an unsigned int is 32 bits = 4 bytes */

int mian{





	return 0;
}