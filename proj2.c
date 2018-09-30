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
#define REQUIRED_ARGC 3
#define HOST_POS 1
#define PORT_POS 2
#define FILENAME_POS 3
#define PROTOCOL "tcp"
#define BUFLEN 1024

int errexit (char *format, char *arg){
    fprintf (stderr,format,arg);
    fprintf (stderr,"\n");
    exit (ERROR);
}

//This checks the validity of the url and breaks the arguments into a form that strtok can easily make an array
char *processURL(char *url){
	char processed_url[strlen(url)*2];
	const char *http_start = "http://"; 
    int url_index = 0;
    int purl_index = 0;
    
	//checking for correct "http://"
    if(url == NULL || strncasecmp(url, http_start, strlen(http_start)) != 0){
        return "invalid";
    }
    else
        for(int i=0; i<strlen(http_start); i++){
            processed_url[purl_index] = http_start[i];
            purl_index++;
        }
    
    url_index = strlen(http_start);
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding hostname
	while(url[url_index] != ':' && url[url_index] != '/' && url[url_index] != '\0'){
		processed_url[purl_index] = url[url_index];
        url_index++;
		purl_index++;
	}
	
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding port #
	if(url[url_index] == ':'){
		url_index++;
		while(url[url_index] != '/' && url[url_index] != '\0'){
			processed_url[purl_index] = url[url_index];
            url_index++;
            purl_index++;
		}
	}else{
		for(int i=0; i<strlen("80"); i++){
            processed_url[purl_index] = "80"[i];
            purl_index++;
        }
	}
	
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding filename
	if(url[url_index] == '\0'){
			processed_url[purl_index] = '/';
        purl_index++;
	}
	else{
		while(url[url_index] != '\0'){
				processed_url[purl_index] = url[url_index];
                url_index++;
                purl_index++;
			}
	}
    processed_url[purl_index] = '\0';
	printf("PROCESSED: %s\n", processed_url);
    fflush(stdout);
    char *url_return = malloc(strlen(processed_url));
    strcpy(url_return, processed_url);
    return url_return;
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
	char *url_array[8];// = {"http://", "www.clevelandparkingtickets.com", "80", "/"};
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
		
		struct sockaddr_in sin;
        struct hostent *hinfo;
        struct protoent *protoinfo;
        char buffer [BUFLEN];
        int sd, ret;

        /* lookup the hostname */
        hinfo = gethostbyname (host);
        if (hinfo == NULL)
            errexit ("cannot find name: %s", host);

        /* set endpoint information */
        memset ((char *)&sin, 0x0, sizeof (sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons (port);
        memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

        if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
            errexit ("cannot find protocol information for %s", PROTOCOL);

        /* allocate a socket */
        /*   would be SOCK_DGRAM for UDP */
        sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
        if (sd < 0)
            errexit("cannot create socket",NULL);

        /* connect the socket */
        if (connect (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            errexit ("cannot connect", NULL);

        char request[100];
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        //request some shit
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        write(sd, request, strlen(request));
        bzero(buffer, BUFLEN);

        /* snarf whatever server provides and print it */
        memset (buffer,0x0,BUFLEN);
        ret = read (sd,buffer,BUFLEN - 1);
        if (ret < 0)
            errexit ("reading error",NULL);
        fprintf (stdout,"%s\n",buffer);

        /* close & exit */
        close (sd);
        exit (0);
        
		
	}
	
	

	return 0;

}