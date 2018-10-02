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
#define URL_ARGS_LEN 5
#define HTTP_START "http://"
#define LABELS_LEN 3
#define EMPTY_LINE "\r\n\r\n"
#define END_STRING '\0'
#define DEFAULT_PORT_STR "80"

//If there's any sort of error the program exits immediately.
int errexit (char *format, char *arg){
    fprintf (stdout,format,arg);
    fprintf (stdout,"\n");
    exit (ERROR);
}

//This checks the validity of the url and breaks the arguments into a form that strtok can easily make an array
char *processURL(char *url){
    if(url == NULL)
            return "empty";
    
    //create an array of double URL length just make sure this array is always longer than the URL, even for very short URLs
    //This uses two indexes to build a string with separated url arguments fro the given URL
	char processed_url[strlen(url)*2];
    int url_index = 0;
    int purl_index = 0;
    
	//checking for correct "http://" at beginning, if so add http:// to the processed URL string"
    if(strncasecmp(url, HTTP_START, strlen(HTTP_START)) != 0){
        return "invalid";
    }
    else{
        for(int i=0; i<strlen(HTTP_START); i++){
            processed_url[purl_index] = HTTP_START[i];
            purl_index++;
        }
    }
    
    url_index = strlen(HTTP_START);
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding hostname from URL to processed URL string
	while(url[url_index] != ':' && url[url_index] != '/' && url[url_index] != END_STRING){
		processed_url[purl_index] = url[url_index];
        url_index++;
		purl_index++;
	}
	
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding port # to processed URL string
	if(url[url_index] == ':'){
		url_index++;
		while(url[url_index] != '/' && url[url_index] != END_STRING){
			processed_url[purl_index] = url[url_index];
            url_index++;
            purl_index++;
		}
	}else{
		for(int i=0; i<strlen(DEFAULT_PORT_STR); i++){
            processed_url[purl_index] = DEFAULT_PORT_STR[i];
            purl_index++;
        }
	}
	
	processed_url[purl_index] = ' ';
    purl_index++;
	
	//adding filename to processed URL string
	if(url[url_index] == END_STRING){
			processed_url[purl_index] = '/';
            purl_index++;
	}
	else{
        //Everything else here until end of string character is part of filename
		while(url[url_index] != END_STRING){
				processed_url[purl_index] = url[url_index];
                url_index++;
                purl_index++;
			}
	}
    
    //Creating a string literal that can be returned
    processed_url[purl_index] = END_STRING;
    char *url_return = malloc(strlen(processed_url));
    strcpy(url_return, processed_url);
    return url_return;
}

//Just takes a string and turns it into an integer.
//Realized later a method already exists for this but I enjoyed the challenge.
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
	const char *LABELS[3] = {"hostname", "port", "web_filename"};
	for(int i=0; i<LABELS_LEN; i++){
		printf("DET: %s = %s\n", LABELS[i], url_array[i+1]);
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
void printResponse(unsigned char *contents){
    char *response = (char*) contents;
    printf("RSP: ");
    fflush(stdout);
    char *end;
    end = strstr(response, EMPTY_LINE);
    int end_index = (end ? end-response : -1)+1;
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

//Writes the the HTML contents of the webpage to a file. Does so by appending an existing file.
bool printToFile(char *filename, unsigned char *contents, bool header_present){
    char *response = (char*) contents;
    FILE *f = fopen(filename, "a");
    if (f == NULL){
        return false;
    }
    
    int start_index = 0;
    if(header_present){
        char *start = strstr(response, EMPTY_LINE);
        start_index = (start ? start-response : -1)+strlen(EMPTY_LINE);
    }
    
    for(int i=start_index; i<strlen(response); i++){
        fwrite(contents, 1, 1, f);
    }
    fclose(f);
    return true;
} 

//Creates or overwrites the file at the location given by the user to be clear to start appending new content
bool createAndClearFile(char *filename){
    FILE *f = fopen(filename, "w");
    if(f == NULL){
        return false;
    }
    else{
        fclose(f);
        return true;
    }
}
    
//This main method processes flags and makes the actual socket connection and HTTP request
int main(int argc, char *argv[]){
	//booleans to record which flags are present
    bool url_present = false;        //-u is present, so the command is valid
	bool print_details = false;		//-d is present, print the details
	bool print_request = false;		//-r is present, so the program will print the get request
	bool print_response = false;	//-R is present, so the program will print the response it gets
	bool save_contents = false;		//-o is present, so the program will save the contents at the url to a file 
	
	//Strings for output file location and url
	char *url_array[URL_ARGS_LEN];
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
        unsigned char buffer [BUFLEN];
        int sd;

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
        char request[100+strlen(url_filename)+strlen(host)];
        //request some stuff
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: CWRU EECS 325 Client 1.0\r\n\r\n", url_filename, host);
        //If the user requested to print request do it
        if(print_request)
            printRequest(url_array);
        write(sd, request, strlen(request));
        bzero(buffer, BUFLEN);
        
        if(!createAndClearFile(output_filename))
            errexit("ERROR: Error opening file. Check permissions.", NULL);
        
        int ret = 1;
        bool handled_header = false;
        bool ok = false; //gets 200 OK response
        //Cycle through reading data and printing what the buffer captures until the return value of read is no longer positive
        while(ret>0){
            /* snarf whatever server provides and print it */
            memset (buffer,0x0,BUFLEN);
            ret = read (sd,buffer,BUFLEN - 1);
            if (ret < 0)
                errexit ("ERROR: reading error",NULL);
            //buffer[ret+1] = END_STRING;
            
            //If it's the first time cycling through print the HTTP response header
            if(print_response && !handled_header)
                printResponse(buffer);
            //Only print if response code is 200 OK
            if(strstr((char*) buffer, "200")==NULL && !handled_header && !ok){
                errexit("ERROR: Could not find content at URL and will not write to file.", NULL);
            }
            else{
                ok = true;
            }
            if(save_contents & ok){
                if(!printToFile(output_filename, buffer, !handled_header))
                    errexit("ERROR: writing to file", NULL);
            }
            handled_header = true;
        }
        
        /* close & exit */
            close (sd);
            exit (0);
	}
	
	

	return 0;

}