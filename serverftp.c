/*
server FTP program

NOTE: Starting homework #2, add more comments here describing the overall
function performed by server ftp program. This includes the list of ftp
commands processed by server ftp.
*/

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h> /* TODONEJacob: avoids implicit dec error for printf */
#include <stdlib.h>  /* TODONEJacob: avoids implicit dec error for exit */

#define SERVER_FTP_PORT 6798

/* Error and OK codes */
#define OK 0
#define ER_INVALID_HOST_NAME -1
#define ER_CREATE_SOCKET_FAILED -2
#define ER_BIND_FAILED -3
#define ER_CONNECT_FAILED -4
#define ER_SEND_FAILED -5
#define ER_RECEIVE_FAILED -6
#define ER_ACCEPT_FAILED -7

/* Function prototypes */
int svcInitServer(int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */
char userCmd[1024];	/* user typed ftp command line received from client */
char cmd[1024];		/* ftp command (without argument) extracted from userCmd */
char argument[1024];	/* argument (without ftp command) extracted from userCmd */
char replyMsg[1024];       /* buffer to send reply message to client */
char *tok;


/*
main

Function to listen for connection request from client
Receive ftp command one at a time from client
Process received command
Send a reply message to the client after processing the command with staus of
performing (completing) the command
On receiving QUIT ftp command, send reply to client and then close all sockets

Parameters
argc		- Count of number of arguments passed to main (input)
argv  	- Array of pointer to input parameters to main (input)
	Not required to pass any parameter to main
	Can use it if needed.

Return status
0			- Successful execution until QUIT command from client
ER_ACCEPT_FAILED	- Accepting client connection request failed
N			- Failed stauts, value of N depends on the command processed
*/
int main( int argc, char *argv[] )
{
	int msgSize;      /* Size of msg received in octets (bytes) */
	int listenSocket; /* listening server ftp socket for client connect request */
	int ccSocket;     /* Control connection socket - to be used in all client communication */
	int status;
	/* Hw2 Addtions */
	char *users[4]={"root","Jacob","Stiv","Kevin"};
	char *pwrds[4]={"root","passbutt","somebody1","8389836296"};
	int user_index = -1; /* to be assigned an int index  */
	char entered_pwrd[1024]; /* string entered with the pass command */

	/*
	NOTE: without \n at the end of format string in printf,	UNIX will buffer (not
	flush) output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n");
	/* initialize server ftp */
	printf("Initialize ftp server\n");
	status=svcInitServer(&listenSocket);
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n");
		exit(status);
	}

	printf("ftp server is waiting to accept connection\n");
	/* wait until connection request comes from client ftp */
	ccSocket = accept(listenSocket, NULL, NULL);
	printf("Came out of accept() function \n");
	if(ccSocket < 0)
	{
		perror("cannot accept connection:");
		printf("Server ftp is terminating after closing listen socket.\n");
		close(listenSocket);  /* close listen socket */
		return (ER_ACCEPT_FAILED);  // error exist
	}
	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n");


	/*
	Receive and process ftp commands from client until quit command.
	On receiving quit command, send reply to client and	then close the control
	connection socket "ccSocket".
	*/
	while(1)
	{
		status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize);
		if(status < 0)
    {
			printf("Receive message failed. Closing control connection \n");
			printf("Server ftp is terminating.\n");
			break;
    }

		/*
		Starting Homework#2 program to process all ftp commandsmust be added here.
		See Homework#2 for list of ftp commands to implement.
		Separate command and argument from userCmd.
		*/
		tok = strtok(userCmd, " ");
		strcpy(cmd, tok);
		while ( tok != NULL ) {
			strcpy(argument, tok);
			tok = strtok(NULL, " ");
		}

		if(strncmp(cmd, "user", 4) == 0) 											/* command: user */
		{
			// int users_size = sizeof users / sizeof users[0];
			int users_size = 4;
			user_index = -1;
			strcpy(replyMsg,"Username received.");
			/* when the array of users has been iterated through, end the loop.*/
			while(user_index < users_size)
			{
				/* when the index is correct, exit the loop*/
				if(strcmp(users[user_index], argument)==0) break;
				user_index++;
			}
			/* avoid out-of-bounds segfaut */
			if (user_index >= users_size) user_index = -1;
		}
		else if (strncmp(cmd, "pass", 4)==0) 								/* command: pass */
		{
			strcpy(replyMsg,"Password received.");
			strcpy(entered_pwrd, argument);
		}
		else if(strncmp(cmd, "quit", 4)==0)									/* command: quit */
		{
			printf("Closing listen 	socket.\n");
			close(listenSocket);  /*close listen socket */
			printf("Closing control connection socket.\n");
			close (ccSocket);  /* Close client control connection socket */
			printf("Existing from serv	er ftp main \n");
			return (status);
		}
		else																						/* all other commands */
		{
			if(strcmp(entered_pwrd, pwrds[user_index])!=0) // if login is invalid
			{
				strcpy(replyMsg,"Username and password do not match.");
		    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);
				continue;
			}
			/* only reachable with valid login */
			int switch_c =	/* converts string input to switch-caseable int */
				strcmp(cmd, "mkdir")==0 ? 1  :	// (or mkd)
				strcmp(cmd, "rmdir")==0 ? 2  :	// (or rmd)
				strcmp(cmd, "cd")==0 		? 3  :	// (or cwd)
				strcmp(cmd, "dele")==0 	? 4  :
				strcmp(cmd, "pwd")==0 	? 5  :
				strcmp(cmd, "ls")==0 		? 6  :
				strcmp(cmd, "stat")==0 	? 7  :	// (or status)
				strcmp(cmd, "help")==0 	? 8  :
				strcmp(cmd, "send")==0 	? 9  :
				strcmp(cmd, "recv")==0 	? 10 :
				0;															// unreconized command
			printf("switch_c: %i\n", switch_c);
			switch(switch_c)
			{
				FILE *file;
				char buffer[1024];       					 /* buffer to read into replyMsg */
				case 0:
					strcpy(replyMsg,"Unreconized command.");
					break;
				case 1:	//mkdir
					strcpy(replyMsg,"mkdir command recieved.");
					system(cmd);
					break;
				case 2: //rmdir
					strcpy(replyMsg,"rmdir command recieved.");
					break;
				case 3: //cd
					strcpy(replyMsg,"cd command recieved.");
					break;
				case 4: //dele
					strcpy(replyMsg,"dele command recieved.");
					break;
				case 5: //pwd
					system("pwd > commandOutput");
					file = fopen("commandOutput","r");
					strcpy(replyMsg,"");							 /* clear replyMsg */
					while (fgets(buffer, 1024, file))  /* read file into replyMsg */
						strcat(replyMsg, buffer);
					fclose(file);
					break;
				case 6: //ls
					system("ls > commandOutput");
					file = fopen("commandOutput","r");
					strcpy(replyMsg,"");							 /* clear replyMsg */
					while (fgets(buffer, 1024, file))  /* read file into replyMsg */
						strcat(replyMsg, buffer);
					fclose(file);
					break;
				case 7: //stat(or status)
					strcpy(replyMsg,"stat command recieved.");
					break;
				case 8: //help
					strcpy(replyMsg,"Commands and how to use\n"										
					"user  \t  log in as user\n"
                    "pass  \t log in password\n"
                    "mkdir \t make directories\n"
                    "rmdir \t remove directories\n"
                    "cd   \t change directory\n"
                    "dele \t remove a file\n"
                    "pwd  \t print directory\n"
                    "ls   \t print files\n"
                    "stat \t print stats\n"
					"help \t print help\n"
					"send \t put file from client to server\n"
					"recv \t get file from server to client\n"
					);				
					break;
				case 9: //send
					strcpy(replyMsg,"send command recieved.");
					break;
				case 10: //recv
					strcpy(replyMsg,"recv command recieved.");
					break;
			}
			/* Now has an appropriate reply msg in HW2 */
		}
		/*
		ftp server sends only one reply message to the client for	each command
		received in this implementation.
		*/
    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);	/* Added 1 to
		include NULL character in the reply string, as strlen does not count NULL
		character. */
    if(status < 0)
    {
			break;  /* exit while loop */
    }
	}
	// while(strncmp(cmd, "quit", 4) != 0);
	// printf("Closing control connection socket.\n");
	// close (ccSocket);  /* Close client control connection socket */
	// // shutdown (ccSocket);
	// printf("Closing listen socket.\n");
	// close(listenSocket);  /*close listen socket */
	// // shutdown(listenSocket);
	// printf("Existing from server ftp main \n");
	// return (status);
} /* end main() */


/*
 * svcInitServer
 *
 * Function to create a socket and to listen for connection request from client
 *    using the created listen socket.
 *
 * Parameters
 * s		- Socket to listen for connection request (output)
 *
 * Return status
 *	OK			- Successfully created listen socket and listening
 *	ER_CREATE_SOCKET_FAILED	- socket creation failed
 */

int svcInitServer (
	int *s 		/*Listen socket number returned from this function */
	)
{
	int sock;
	struct sockaddr_in svcAddr;
	int qlen;

	/*create a socket endpoint */
	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); /* added by jacob to hopefully solve "address already in use" error. update: doesnt work. */

	/*initialize memory of svcAddr structure to zero. */
	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* initialize svcAddr to have server IP address and server listen port#. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);  /* server IP address */
	svcAddr.sin_port=htons(SERVER_FTP_PORT);    /* server listen port # */

	/* bind (associate) the listen socket number with server IP and port#.
	 * bind is a socket interface function
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}

	/*
	 * Set listen queue length to 1 outstanding connection request.
	 * This allows 1 outstanding connect request from client to wait
	 * while processing current connection request, which takes time.
	 * It prevents connection request to fail and client to think server is down
	 * when in fact server is running and busy processing connection request.
	 */
	qlen=1;


	/*
	 * Listen for connection request to come from client ftp.
	 * This is a non-blocking socket interface function call,
	 * meaning, server ftp execution does not block by the 'listen' funcgtion call.
	 * Call returns right away so that server can do whatever it wants.
	 * The TCP transport layer will continuously listen for request and
	 * accept it on behalf of server ftp when the connection requests comes.
	 */

	listen(sock,qlen);  /* socket interface function call */

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /*successful return */
}


/*
 * sendMessage
 *
 * Function to send specified number of octet (bytes) to client ftp
 *
 * Parameters
 * s		- Socket to be used to send msg to client (input)
 * msg  	- Pointer to character arrary containing msg to be sent (input)
 * msgSize	- Number of bytes, including NULL, in the msg to be sent to client (input)
 *
 * Return status
 *	OK		- Msg successfully sent
 *	ER_SEND_FAILED	- Sending msg failed
 */

int sendMessage(
	int    s,	/* socket to be used to send msg to client */
	char   *msg, 	/* buffer having the message data */
	int    msgSize 	/* size of the message/data in bytes */
	)
{
	int i;


	/* Print the message to be sent byte by byte as character */
	for(i=0; i<msgSize; i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s, msg, msgSize, 0)) < 0) /* socket interface call to transmit */
	{
		perror("unable to send ");
		return(ER_SEND_FAILED);
	}

	return(OK); /* successful send */
}


/*
 * receiveMessage
 *
 * Function to receive message from client ftp
 *
 * Parameters
 * s		- Socket to be used to receive msg from client (input)
 * buffer  	- Pointer to character arrary to store received msg (input/output)
 * bufferSize	- Maximum size of the array, "buffer" in octent/byte (input)
 *		    This is the maximum number of bytes that will be stored in buffer
 * msgSize	- Actual # of bytes received and stored in buffer in octet/byes (output)
 *
 * Return status
 *	OK			- Msg successfully received
 *	R_RECEIVE_FAILED	- Receiving msg failed
 */


int receiveMessage (
	int s, 		/* socket */
	char *buffer, 	/* buffer to store received msg */
	int bufferSize, /* how large the buffer is in octet */
	int *msgSize 	/* size of the received msg in octet */
	)
{
	int i;

	*msgSize=recv(s,buffer,bufferSize,0); /* socket interface call to receive msg */

	if(*msgSize<0)
	{
		perror("unable to receive");
		return(ER_RECEIVE_FAILED);
	}

	/* Print the received msg byte by byte */
	for(i=0;i<*msgSize;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n");

	return (OK);
}
