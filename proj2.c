//Chris Shorter
//This contains the main method for project 2
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINE 80

//helps me add a character to a string because working with strings in C kinda sucks a little bit
char *append(char *str1, char character){
	char *new_string = malloc(100);
	sprintf(new_string, "%s%c", str1, character);
	return new_string;
}

//This checks the validity of the url and breaks the arguments into a form that strtok can easily make an array
char *processURL(char *url){
	char *processed_url = malloc(100);
	bool valid = true;
	const int url_http_length = 7;
	
	//checking for correct "http://"
	for(int i=0; i<7; i++){
		processed_url = append(processed_url, url[i]);
	}
	if(strncasecmp(processed_url, "http://", url_http_length) != 0)
		valid = false;
	
	processed_url = append(processed_url, ' ');
	int i = url_http_length;
	
	//adding hostname
	while(url[i] != ':' && url[i] != '/' && url[i] != '\0'){
		processed_url = append(processed_url, url[i]);
		i++;
	}
	
	processed_url = append(processed_url, ' ');
	
	//adding port #
	if(url[i] == ':'){
		i++;
		while(url[i] != '/' && url[i] != '\0'){
			processed_url = append(processed_url, url[i]);
			i++;
		}
	}else{
		processed_url = strcat(processed_url, "80");
	}
	
	processed_url = append(processed_url, ' ');
	
	//adding filename
	if(url[i] == '\0'){
		processed_url = append(processed_url, '/');
		i++;
	}
	else{
		while(url[i] != '\0'){
				processed_url = append(processed_url, url[i]);
				i++;
			}
	}
	
	if(valid)
		return processed_url;
	else
		return "invalid";
}

void printDetails(char *processed_url, char *output_filename){
	char *token = strtok(processed_url, " ");
	char *labels[4] = {"hostname", "port", "web_filename"};
	int i=0;
	token = strtok(NULL, " ");
	
	while(token != NULL){
		printf("DET: %s = %s\n", labels[i], token);
		fflush(stdout);
		token = strtok(NULL, " ");
		i++;
	}
	printf("DET: output_filename = %s\n", output_filename);
	fflush(stdout);
}

int main(int argc, char *argv[]){
	//booleans to record which flags are present
	bool valid = true; 				//No invalid args, includes URL, etc.
	bool url_present = false; 		//-u flag is there and a valid url follows it
	bool print_details = false;		//-d is present, print the details
	bool print_request = false;		//-r is present, so the program will print the get request
	bool print_response = false;	//-R is present, so the program will print the response it gets
	bool save_contents = false;		//-o is present, so the program will save the contents at the url to a file 
	
	//Strings for output file location and url
	char *url = "";
	char *output_filename = "";
	
	//check for flags/invalid arguments
	for(int i=1; argv[i] != NULL; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'u':
					url_present = true; 
					i++;
					url = argv[i];
					url = processURL(url);
					if(strcmp(url, "invalid") == 0)
						valid = false;
					break;
				case 'd':
					print_details = true;
					break;
				case 'r':
					print_request = true;
					break;
				case 'R':
					print_response = true;
					break;
				case 'o':
					save_contents = true;
					i++;
					output_filename = argv[i];
					break;
				default:
					valid = false;
					break;
			}
		}else{
			valid = false;
		}
	}
	printf("%d\n", valid);
		printf("%d %s\n", url_present, url);
		printf("%d\n", print_details);
		printf("%d\n", print_request);
		printf("%d\n", print_response);
		printf("%d %s\n", save_contents, output_filename);
	
	
	if(valid){
		if(print_details){
			printDetails(url, output_filename);
		}
	}
	
	

	return 0;
}