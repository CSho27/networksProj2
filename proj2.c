//Chris Shorter
//This contains the main method for project 2
#include <stdio.h>
#include <string.h>

#define MAX_LINE 80

int main(){
	char line[50];
    char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	int run = 1;
	
    while(run == 1){
        printf("web-client>");
        fgets(line, sizeof(line), stdin);
		char *token = strtok(line, " ");
		int i=0;
		while(token != NULL){
			args[i] = token;
			fflush(stdout);
			if(strncmp(args[i], "-e", 2) == 0){
				run = 0;
			}
			token = strtok(NULL, " ");
			i++;
		}
	}
	return 0;
}