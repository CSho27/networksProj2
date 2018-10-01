//Chris Shorter
//cws68
//proj2.c
//9-29-18
//This contains all methods for project 2, which makes HTTP GET requests, prints responses, and writes contents to files
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
#define BUFLEN 2048
#define RSP_LEN 7

int errexit (char *format, char *arg){
    fprintf (stdout,format,arg);
    fprintf (stdout,"\n");
    exit (ERROR);
}

//This checks the validity of the url and breaks the arguments into a form that strtok can easily make an array
char *processURL(char *url){
    if(url == NULL)
            return "empty";
    
	char processed_url[strlen(url)*2];
	const char *http_start = "http://"; 
    int url_index = 0;
    int purl_index = 0;
    
	//checking for correct "http://"
    if(strncasecmp(url, http_start, strlen(http_start)) != 0){
        return "invalid";
    }
    else{
        for(int i=0; i<strlen(http_start); i++){
            processed_url[purl_index] = http_start[i];
            purl_index++;
        }
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

//Prints the details provided by the user that will be used in the GET
void printDetails(char *url_array[], char *output_filename){
	char *labels[4] = {"hostname", "port", "web_filename"};
	for(int i=0; i<3; i++){
		printf("DET: %s = %s\n", labels[i], url_array[i+1]);
		fflush(stdout);
    }
    printf("DET: output_filename = %s\n", output_filename);
	fflush(stdout);
}

//Prints the request just as it is written to the socket
void printRequest(char *args[]){
    printf("REQ: GET %s HTTP/1.0\r\nREQ: Host: %s\r\nREQ: User-Agent: CWRU EECS 325 Client 1.0\r\n", args[FILENAME_POS], args[HOST_POS]);
    fflush(stdout);
}

//Prints the HTTP response just as it was received from the socket
void printResponse(char *response){
    printf("RSP: ");
    fflush(stdout);
    char *end;
    end = strstr(response, "\r\n\r\n");
    int end_index = (end ? end-response : -1);
    for(int i=0; i<end_index; i++){
        if(response[i]=='\n'){
            printf("\nRSP: ");
            fflush(stdout);
        }
        else{
            printf("%c", response[i]);
            fflush(stdout);
        }
    }
    printf("%c", response[end_index]);
}

//Writes the the HTML contents of the webpage to a file
bool printToFile(char *filename, char *response){
    FILE *f = fopen(filename, "w");
    if (f == NULL){
        return false;
    }
    char *start;
    start = strstr(response, "\r\n\r\n");
    int start_index = (start ? start-response : -1)+4;
    for(int i=start_index; i<strlen(response); i++){
        fprintf(f, "%c", response[i]);
    }
    fclose(f);
    return true;
}            

int main(int argc, char *argv[]){
	//booleans to record which flags are present
    bool url_present = false;        //-u is present, so the command is valid
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
					i++;
					url = argv[i];
					url = processURL(url);
					if(strcmp(url, "invalid") == 0){
						errexit("ERROR: URL entered was invalid\n(Only \"http\" URLs are accepted)", NULL);
                    }
                    else{
                        if(strcmp(url, "empty") == 0)
                            errexit("ERROR: please enter a URL follwing the -u flag", NULL);
                        else
                            url_present = true;
                    }
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
                    if(argv[i] == NULL || argv[i][0] == '-')
                        errexit("ERROR: -o flag must be followed by a filename", NULL);
                    else
					    output_filename = argv[i];
					break;
				default:
					errexit("ERROR: Invalid flag. Enter -u followed by a URL to make an HTTP request to that webpage", NULL);
					break;
			}
		}
        else{
            errexit("ERROR: Either no flags were entered or an invalid argument was passed.\nValid flags/arguments are: -u <url>, -d, -r, -R, -o <filename>\nEnter -u followed by a URL to make an HTTP request to that webpage", NULL);
		}
	}
    
    if(!url_present){
        errexit("ERROR: -u flag not included. Enter -u followed by a URL to make an HTTP request to that webpage", NULL);
    }
	else{
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
            errexit ("ERROR: cannot find name: %s", host);

        /* set endpoint information */
        memset ((char *)&sin, 0x0, sizeof (sin));
        sin.sin_family = AF_INET;
        sin.sin_port = htons (port);
        memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

        if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
            errexit ("ERROR: cannot find protocol information for %s", PROTOCOL);

        /* allocate a socket */
        /*   would be SOCK_DGRAM for UDP */
        sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
        if (sd < 0)
            errexit("ERROR: cannot create socket",NULL);

        /* connect the socket */
        if (connect (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            errexit ("ERROR: cannot connect", NULL);
        
        //compose and make request
        char request[100];
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        //request some stuff
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        write(sd, request, strlen(request));
        bzero(buffer, BUFLEN);

        /* snarf whatever server provides and print it */
        memset (buffer,0x0,BUFLEN);
        ret = read (sd,buffer,BUFLEN - 1);
        if (ret < 0)
            errexit ("ERROR: reading error",NULL);
        buffer[ret+1] = '\0';
        
        //Finish operating upon flags
        if(print_request)
            printRequest(url_array);
        if(print_response)
            printResponse(buffer);
        if(save_contents){
            if(strstr(buffer, "200")==NULL){
                errexit("ERROR: Could not find content at URL and will not write to file.", NULL);
            }
            else{
                if(!printToFile(output_filename, buffer))
                errexit("ERROR: Error opening or writing to file", NULL);
            }
        }
    
        /* close & exit */
        close (sd);
        exit (0);
	}
	
	

	return 0;

}