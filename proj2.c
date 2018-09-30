//Chris Shorter
//This contains the main method for project 2
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netinet/tcp.h>

#define ERROR 1
#define HOST_POS 1
#define PORT_POS 2
#define FILENAME_POS 3
#define PROTOCOL "tcp"
#define BUFLEN 10024

int errexit (char *format, char *arg){
    fprintf (stderr,format,arg);
    fprintf (stderr,"\n");
    exit (ERROR);
}

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

int getPortFromString(char *port_string){
	int port = 0;
	int place = 1;
	for(int i=strlen(port_string)-1; i>-1; i--){
		port += place*(((int) port_string[i])-48);
		place = place*10;
	}
	return port;
}

int socketSetup(char *host, in_port_t port){
	struct hostent *hinfo;
	struct sockaddr_in addr;
	int on = 1;
	int sock;     

	if((hinfo = gethostbyname(host)) == NULL){
		printf("Error: gethostbyname");
		exit(1);
	}
	bcopy(hinfo->h_addr, &addr.sin_addr, hinfo->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		//errexit("setsockopt");
		exit(1);
    }
	return sock;
}

void printDetails(char *url_array[], char *output_filename){
	char *labels[4] = {"hostname", "port", "web_filename"};
	for(int i=0; i<3; i++){
		printf("DET: %s = %s\n", labels[i], url_array[i+1]);
		fflush(stdout);
    }
    printf("DET: output_filename = %s\n", output_filename);
	fflush(stdout);
}

void printRequest(char *args[]){
    printf("REQ: GET %s HTTP/1.0\r\nREQ: Host: %s\r\nREQ: User-Agent: CWRU EECS 325 Client 1.0\r\n",
            args[3], args[1]);
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
	char *url_array[8];
	char *url = "";
	char *host;
	char *url_filename = "";
	char *output_filename = "none";
	
	int port = 0;

	
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
            printf("Error: no URL included");
               
		}
	}
	printf("%d\n", valid);
		printf("%d %s\n", url_present, url);
		printf("%d\n", print_details);
		printf("%d\n", print_request);
		printf("%d\n", print_response);
		printf("%d %s\n", save_contents, output_filename);
	
	
	if(valid){
        char *token = strtok(url, " ");
        int i = 0;
        while (token != NULL){
           url_array[i] = token;
           token = strtok(NULL, " ");
           i++;
		}
        
		if(print_details)
			printDetails(url_array, output_filename);
	
		host = url_array[HOST_POS];
		port = getPortFromString(url_array[PORT_POS]);
        url_filename = url_array[FILENAME_POS];
		
		int sd = socketSetup(host, port);
        printf("socket: %d\n", sd);
        int ret = -1;
		char buffer [BUFLEN];
        char request[100];
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        if(print_request)
            printRequest(url_array);
        printf("return:%d\n", ret);
        fflush(stdout);
		write(sd, request, strlen(request));
        bzero(buffer, BUFLEN);
        
        memset (buffer,0x0,BUFLEN);
        ret = read (sd,buffer,BUFLEN - 1);
        printf("return:%d\n", ret);
        fflush(stdout);
        if (ret < 0)
            printf("reading error");
        fprintf (stdout,"%s\n",buffer);
        fflush(stdout);
		
        shutdown(sd, SHUT_RDWR); 
        close(sd);
        
        
		
	}
	
	

	return 0;
}