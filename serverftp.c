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

#define SERVER_FTP_PORT 2076
#define DATA_CONNECTION_PORT 2077

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
int clntConnect(char *serverName, int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */
char userCmd[1024];/* user typed ftp command line received from client */
char cmd[1024];		/* ftp command (without argument) extracted from userCmd */
char argument[1024];	/* argument (without ftp command) extracted from userCmd */
char replyMsg[1024];       /* buffer to send reply message to client */
char ftpData[100]; /*Buffer to send/receive message to client*/
int fileBytesRead = 0; //Used to count the toral number of bytes transferred during ftp
int ftpBytes = 0; //The number of bytes read by fread
int bytesReceived = 0 ; //The number of bytes received in a singel ftp message
char *tok;

// int isLoggedIN = -1; //-1 is default, 0 if the is not logged in, 1 otherwise

FILE *filePrt; //Used to point to the temporary file that logged commad output
int bytesRead = -1; //The number of bytes read by fread()
char fileData[1024]; //Used to store the byte data obtained by fread(). Max size = 1024 bytes per in-class dicussion

//login codes
// #define LOGGEN_IN 1
// #define LOGGEN_OUT 0
// #define NO_USER -1

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
	int dcSocket; 		//Data connection sockert to be used in all server communication
	/* Hw2 Addtions */
	char *users[4]={"root","Jacob","Stiv","Kevin"};
	char *pwrds[4]={"root","passbutt","somebody1","8389836296"};
	int user_index = -1; /* to be assigned an int index  */
	char entered_pwrd[1024]; /* string entered with the pass command */
	int cmdStatus;

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
		printf("userCmd: %s\n", userCmd);
		tok = strtok(userCmd, " ");
		strcpy(cmd, tok);
		// while ( tok != NULL ) {
		// 	strcpy(argument, tok);
		// 	tok = strtok(NULL, " ");
		// }
		tok = strtok(NULL, " ");
		if( tok!=NULL ) {
			strcpy(argument, tok);
			tok = strtok(NULL, " ");
		} else {
			strcpy(argument, "");
		}
		printf("cmd: %s\n", cmd);
		printf("argument: %s\n", argument);

		if(strncmp(cmd, "user", 4)==0) 											/* command: user */
		{
			// int users_size = sizeof users / sizeof users[0];
			int users_size = 4;
			user_index = -1;
			strcpy(replyMsg,"Username received.\n");
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
			strcpy(replyMsg,"Password received.\n");
			strcpy(entered_pwrd, argument);
		}
		else if(strncmp(cmd, "quit", 4)==0)									/* command: quit */
		{
			printf("Closing listen 	socket.\n");
			close(listenSocket);  /*close listen socket */
			printf("Closing control connection socket.\n");
			close (ccSocket);  /* Close client control connection socket */
			printf("Exiting from server ftp main.\n");
			return (status);
		}
		else																						/* all other commands */
		{
			if(0) // turn off password check for testing
			// if(strcmp(entered_pwrd, pwrds[user_index])!=0) // if login is invalid
			{
				strcpy(replyMsg,"Username and password do not match.\n");
		    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);
				continue;
			}
			/* only reachable with valid login */
			int switch_c =	/* converts string input to switch-caseable int */
				strcmp(cmd, "mkdir")==0 ? 1 :	// 1 for general arg-needed
				strcmp(cmd, "mkd")==0 	? 1 :
				strcmp(cmd, "rmdir")==0 ? 1 :
				strcmp(cmd, "rmd")==0 	?	1 :
				strcmp(cmd, "pwd")==0 	? 2 :	// 2 for general arg-optional
				strcmp(cmd, "ls")==0 		? 2 :
				strcmp(cmd, "cd")==0 		? 3 :
				strcmp(cmd, "cwd")==0 	? 3 :
				strcmp(cmd, "dele")==0 	? 4 :
				strcmp(cmd, "stat")==0 	? 5 :
				strcmp(cmd, "status")==0? 5 :
				strcmp(cmd, "help")==0 	? 6 :
				strcmp(cmd, "send")==0 	? 7 :
				strcmp(cmd, "recv")==0 	? 8 :
				0;															// unreconized command
			printf("switch_c: %i\n", switch_c);
			switch(switch_c)
			{
				FILE *file;
				char buffer[1024];       					 /* buffer to read into replyMsg */
				case 0:
					// strcpy(replyMsg,"Unreconized command.");
					strcpy(replyMsg, "502 command not implemented. Use \"help\" for a list of valid commands.\n");
					break;
				case 1: // mkdir and rmdir (general arg-needed)
					strcpy(replyMsg, "COMMAND FAILURE: Missing argument\n");
					if (strcmp(argument, "")==0)	break;	// if arg, proceed to case 2
				case 2: //ls, pwd
					strcpy(replyMsg, "COMMAND FAILURE\n");
					strcat(cmd, " ");									// space before argument
					strcat(cmd, argument);						// include argument (if present)
					strcat(cmd, " > commandOutput");	// add pipe(?) to output file
					// strcat(cmd, " > ~/ftpProject/server/commandOutput.txt");	// add pipe(?) to output file
					cmdStatus = system(cmd);											// call command to UNIX
					if (cmdStatus!=0) break;					// catch and report command failure
					// if (system(cmd)!=0) break;					// catch and report command failure
					file = fopen("commandOutput","r");	// open output
					// file = fopen("~/ftpProject/server/commandOutput.txt","r");	// open output
					strcpy(replyMsg,"");							 /* clear replyMsg */
					printf("%s\n","break1");
					while (fgets(buffer, 1024, file))  /* read file into replyMsg */
						strcat(replyMsg, buffer);
					printf("%s\n","break2");
					fclose(file);

					status=unlink("commandOutput");
					if(status!=0) strcpy(replyMsg, "Unable to remove commandOutput.\n");
					break;
				case 3: //cd
					if (strcmp(argument,"")==0)
						strcpy(argument,"..");	// parent directory
					status=chdir(argument);
					strcpy(replyMsg, "DIRECTORY CHANGE FAILURE\n");
					if(status!=0) break;

					status=system("pwd > commandOutput");
					strcpy(replyMsg, "DIRECTORY REPORT FAILURE\n");
					if(status!=0) break;

					file=fopen("commandOutput","r");
					strcpy(replyMsg,"Directory changed to ");		/* clear replyMsg */
					while (fgets(buffer, 1024, file))  /* read file into replyMsg */
						strcat(replyMsg, buffer);
					fclose(file);

					status=unlink("commandOutput");
					if(status!=0) strcpy(replyMsg, "Unable to remove commandOutput.\n");
					break;
				case 4: //dele
					strcpy(replyMsg, "Please retry with a another argument.\n");
					if (*argument=='\0') break; 		//check it there is an argument

					status=unlink(argument); // is this better than rm?
					strcpy(replyMsg, "Unable to remove the file.\n");
					if(status!=0) break;

					strcpy(replyMsg, "The file is removed.\n");
					break;
				case 5: //stat(or status)
					strcpy(replyMsg,"Command is Good. Transfer mode is ASCII.\n");
					break;
				case 6: //help
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
				case 7: //send
					//Attempt data connection despit any error to prevent client from endlessly listening
					printf("Calling clntConnect to connect to the client.\n");
					status = clntConnect("10.3.200.17", &dcSocket); //offcampus IP: 134.241.37.12

					//Data connection failed this will tell the client and it close the socket
					strcpy(replyMsg, "425 'send' coould not open data connection. closing data connection socket.\n");
					if(status!=0) break;	//updated by jacob to match project convention
						/* when the connection is established, this checks that the user
						provided a file. */
					printf("Data connection established to client.\n");

					//Check for invalid argument. Throw away any message received on dcSocket
					strcpy(replyMsg, "501 invalid syntax. Use: \"send <filename>\". Closing data connection.\n");
					if(argument[0]==NULL || strcmp(argument, "")==0) break;

					filePrt=NULL;
					filePrt=fopen(argument, "w");
					if(filePrt==NULL)
					{
						strcpy(replyMsg, "550 The file \"");
						strcat(replyMsg, argument);
						strcat(replyMsg, "\" could not be opened/created. Closing data connection.\n");
						fclose(filePrt); //close file when it has been not received.
						close(dcSocket); /*close the data connection if an error occur*/
						break;
					}

					//File can be written. Begin receiving file until no
					ftpBytes = 0; //initialize by count
					printf("Waiting for file transmission from client.\n");
					do
					{
						bytesReceived = 0;
						status = receiveMessage(dcSocket, ftpData, sizeof(ftpData), &bytesReceived);
						fwrite(ftpData, 1, bytesReceived, filePrt); //sucess or fail fwrite returns >= 0. this info is uselss
						ftpBytes = ftpBytes + bytesReceived;
					}
					while(bytesReceived > 0 && status == OK);

					strcpy(replyMsg, "");
					sprintf(replyMsg, "226 Received %d", ftpBytes);
					strcat(replyMsg, " bytes. closing data connection.\n");

					fclose(filePrt); // close file when it has been received.
					close(dcSocket); // close the data connection when no error occured.
					printf("Data connection closed.\n");
					break;
				case 8: //recv
					printf("Calling clntConnect to connect to the client.\n");
					status = clntConnect("10.3.200.17", &dcSocket); //offcampus IP: 134.241.37.12

					strcpy(replyMsg, "425 'revc' could not establish data connection. Closing connection.\n");
					if(status != 0) break;
					printf("Data Connection to client succesful.\n");
					strcpy(replyMsg, "501 invalid syntax. Use: \"Send <filename>\". Closing data connection.\n");
					if(argument[0] == NULL || strcmp(argument, "") == 0) break;

					filePrt = NULL;
					filePrt = fopen(argument, "r");
					if(filePrt == NULL)
					{
						strcpy(replyMsg, "550 the file \"");
						strcat(replyMsg, argument);
						strcat(replyMsg, "\" could not be opened. Closing data connection.\n");
						fclose(filePrt); //close file when it has been not received.
						close(dcSocket); /*close the data connection if an error occur*/
						break;
					}

					//File can be read. Begin sending file until no
					ftpBytes = 0;
					printf("Sending file.\n");
					do
					{
						fileBytesRead = 0;
						fileBytesRead = fread(ftpData, 1, 100, filePrt);
						status = sendMessage(dcSocket, ftpData, fileBytesRead);
						ftpBytes = ftpBytes + fileBytesRead;
					}
					while(!feof(filePrt) && status == OK);

					strcpy(replyMsg, "");
					sprintf(replyMsg, "226 sendt %d", ftpBytes);
					strcat(replyMsg, " bytes. Closing data connection.\n");

					fclose(filePrt);
					close(dcSocket);
					printf("Data connection closed.\n");
					break;
			}
			/* Now has an appropriate reply msg in HW2 */
		}
		/*
		ftp server sends only one reply message to the client for	each command
		received in this implementation.
		Added 1 to include NULL character in the reply string, as strlen does not
		count NULL character.
		*/
    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);
    if(status<0) break;  /* exit while loop */
	}
	while(strncmp(cmd, "quit", 4) != 0);

	// strcpy(replyMsg, "221 Service closing data and control connection.\n");

	printf("Closing control connection socket.\n");
	close (ccSocket);  /* Close client control connection socket */

	printf("Closing data connection socket.\n");
	close (dcSocket);  /* Close client control connection socket */

	printf("Closing listen socket.\n");
	close(listenSocket);  /*close listen socket */

	printf("Existing from server ftp main \n");
	return (status);
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
 * clntConnect
 * copied from clientftp.c
 *
 * Function to create a socket, bind local client IP address and port to the socket
 * and connect to the server
 *
 * Parameters
 * serverName	- IP address of server in dot notation (input)
 * s		- Control connection socket number (output)
 *
 * Return status
 *	OK			- Successfully connected to the server
 *	ER_INVALID_HOST_NAME	- Invalid server name
 *	ER_CREATE_SOCKET_FAILED	- Cannot create socket
 *	ER_BIND_FAILED		- bind failed
 *	ER_CONNECT_FAILED	- connect failed
 */


int clntConnect (
	char *serverName, /* server IP address in dot notation (input) */
	int *s 		  /* control connection socket number (output) */
	)
{
	int sock;	/* local variable to keep socket number */

	struct sockaddr_in clientAddress;  	/* local client IP address */
	struct sockaddr_in serverAddress;	/* server IP address */
	struct hostent	   *serverIPstructure;	/* host entry having server IP address in binary */


	/* Get IP address os server in binary from server name (IP in dot natation) */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* error return */
	}

	/* Create control connection socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* error return */
	}

	/* initialize client address structure memory to zero */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set local client IP address, and port in the address structure */
	clientAddress.sin_family = AF_INET;	/* Internet protocol family */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY is 0, which means */
						 /* let the system fill client IP address */
	clientAddress.sin_port = 0;  /* With port set to 0, system will allocate a free port */
			  /* from 1024 to (64K -1) */

	/* Associate the socket with local client IP address and port */
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}


	/* Initialize serverAddress memory to 0 */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set ftp server ftp address in serverAddress */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr,
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(DATA_CONNECTION_PORT);

	/* Connect to the server */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to server ");
		close (sock); 	/* close the control connection socket */
		return(ER_CONNECT_FAILED);  	/* error return */
	}


	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /* successful return */
}  // end of clntConnect() */

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
