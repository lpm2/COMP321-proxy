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

/*
 * Function prototypes
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr,
    char *uri, int size);
void logging(char *logString, char *fileName);
int parse_uri(char *uri, char *target_addr, char *path, int *port);
int Rio_readn_w();
int Rio_readlineb_w();
int Rio_writen_w();

/* Need to write these files
 * open_clientfd_ts - use the thread-safe functions getaddrinfo and getnameinfo.
 * Rio_readn_w
 * Rio_readlineb_w
 * Rio_writen_w
 */

bool verbose = false;

/* 
 * main - Main routine for the proxy program 
 */
int
main(int argc, char **argv)
{
	int listenfd, connfd, port, error;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	char haddrp[INET_ADDRSTRLEN];
	char host_name[NI_MAXHOST];
	
	if (argc != 2) {
        	fprintf(stderr, "usage: %s <port>\n", argv[0]);
        	exit(0);
	}
	
	port = atoi(argv[1]);
	listenfd = Open_listenfd(port);
	
	while (1) {
    		
    		clientlen = sizeof(clientaddr);
    		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

    		/* determine the domain name and IP address of the client */
		error = getnameinfo((struct sockaddr *)&clientaddr, sizeof(clientaddr), host_name, sizeof(host_name), NULL,0, 0);
	
		if (error != 0) {
			fprintf(stderr, "ERROR: %s\n", gai_strerror(error));
			Close(connfd);
			continue;
		}
		inet_ntop(AF_INET, &clientaddr.sin_addr, haddrp, INET_ADDRSTRLEN);
		printf("server connected to %s (%s)\n", host_name, haddrp);

    		Close(connfd);
    	}
    	exit(0);

	/* Check the arguments. */
	/*if (argc != 2) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		exit(0);
	}*/

	/* Return success. */
	//return (0);
}


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
		hostend = hostbegin + strlen(hostbegin);
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
