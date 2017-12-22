#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]) {
	srand(time(NULL));
	
	if (argc != 2) { //print usage error if wrong usage
		printf("Usage: %s length\n", argv[0]);
		exit(0);
	}

	int length = atoi(argv[1]);
	char key[length + 1]; //+ 1 for the null terminator
	int i;
	char* arr =" ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (i = 0; i < length; i++) //fill with random CAPS letters
		//key[i] = 'A' + random() % 26;
		key[i] = arr[random() % 27];

	key[length] = '\0'; //set last char to null terminator

	printf("%s\n", key); 
	return 0;
}