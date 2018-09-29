//Chris Shorter
//This contains the main method for project 2
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 80

int urlValid(char *url){
	
}

int main(int argc, char *argv[]){
	//booleans to record which flags are present
	bool valid = true; 				//No invalid args, includes URL, etc.
	bool url_present = false; 		//-u flag is there and a valid url follows it
	bool print_details = false;		//-d is present, print the details
	bool print_request = false;		//-r is present, so the program will print the get request
	bool print_response = false;	//-R is present, so the program will print the response it gets
	bool save_contents = false;		//-o is present, so the progrm will save the contents at the url to a file 
	
	//Strings for output file location and url
	char *url = "";
	char *contents_file = "";
	
	//check for flags/invalid arguments
	for(int i=1; argv[i] != NULL; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'u':
					url_present = true; 
					i++;
					url = argv[i];
					//have a urlValid() method here or something
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
					contents_file = argv[i];
					break;
				default:
					valid = false;
					break;
			}
		}else{
			valid = false;
		}
	}
	
	

	return 0;
}