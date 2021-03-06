/* 
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Xin Huang, xyh1@rice.edu 
 *     Leo Meister, lpm2@rice.edu
 * 
 */ 

#include <stdbool.h>
#include "csapp.h"
#include "sbuf.c"

/*
 * Function prototypes
 */
ssize_t Rio_readn_w(int fd, void *ptr, size_t nbytes);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen);
int parse_uri(char *uri, char *hostname, char *pathname, int *port);
int Open_clientfd_ts(char *hostname, int port);
int open_clientfd_ts(char *hostname, int port);
void Rio_writen_w(int fd, void *usrbuf, size_t n);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
    char *uri, int size);
void logging(char *logString, char *fileName);
void read_requesthdrs(rio_t *rp) ;
void doit(int fd); //handle requests

#define SIZEOF_GET 4
#define SIZEOF_VERSION 9
unsigned int number_Requests = 0;
bool verbose = true;
//static char GET[4] = "GET";
static char connection_hdr[MAXLINE] = "Connection: close\r\n";

/* 
 * main - Main routine for the proxy program 
 */
int
main(int argc, char **argv)
{	
	//int i;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	rio_t client_rio, server_rio;
	char buf[MAXLINE], host_name[MAXLINE], logstring[MAXLINE];
	char path_name[MAXLINE], uri[MAXLINE];
	char haddrp[INET_ADDRsizeof];
	char version[SIZEOF_VERSION];
	char method[SIZEOF_GET]; 
	char *request;
	//char *host_header = "Host: "; //hackish solution to strcat?
	//int listenfd, port;//, error;
	int conn_to_clientfd, conn_to_serverfd, cur_bytes, listenfd, port;
	//the number of bytes read in from a single read
	unsigned int num_bytes;	//the number of bytes returned in the server response
	
	if (argc != 2) {
        	fprintf(stderr, "usage: %s <port>\n", argv[0]);
        	exit(0);
	}
	
	// Handle sigpipe signals
	Signal(SIGPIPE, SIG_IGN);
	
	port = atoi(argv[1]);
	listenfd = Open_listenfd(port);
	
	while (1) {
    		
		num_bytes = 0;
		cur_bytes = 0;

		clientlen = sizeof(clientaddr);
		conn_to_clientfd = Accept(listenfd, (SA *) &clientaddr,
		    &clientlen);
		
		Rio_readinitb(&client_rio, conn_to_clientfd);
		Rio_readlineb_w(&client_rio, buf, MAXLINE);
		
		if (verbose)
			printf("Request line: %s\n", buf);
		
		
		/*memcpy(method, &buf[0], 3);
		method[3] = '\0';
		//int space_index = 0;
		i = 4;
		do {
			uri[i-4] = buf[i];
			i++;
		} while(buf[i] != ' ');
		
		uri[i] = ' ';
		uri[i+1] = 'a';
		
		memcpy(version, &buf[sizeof(buf)-10], sizeof(buf));
		version[8] = '\0'; */
		
		if (sscanf(buf, "%s %s %s", method, uri, version) <= 0) {
			
			printf("Error with sscanf\n");
			Close(conn_to_clientfd);
			continue;	
		}
		
		if (verbose)
			printf("Method: %s\nURI: %s\nVersion: %s\n", method, uri, version);		
		
		if (strstr(buf, "GET") == NULL) {
			printf("Error! Expected GET request; any other unsupported.\n");
			Close(conn_to_clientfd);
			continue;
		}		
	
		if (parse_uri(&buf[4], host_name, path_name, &port) < 0) {
			printf("Error parsing URI!\n");
			Close(conn_to_clientfd);
			continue;
		}
		
		if (verbose)
			printf("host_name: %s\npath_name: %s\nport: %d\n", host_name, path_name, port);
		inet_ntop(AF_INET, &clientaddr.sin_addr, haddrp, 
		    INET_ADDRsizeof);

		// Print statements like proxyref
		printf("Request %u: Received request from %s (%s)\n", 
			number_Requests, host_name, haddrp);
		printf("%s %s %s\n", method, uri, version);
		printf("\n*** End of Request ***\n\n");
		
		
		if (strcasecmp(version, "HTTP/1.1") == 0) {
			request = strcat(method, " ");
			request = strncat(request, path_name, sizeof(path_name));
			request = strcat(request, " ");
			request = strncat(request, version, sizeof(version));
			request = strcat(request, "\r\n");
		} else
			strncpy(request, buf, sizeof(buf));
		
		printf("*****Earlier request*****\n%s", request);

		/* 
		* open connection to server, read request into server, 
		* making sure to use parsed pathname, not full url
		*/
		if (verbose)
			printf("Opening connection to server with:\nhost_name: %s\nport: %d\n", host_name, port);
		
		
		
		//[TODO] add error check
		Rio_writen_w(conn_to_serverfd, request, sizeof(request));
		
		if (verbose)
			printf("Wrote request to server: %s\n", request);
		
		/*if (strcasecmp(version, "HTTP/1.1") == 0) {
			//request = strcat(request, "Connection: close\r\n");
			
			Rio_writen_w(conn_to_serverfd, request, 
			    sizeof(request));
			printf("%s", request);
			
			
			printf("%s", connection_hdr);
		}*/
		
		//if HTTP/1.1, it requires a host header Host: host_name
		//[TODO] Strip Proxy-Connection and Connection headers
		// out of the request, add in Connection: close if using 
		// HTTP/1.1
		request = strncat(request, connection_hdr, sizeof(connection_hdr));
    		while(strcmp(buf, "\r\n")) {
			
			Rio_readlineb(&client_rio, buf, MAXLINE);
			
			if (strstr(buf, "Connection: ") != NULL)
				continue;
			else
				request = strncat(request, buf, sizeof(buf));
    		}
    		
    		
    		printf("Request:\n%s", request);
    		
    		if ((conn_to_serverfd = Open_clientfd_ts(host_name,
		    port)) < 0) {
		    Close(conn_to_clientfd);
		    continue;
		}
		
    		Rio_writen_w(conn_to_serverfd, request, sizeof(request));
    		
    		
    		//Rio_writen_w(conn_to_serverfd, buf, sizeof(buf));	
		
		// Print statements like proxyref
		printf("Request %u: Forwarding request to end server\n", 
			number_Requests);
		printf("%.14s\n", method);
		printf("\n*** End of Request ***\n\n");

		if (verbose)
			printf("Preparing to read reply to client\n");
		
		//int flag = 0;
		//int content_len = 0;
		//char dontcare[MAXLINE];
		//receive reply and forward it to browser
		//while(read != 0) increment num_bytes during this
		
		// Read in response headers
		//buf[0] = ' ';
		
		Rio_readinitb(&server_rio, conn_to_serverfd);
		
		/*while(strcmp(buf, "\r\n")) {
			printf("Resp headers\n");
			num_bytes += Rio_readlineb(&server_rio, buf, MAXLINE);
			Rio_writen_w(conn_to_clientfd, buf, sizeof(buf));
			
			if (strstr(buf, "Content-length:") != NULL) {
				printf("Scanning for content length\n");
				sscanf(buf, "%s %d", dontcare, &content_len);
				printf("Header: %s\nLength: %d\n", dontcare, content_len);
			}
			
			if (verbose)
				printf("%s", buf);
    		}*/
    		
    		// Read in response content
    		while ((cur_bytes = Rio_readn_w(conn_to_serverfd, buf, MAXLINE))
    		    > 0) {
    			num_bytes += cur_bytes;
    			printf("%s", buf);
    			Rio_writen_w(conn_to_clientfd, buf, cur_bytes);
    		}
    		

		if (verbose)
			printf("Closing connection to server\n");
		
		Close(conn_to_serverfd);

		// Print statements like proxyref
		printf("Request %u: Forwarded %d bytes from end server to client\n", 
			number_Requests, num_bytes);

		if (verbose)
			printf("Closing connection to client\n");
		
		Close(conn_to_clientfd);

		if (verbose)
			printf("Writing log to file\n");
		
		format_log_entry(logstring, &clientaddr, uri, num_bytes); // [TODO] This is very slow
		logging(logstring, "proxy.log");

		if (verbose)
			printf("Finished writing log\n");

		number_Requests += 1; // iterate request count
	}	
		
	Close(listenfd);

	exit(0);
}

void
doit (int fd)
{
	fd = (int)fd;
	

}

/******************************** 
 * Client/server helper functions
 ********************************/
/*
 * logging - Make log entries
 *
 * Requests:
 *	Log string to be entered
 *	Name of log file
 * 
 * Effects:
 *  Create log file if DNE; else open
 *	Append log string to end of file
 */
void
logging(char *logString, char *fileName)
{
	// use csapp function calls
	FILE *logFile = Fopen(fileName, "ab+"); // able to read/write binary files
	fprintf(logFile, "%s\n", logString);
	Fclose(logFile);
}

/*
 * open_clientfd - open connection to server at <hostname, port> 
 *   and return a socket descriptor ready for reading and writing.
 *   Returns -1 and sets errno on Unix error. 
 *   Returns -2 and sets h_errno on DNS (getaddrinfo) error.
 */
/* $begin open_clientfd */
int open_clientfd_ts(char *hostname, int port)
{
    int clientfd, ai;
    struct addrinfo *ai_struct;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	return -1; /* check errno for cause of error */

    /* Use getaddrinfo for server IP */
    if ((ai = getaddrinfo(hostname, NULL, NULL, &ai_struct)) != 0) {
    	fprintf(stderr, "DNS error: %s\n", gai_strerror(ai));
    	return -2; /* check h_errno for cause of error */
    } 

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    bcopy(ai_struct->ai_addr, (struct sockaddr *)&serveraddr, 
    	ai_struct->ai_addrlen);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0) {
		printf("Error in ts: %s\n", strerror(errno));
		return -1;
    }

	freeaddrinfo(ai_struct);
    return clientfd;
}
/* $end open_clientfd */


/*
 *
 *
 *
 * read_requesthdrs - read and parse HTTP request headers
 */
void
read_requesthdrs(rio_t *rp) 
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
	Rio_readlineb(rp, buf, MAXLINE);
	printf("%s", buf);
    }
    return;
}

/**********************************
 * Wrappers for robust I/O routines
 **********************************/
/*
 *
 */
int
Open_clientfd_ts(char *hostname, int port) 
{
    int rc;

    if ((rc = open_clientfd_ts(hostname, port)) < 0) {
	if (rc == -1)
		fprintf(stderr, "Unix error in open_clientfd_ts!\n");

	else        
		fprintf(stderr, "DNS error: %s\n", gai_strerror(rc));
    }
    return rc;
}

/*
 *
 */
ssize_t
Rio_readn_w(int fd, void *ptr, size_t nbytes) 
{
	ssize_t readn;
	if ((readn = rio_readn(fd, ptr, nbytes)) < 0) {
		fprintf(stdout, "Error! Failed to read request: %s\n", 
			strerror(errno));
		return 0;
	}
	return readn;
}

/*
 *
 */
void 
Rio_writen_w(int fd, void *usrbuf, size_t n)
{
	if (rio_writen(fd, usrbuf, n) != (int) n)
		fprintf(stdout, "Error! Failed to write into file: %s\n", 
			strerror(errno));
}

/*
 *
 */
ssize_t
Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen)
{
	ssize_t readline;
	if ((readline = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
		fprintf(stdout, "Error! Failed to read line: %s\n", strerror(errno));
		return 0;
	}
    return readline;
}

/* --- Given Functions --- */

/*
 * parse_uri - URI parser
 * 
 * Requires: 
 *   The memory for hostname and pathname must already be allocated
 *   and should be at least MAXLINE bytes.  Port must point to a
 *   single integer that has already been allocated.
 *
 * Effects:
 *   Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 *   the host name, path name, and port.  Return -1 if there are any
 *   problems and 0 otherwise.
 */
int 
parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
	int len, i, j;
	char *hostbegin;
	char *hostend;
	if (strncasecmp(uri, "http://", 7) != 0) {
		hostname[0] = '\0';
		return (-1);
	}
	   
	/* Extract the host name. */
	hostbegin = uri + 7;
	hostend = strpbrk(hostbegin, " :/\r\n");
	if (hostend == NULL)
		hostend = hostbegin + sizeof(hostbegin);
	len = hostend - hostbegin;
	strncpy(hostname, hostbegin, len);
	hostname[len] = '\0';
	
	/* Look for a port number.  If none is found, use port 80. */
	*port = 80;
	if (*hostend == ':')
		*port = atoi(hostend + 1);
	
	/* Extract the path. */
	for (i = 0; hostbegin[i] != '/'; i++) {
		if (hostbegin[i] == ' ') 
			break;
	}
	if (hostbegin[i] == ' ')
		strcpy(pathname, "/");
	else {
		for (j = 0; hostbegin[i] != ' '; j++, i++) 
			pathname[j] = hostbegin[i];
		pathname[j] = '\0';
	}

	return (0);
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 *
 * Requires:
 *   The memory for logstring must already be allocated and should be
 *   at least MAXLINE bytes.  Sockaddr must point to an allocated
 *   sockaddr_in structure.  Uri must point to a properly terminated
 *   string.
 *
 * Effects:
 *   A properly formatted log entry is stored in logstring using the
 *   socket address of the requesting client (sockaddr), the URI from
 *   the request (uri), and the size in bytes of the response from the
 *   server (size).
 */
void
format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri,
    int size)
{
	time_t now;
	unsigned long host;
	unsigned char a, b, c, d;
	char time_str[MAXLINE];

	/* Get a formatted time string. */
	now = time(NULL);
	strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z",
	    localtime(&now));

	/*
	 * Convert the IP address in network byte order to dotted decimal
	 * form.  Note that we could have used inet_ntoa, but chose not to
	 * because inet_ntoa is a Class 3 thread unsafe function that
	 * returns a pointer to a static variable (Ch 13, CS:APP).
	 */
	host = ntohl(sockaddr->sin_addr.s_addr);
	a = host >> 24;
	b = (host >> 16) & 0xff;
	c = (host >> 8) & 0xff;
	d = host & 0xff;

	/* Return the formatted log entry string */
	sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri,
	    size);
}

/*
 * The last lines of this file configure the behavior of the "Tab" key in
 * emacs.  Emacs has a rudimentary understanding of C syntax and style.  In
 * particular, depressing the "Tab" key once at the start of a new line will
 * insert as many tabs and/or spaces as are needed for proper indentation.
 */

/* Local Variables: */
/* mode: c */
/* c-default-style: "bsd" */
/* c-basic-offset: 8 */
/* c-continued-statement-offset: 4 */
/* indent-tabs-mode: t */
/* End: */