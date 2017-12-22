#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int charToInt (char c){ //convert a character to an integer
	if (c == ' '){
		return 26;
	}
	else {
		return (c - 'A');
	}
	return 0;
}

char intToChar(int i){ //convert an integer to a character
	if (i == 26){
		return ' ';
	}
	else {
		return (i + 'A');
	}
}

void decode(char message[], char key[]){ //decode the message
	  int i;
	  char n;

	  for (i=0; message[i] != '\n' ; i++){

	  		n = charToInt(message[i]) - charToInt(key[i]);
	  		if (n<0){
	  			n += 27;
	  		}
	  		message[i] = intToChar(n);

	  }
	  message[i] = '\0';
	  return;
}

int main(int argc, char *argv[]) //main function where server is initialized and communicates with client
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, status;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;

	if (argc < 2) { fprintf(stderr, "USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

																			 // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

												// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	while(1){ //server infinite loop
		listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections	

								   // Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

		//server loop
		// Get the message from the client and display it
		pid = fork();
		switch (pid){
			case -1:{
				error("Hull Breach! Couldn't fork!\n");
			}
			case 0:{
				char buffer[1024];
				char* encryptedMessage[100000];
				char message[100000];
				char key[100000];
				int charsWritten = 0;
				memset(buffer, '\0', 1024); //CHECK FOR RIGHT CONNECTION
				charsRead = 0;
				while(charsRead == 0)
					charsRead = recv(establishedConnectionFD, buffer, 1023, 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
					//printf("SERVER: I received this from the client: \"%s\"\n", buffer);
				if(strcmp(buffer, "otp_dec") != 0){
					charsRead = send(establishedConnectionFD, "no", 2, 0);
					exit(2);
				}else{
					memset(buffer, '\0', 1024); //clear buffer
					charsRead = send(establishedConnectionFD, "yes", 3, 0); //SEND RESPONSE
					charsRead = 0;
					while(charsRead == 0){ //RECEIVE FILE LENGTH
						charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE FILE LENGTH
						//printf("%d\n", charsRead);

					}
					//printf("buffer on server: %s", buffer);
					int size = atoi(buffer);
					//printf("size of file on server: %d\n", size);

					charsRead = send(establishedConnectionFD, "cont", 4, 0);//continue
					charsRead = 0;
					int charsSent = 0;
					//printf("size: %d", size);
					while(charsRead < size){
						memset(buffer, '\0', 1024); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE THE MESSAGES
						charsRead += charsSent;
						charsSent = 0;
						strcat(message, buffer);
						//printf("charsRead: %d\n", charsRead);
						memset(buffer, '\0', 1024); //clear buffer
						//charsWritten = send(establishedConnectionFD, "next", 4, 0); //ask for more
					}
					//printf("%d ", charsRead);
					//printf("%s\n", message);
					//charsWritten = send(establishedConnectionFD, "givekey", 7, 0);
					charsRead = 0;
					charsSent = 0;
					//printf("size: %d", size);
					while(charsRead < size){
						memset(buffer, '\0', 1024); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE THE MESSAGES
						charsRead += charsSent;
						charsSent = 0;
						strcat(key, buffer);
						//printf("charsRead: %d\n", charsRead);
						memset(buffer, '\0', 1024); //clear buffer
					}
					//printf("%s\n", message);
					//printf("%s", key);
					//printf("ENCRYPTED TEXT:\n\n\n\n\n");

					//encrypt
					decode(message, key);
					//printf("%s\n", message);
					memset(buffer, '\0', 1024); //clear buffer

					//SEND BACK TO CLIENT

					charsWritten = 0;
					while(charsWritten < size){
						memset(buffer, '\0', sizeof(buffer)); //clear out buffer
						charsWritten += send(establishedConnectionFD, message, sizeof(message), 0);
						memset(buffer, '\0', sizeof(buffer)); //clear out buffer						
					}	

					exit(0);

				}
			}
			default:{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}

