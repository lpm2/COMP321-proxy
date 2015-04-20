/* 
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Xin Huang, xyh1@rice.edu 
 *     Leo Meister, lpm2@rice.edu
 * 
 */ 

#include "csapp.h"

/*
 * Function prototypes
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
	char *uri, int size);
void logging(char *logString, char *fileName);
int	parse_uri(char *uri, char *target_addr, char *path, int *port);

/* Need to write these functions
open_clientfd_ts - use the thread-safe functions getaddrinfo and getnameinfo.
Rio_readn_w
Rio_readlineb_w
Rio_writen_w
*/

bool verbose = false;

/* 
 * main - Main routine for the proxy program 
 */
int
main(int argc, char **argv)
{

	/* Check the arguments. */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		exit(0);
	}

	/* Return success. */
	return (0);
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

/**********************************
 * Wrappers for robust I/O routines
 **********************************/
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

void 
Rio_writen_w(int fd, void *usrbuf, size_t n)
{
	if (rio_writen(fd, usrbuf, n) != (int) n)
		fprintf(stdout, "Error! Failed to write into file: %s\n", 
			strerror(errno));
}

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
