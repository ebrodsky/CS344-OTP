#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ctype.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int getNumChars(const char* filename);

int main(int argc, char *argv[]) //where communication between client and server happens
{
	int socketFD, portNumber, charsWritten, charsRead, bytesread;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];
	char ciphertext[100000];
	if (argc < 3) { fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args
																					  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
																										// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); //make socket reusable

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){ // Connect socket to address
		error("CLIENT: ERROR connecting");
	}

	long filelength = getNumChars(argv[1]);
	long keylength = getNumChars(argv[2]);

    if(filelength > keylength){ //if file is greater than key
        printf("Key is too short!\n");
		error("Key is too short!\n");
	}
	
	char* msg = "otp_dec";
	charsWritten = send(socketFD, msg, strlen(msg), 0); // ASK SERVER FOR CONFIRMATION
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	if (charsWritten < 0) 
		error("CLIENT: ERROR writing from socket");
	charsRead = 0;
	while(charsRead == 0) //GET RETURN CONFIRMATION FROM SERVER
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end //RECEIVE RESPONSE
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	if(strcmp(buffer, "no") == 0){ //check if connecting to good port.
		//error("Can't connect this. Incompatible.\n");
		fprintf(stderr, "Bad client\n");
		//printf("%s\n", buffer);
		exit(2);
	}
	//at this point, key length, bad characters and port have all been checked. Assume everything is OK.
	//send size of file
	//printf("response: %s\n", buffer);
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	sprintf(buffer, "%d", filelength);
	//printf("file length: %d", filelength);
	//printf("buffer: %s\n", buffer);
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0); //SEND FILE LENGTH
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = 0;
	while(charsRead == 0)
		charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
	char* junk;
	if(strcmp(buffer, "cont") == 0){
		int fd = open(argv[1], 'r');
		charsWritten = 0;
		//SEND FILE TEXT
		while(charsWritten <= filelength){
			memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
			bytesread = read(fd, buffer, sizeof(buffer)-1);
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);
			memset(buffer, '\0', 1024); //clear buffer
		}
		//SEND KEY TEXT
		fd = open(argv[2], 'r');
		charsWritten = 0;
		while(charsWritten <= filelength){
			memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
			bytesread = read(fd, buffer, sizeof(buffer)-1);
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);
			memset(buffer, '\0', 1024); //clear buffer
		}
	}
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

	//RECEIVE MESSAGE
	charsRead = 0;
	int charsSent = 0;
	//printf("size: %d", filelength);
	while(charsRead < filelength){
		memset(buffer, '\0', 1024); //clear buffer
		charsSent = recv(socketFD, buffer, sizeof(buffer)-1, 0); //RECEIVE THE MESSAGES
		charsRead += charsSent;
		charsSent = 0;
		strcat(ciphertext, buffer);
		//printf("charsRead: %d\n", charsRead);
		memset(buffer, '\0', 1024); //clear buffer
	}
	strcat(ciphertext, "\n");
	printf("%s", ciphertext);
	close(socketFD); // Close the socket
	return 0;
}

int getNumChars(const char* filename){
	int character;
	int count = 0;
	FILE* file = fopen(filename, "r");

    while (1) {
        character = fgetc(file);

        if (character == EOF || character == '\n')
            break;
		if(!isupper(character) && character != ' '){
            //printf("File contains bad characters!\n");
            error("File contains bad characters!\n");
        }
        ++count;
    }
	fclose(file);
	//printf("File %s has %d characters\n", filename, count);
	return count;
}
