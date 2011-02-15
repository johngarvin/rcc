/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001-3   The R Development Core Team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* <UTF8> the only interpretation of char is ASCII 
   <MBCS> all the char quantities should be ASCII
 */


/* based on libxml2-2.3.6:
 * nanohttp.c: minimalist HTTP GET implementation to fetch external subsets.
 *             focuses on size, streamability, reentrancy and portability
 *
 * This is clearly not a general purpose HTTP implementation
 * If you look for one, check:
 *         http://www.w3.org/Library/
 *
 * See Copyright for the status of this software.
 *
 * Daniel.Veillard@w3.org
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#ifdef Win32
#define _(String) libintl_gettext (String)
#undef gettext /* needed for graphapp */
#else
#define _(String) gettext (String)
#endif
#else /* not NLS */
#define _(String) (String)
#endif

#if !defined(Unix) || defined(HAVE_BSD_NETWORKING)

#ifdef Win32
#include <io.h>
#include <winsock.h>
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define _WINSOCKAPI_
extern void R_ProcessEvents(void);
extern void R_FlushConsole(void);
#define R_SelectEx(n,rfd,wrd,efd,tv,ih) select(n,rfd,wrd,efd,tv)
#endif

#include <R_ext/R-ftp-http.h>
#include <R_ext/PrtUtil.h>

#ifdef HAVE_STRINGS_H
   /* may be needed to define bzero in FD_ZERO (eg AIX) */
  #include <strings.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_BSD_NETWORKING
#  include <netdb.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef SUPPORT_IP6
#include <resolv.h>
#endif

/* #define DEBUG_HTTP */

#define BAD_CAST (unsigned char *)

#define xmlFree free
#define xmlMalloc malloc
#define xmlRealloc realloc
#define xmlMemStrdup strdup

static void RxmlNanoHTTPScanProxy(const char *URL);
static void *RxmlNanoHTTPMethod(const char *URL, const char *method,
				const char *input, char **contentType,
				const char *headers, const int cacheOK);

#ifdef Unix
#include <R_ext/eventloop.h>

/* modified from src/unix/sys-std.c  */
static int
setSelectMask(InputHandler *handlers, fd_set *readMask)
{
    int maxfd = -1;
    InputHandler *tmp = handlers;
    FD_ZERO(readMask);

    while(tmp) {
	if(tmp->fileDescriptor > 0) {
	    FD_SET(tmp->fileDescriptor, readMask);
	    maxfd = maxfd < tmp->fileDescriptor ? tmp->fileDescriptor : maxfd;
	}
	tmp = tmp->next;
    }

    return(maxfd);
}
#endif

/**
 * A couple of portability macros
 */
#ifndef _WINSOCKAPI_
#define closesocket(s) close(s)
#define SOCKET int
#endif

/* where strncasecmp is defined seems system-specific,
   and on Windows the cross-compiler doesn't find strings.h */
#if defined(HAVE_STRINGS_H) && !defined(Win32)
# include <strings.h>
#endif
#if defined(HAVE_DECL_STRNCASECMP) || !HAVE_DECL_STRNCASECMP
extern int strncasecmp(const char *s1, const char *s2, size_t n);
#endif
#define xmlStrncasecmp(a, b, n) strncasecmp((char *)a, (char *)b, n)

#define XML_NANO_HTTP_MAX_REDIR	10

#define XML_NANO_HTTP_CHUNK	4096

#define XML_NANO_HTTP_CLOSED	0
#define XML_NANO_HTTP_WRITE	1
#define XML_NANO_HTTP_READ	2
#define XML_NANO_HTTP_NONE	4

typedef struct RxmlNanoHTTPCtxt {
    char *protocol;	/* the protocol name */
    char *hostname;	/* the host name */
    int port;		/* the port */
    char *path;		/* the path within the URL */
    SOCKET fd;		/* the file descriptor for the socket */
    int state;		/* WRITE / READ / CLOSED */
    char *out;		/* buffer sent (zero terminated) */
    char *outptr;	/* index within the buffer sent */
    char *in;		/* the receiving buffer */
    char *content;	/* the start of the content */
    char *inptr;	/* the next byte to read from network */
    char *inrptr;	/* the next byte to give back to the client */
    int inlen;		/* len of the input buffer */
    int last;		/* return code for last operation */
    int returnValue;	/* the protocol return value */
    char *statusMsg;    /* the protocol status message */
    char *contentType;	/* the MIME type for the input */
    int contentLength;	/* the reported length */
    char *location;	/* the new URL in case of redirect */
    char *authHeader;	/* contents of {WWW,Proxy}-Authenticate header */
} RxmlNanoHTTPCtxt, *RxmlNanoHTTPCtxtPtr;

static int initialized = 0;
static char *proxy = NULL;	 /* the proxy name if any */
static int proxyPort;	/* the proxy port if any */
static char *proxyUser; /* the proxy user:pwd if any */
static unsigned int timeout = 60;/* the select() timeout in seconds */

/**
 * A portability function
 */
static int socket_errno(void)
{
#ifdef _WINSOCKAPI_
    return(WSAGetLastError());
#else
    return(errno);
#endif
}

/**
 * RxmlNanoHTTPInit:
 *
 * Initialize the HTTP protocol layer.
 * Currently it just checks for proxy informations
 */

#ifdef Win32
# include "graphapp/graphapp.h"
#endif

static void
RxmlNanoHTTPInit(void)
{
    const char *env;
#ifdef _WINSOCKAPI_
    WSADATA wsaData;
#endif

    if (initialized)
	return;

#ifdef _WINSOCKAPI_
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
	return;
#endif

    if (proxy == NULL) {
	proxyPort = 80;
	env = getenv("no_proxy");
	if (env != NULL)
	    goto done;
	env = getenv("http_proxy");
	if (env != NULL) {
	    RxmlNanoHTTPScanProxy(env);
	    goto chkuser;
	}
	env = getenv("HTTP_PROXY");
	if (env != NULL) {
	    RxmlNanoHTTPScanProxy(env);
	    goto chkuser;
	}
    chkuser:
	if((env = getenv("http_proxy_user")) != NULL) {   
	    if (proxyUser != NULL) {xmlFree(proxyUser); proxyUser = NULL;}
#ifdef Win32
	    if (strcmp(env, "ask") == 0) 
		env = askUserPass("Proxy Authentication");
#endif
	    proxyUser = xmlMemStrdup(env);
	}
    }
 done:
    initialized = 1;
}

/**
 * RxmlNanoHTTPClenup:
 *
 * Cleanup the HTTP protocol layer.
 */

void
RxmlNanoHTTPCleanup(void)
{
    if (proxy != NULL) xmlFree(proxy);
    if (proxyUser != NULL) xmlFree(proxyUser);
#ifdef _WINSOCKAPI_
    if (initialized)
	WSACleanup();
#endif
    initialized = 0;
    return;
}

/**
 * RxmlNanoHTTPTimeout:
 * @delay:  the delay in seconds
 *
 * Set the HTTP timeout, (default is 60secs).  0 means immediate
 * return, while -1 infinite.
 */

void
RxmlNanoHTTPTimeout(int delay)
{
    timeout = (unsigned int) delay;
}

/**
 * RxmlNanoHTTPScanURL:
 * @ctxt:  an HTTP context
 * @URL:  The URL used to initialize the context
 *
 * (Re)Initialize an HTTP context by parsing the URL and finding
 * the protocol host port and path it indicates.
 */

static void
RxmlNanoHTTPScanURL(RxmlNanoHTTPCtxtPtr ctxt, const char *URL)
{
    const char *cur = URL;
    char buf[4096];
    int indx = 0;
    int port = 0;

    if (ctxt->protocol != NULL) {
        xmlFree(ctxt->protocol);
	ctxt->protocol = NULL;
    }
    if (ctxt->hostname != NULL) {
        xmlFree(ctxt->hostname);
	ctxt->hostname = NULL;
    }
    if (ctxt->path != NULL) {
        xmlFree(ctxt->path);
	ctxt->path = NULL;
    }
    if (URL == NULL) return;
    buf[indx] = 0;
    while (*cur != 0) {
        if ((cur[0] == ':') && (cur[1] == '/') && (cur[2] == '/')) {
	    buf[indx] = 0;
	    ctxt->protocol = xmlMemStrdup(buf);
	    indx = 0;
            cur += 3;
	    break;
	}
	buf[indx++] = *cur++;
    }
    if (*cur == 0) return;

    buf[indx] = 0;
    while (1) {
        if (cur[0] == ':') {
	    buf[indx] = 0;
	    ctxt->hostname = xmlMemStrdup(buf);
	    indx = 0;
	    cur += 1;
	    while ((*cur >= '0') && (*cur <= '9')) {
	        port *= 10;
		port += *cur - '0';
		cur++;
	    }
	    if (port != 0) ctxt->port = port;
	    while ((cur[0] != '/') && (*cur != 0))
	        cur++;
	    break;
	}
        if ((*cur == '/') || (*cur == 0)) {
	    buf[indx] = 0;
	    ctxt->hostname = xmlMemStrdup(buf);
	    indx = 0;
	    break;
	}
	buf[indx++] = *cur++;
    }
    if (*cur == 0)
        ctxt->path = xmlMemStrdup("/");
    else {
        indx = 0;
        buf[indx] = 0;
	while (*cur != 0)
	    buf[indx++] = *cur++;
	buf[indx] = 0;
	ctxt->path = xmlMemStrdup(buf);
    }
}

/**
 * RxmlNanoHTTPScanProxy:
 * @URL:  The proxy URL used to initialize the proxy context
 *
 * (Re)Initialize the HTTP Proxy context by parsing the URL and finding
 * the protocol host port it indicates.
 * Should be like http://myproxy/ or http://myproxy:3128/
 * A NULL URL cleans up proxy informations.
 */

void
RxmlNanoHTTPScanProxy(const char *URL)
{
    const char *cur = URL;
    char buf[4096];
    int indx = 0;
    int port = 0;

    if (proxy != NULL) {
        xmlFree(proxy);
	proxy = NULL;
    }
    /*if (proxyPort != 0) {
	proxyPort = 0;
	}*/
    if (URL == NULL)
	RxmlMessage(0, _("removing HTTP proxy info"));
    else
	RxmlMessage(1, _("using HTTP proxy '%s'"), URL);
    if (URL == NULL) return;
    buf[indx] = 0;
    while (*cur != 0) {
        if ((cur[0] == ':') && (cur[1] == '/') && (cur[2] == '/')) {
	    buf[indx] = 0;
	    indx = 0;
            cur += 3;
	    break;
	}
	buf[indx++] = *cur++;
    }
    if (*cur == 0) return;

    if(strchr(cur, '@')) {
	strcpy(buf, cur);
	*(strchr(buf, '@')) = '\0';
	if(proxyUser) xmlFree(proxyUser);
	proxyUser = xmlMemStrdup(buf);
	cur += strlen(buf) + 1;
    }
    buf[indx] = 0;
    while (1) {
        if (cur[0] == ':') {
	    buf[indx] = 0;
	    proxy = xmlMemStrdup(buf);
	    indx = 0;
	    cur += 1;
	    while ((*cur >= '0') && (*cur <= '9')) {
	        port *= 10;
		port += *cur - '0';
		cur++;
	    }
	    if (port != 0) proxyPort = port;
	    while ((cur[0] != '/') && (*cur != 0))
	        cur++;
	    break;
	}
        if ((*cur == '/') || (*cur == 0)) {
	    buf[indx] = 0;
	    proxy = xmlMemStrdup(buf);
	    indx = 0;
	    break;
	}
	buf[indx++] = *cur++;
    }
}

/**
 * RxmlNanoHTTPNewCtxt:
 * @URL:  The URL used to initialize the context
 *
 * Allocate and initialize a new HTTP context.
 *
 * Returns an HTTP context or NULL in case of error.
 */

static RxmlNanoHTTPCtxtPtr
RxmlNanoHTTPNewCtxt(const char *URL)
{
    RxmlNanoHTTPCtxtPtr ret;

    ret = (RxmlNanoHTTPCtxtPtr) xmlMalloc(sizeof(RxmlNanoHTTPCtxt));
    if (ret == NULL) return(NULL);

    memset(ret, 0, sizeof(RxmlNanoHTTPCtxt));
    ret->port = 80;
    ret->returnValue = 0;
    ret->statusMsg = NULL;
    ret->contentLength = -1;
    ret->fd = -1;

    RxmlNanoHTTPScanURL(ret, URL);

    return(ret);
}

/**
 * RxmlNanoHTTPFreeCtxt:
 * @ctxt:  an HTTP context
 *
 * Frees the context after closing the connection.
 */

static void
RxmlNanoHTTPFreeCtxt(RxmlNanoHTTPCtxtPtr ctxt)
{
    if (ctxt == NULL) return;
    if (ctxt->hostname != NULL) xmlFree(ctxt->hostname);
    if (ctxt->protocol != NULL) xmlFree(ctxt->protocol);
    if (ctxt->path != NULL) xmlFree(ctxt->path);
    if (ctxt->out != NULL) xmlFree(ctxt->out);
    if (ctxt->in != NULL) xmlFree(ctxt->in);
    if (ctxt->contentType != NULL) xmlFree(ctxt->contentType);
    if (ctxt->location != NULL) xmlFree(ctxt->location);
    if (ctxt->authHeader != NULL) xmlFree(ctxt->authHeader);
    ctxt->state = XML_NANO_HTTP_NONE;
    if (ctxt->fd >= 0) closesocket(ctxt->fd);
    ctxt->fd = -1;
    xmlFree(ctxt);
}

/**
 * RxmlNanoHTTPSend:
 * @ctxt:  an HTTP context
 *
 * Send the input needed to initiate the processing on the server side
 */

static void
RxmlNanoHTTPSend(RxmlNanoHTTPCtxtPtr ctxt)
{
    if (ctxt->state & XML_NANO_HTTP_WRITE) {
        unsigned int total_sent = 0;
        while (total_sent <strlen(ctxt->outptr)) {
            unsigned int nsent = send(ctxt->fd, ctxt->outptr+total_sent,
                                      strlen(ctxt->outptr)-total_sent, 0);
            if (nsent>0)
                total_sent += nsent;
	}
        ctxt->last = total_sent;
    }
}

/**
 * RxmlNanoHTTPRecv:
 * @ctxt:  an HTTP context
 *
 * Read information coming from the HTTP connection.
 * This is a blocking call (but it blocks in select(), not read()).
 *
 * Returns the number of byte read or -1 in case of error.
 */

static int
RxmlNanoHTTPRecv(RxmlNanoHTTPCtxtPtr ctxt)
{
    fd_set rfd;
    struct timeval tv;
    double used = 0.0;


    if (ctxt->state & XML_NANO_HTTP_READ) {
	if (ctxt->in == NULL) {
	    ctxt->in = (char *) xmlMalloc(65000 * sizeof(char));
	    if (ctxt->in == NULL) {
	        ctxt->last = -1;
		return(-1);
	    }
	    ctxt->inlen = 65000;
	    ctxt->inptr = ctxt->content = ctxt->inrptr = ctxt->in;
	}
	if (ctxt->inrptr > ctxt->in + XML_NANO_HTTP_CHUNK) {
	    int delta = ctxt->inrptr - ctxt->in;
	    int len = ctxt->inptr - ctxt->inrptr;

	    memmove(ctxt->in, ctxt->inrptr, len);
	    ctxt->inrptr -= delta;
	    ctxt->content -= delta;
	    ctxt->inptr -= delta;
	}
        if ((ctxt->in + ctxt->inlen) < (ctxt->inptr + XML_NANO_HTTP_CHUNK)) {
	    int d_inptr = ctxt->inptr - ctxt->in;
	    int d_content = ctxt->content - ctxt->in;
	    int d_inrptr = ctxt->inrptr - ctxt->in;

	    ctxt->inlen *= 2;
            ctxt->in = (char *) xmlRealloc(ctxt->in, ctxt->inlen);
	    if (ctxt->in == NULL) {
	        ctxt->last = -1;
		return(-1);
	    }
            ctxt->inptr = ctxt->in + d_inptr;
            ctxt->content = ctxt->in + d_content;
            ctxt->inrptr = ctxt->in + d_inrptr;
	}

	while(1) {
	    int maxfd = 0, howmany;
#ifdef Unix
	    InputHandler *what;

	    if(R_wait_usec > 0) {
		R_PolledEvents();
		tv.tv_sec = 0;
		tv.tv_usec = R_wait_usec;
	    } else {
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
	    }
#elif defined(Win32)
	    tv.tv_sec = 0;
	    tv.tv_usec = 2e5;
	    R_ProcessEvents();
#else
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;
#endif


#ifdef Unix
	    maxfd = setSelectMask(R_InputHandlers, &rfd);
#else
	    FD_ZERO(&rfd);
#endif
	    FD_SET(ctxt->fd, &rfd);
	    if(maxfd < ctxt->fd) maxfd = ctxt->fd;

	    howmany = R_SelectEx(maxfd+1, &rfd, NULL, NULL, &tv, NULL);

	    if (howmany < 0) {
#ifdef DEBUG_HTTP
		perror("select in RxmlNanoHTTPRecv");
#endif
		return(0);
	    }
	    if (howmany == 0) {
		used += tv.tv_sec + 1e-6 * tv.tv_usec;
		if(used >= timeout) return(0);
		continue;
	    }

#ifdef Unix
	    if(!FD_ISSET(ctxt->fd, &rfd) || howmany > 1) {
		/* was one of the extras */
		what = getSelectedHandler(R_InputHandlers, &rfd);
		if(what != NULL) what->handler((void*) NULL);
		continue;
	    }
#endif

	    /* was the socket */
	    ctxt->last = recv(ctxt->fd, ctxt->inptr, XML_NANO_HTTP_CHUNK, 0);
	    if (ctxt->last > 0) {
		ctxt->inptr += ctxt->last;
		return(ctxt->last);
	    }
	    if (ctxt->last == 0) {
		return(0);
	    }
	    if (ctxt->last == -1) {
		switch (socket_errno()) {
		case EINPROGRESS:
		case EWOULDBLOCK:
#if defined(EAGAIN) && EAGAIN != EWOULDBLOCK
		case EAGAIN:
#endif
		    break;
		default:
		    return(0);
		}
	    }
	}
    }

    return(0);
}

/**
 * RxmlNanoHTTPReadLine:
 * @ctxt:  an HTTP context
 *
 * Read one line in the HTTP server output, usually for extracting
 * the HTTP protocol informations from the answer header.
 *
 * Returns a newly allocated string with a copy of the line, or NULL
 *         which indicate the end of the input.
 */

static char *
RxmlNanoHTTPReadLine(RxmlNanoHTTPCtxtPtr ctxt)
{
    char buf[4096];
    char *bp = buf;

    while (bp - buf < 4095) {
	if (ctxt->inrptr == ctxt->inptr) {
	    if (RxmlNanoHTTPRecv(ctxt) == 0) {
		if (bp == buf)
		    return(NULL);
		else
		    *bp = 0;
		return(xmlMemStrdup(buf));
	    }
	}
	*bp = *ctxt->inrptr++;
	if (*bp == '\n') {
	    *bp = 0;
	    return(xmlMemStrdup(buf));
	}
	if (*bp != '\r')
	    bp++;
    }
    buf[4095] = 0;
    return(xmlMemStrdup(buf));
}


/**
 * RxmlNanoHTTPScanAnswer:
 * @ctxt:  an HTTP context
 * @line:  an HTTP header line
 *
 * Try to extract useful informations from the server answer.
 * We currently parse and process:
 *  - The HTTP revision/ return code
 *  - The Content-Type
 *  - The Location for redirrect processing.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

static void
RxmlNanoHTTPScanAnswer(RxmlNanoHTTPCtxtPtr ctxt, const char *line)
{
    const char *cur = line;

    if (line == NULL) return;

    if (!strncmp(line, "HTTP/", 5)) {
        int version = 0;
	int ret = 0;

	cur += 5;
	while ((*cur >= '0') && (*cur <= '9')) {
	    version *= 10;
	    version += *cur - '0';
	    cur++;
	}
	if (*cur == '.') {
	    cur++;
	    if ((*cur >= '0') && (*cur <= '9')) {
		version *= 10;
		version += *cur - '0';
		cur++;
	    }
	    while ((*cur >= '0') && (*cur <= '9'))
		cur++;
	} else
	    version *= 10;
	if ((*cur != ' ') && (*cur != '\t')) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if ((*cur < '0') || (*cur > '9')) return;
	while ((*cur >= '0') && (*cur <= '9')) {
	    ret *= 10;
	    ret += *cur - '0';
	    cur++;
	}
	if ((*cur != 0) && (*cur != ' ') && (*cur != '\t')) return;
	ctxt->returnValue = ret;
	if ((*cur == ' ') || (*cur == '\t')) cur++;
	if(ctxt->statusMsg) xmlFree(ctxt->statusMsg);
	ctxt->statusMsg = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Content-Type:", 13)) {
        cur += 13;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->contentType != NULL)
	    xmlFree(ctxt->contentType);
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"ContentType:", 12)) {
        cur += 12;
	if (ctxt->contentType != NULL) return;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentType = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Content-Length:", 15)) {
        cur += 15;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	ctxt->contentLength = atoi(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Location:", 9)) {
        cur += 9;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->location != NULL)
	    xmlFree(ctxt->location);
	ctxt->location = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"WWW-Authenticate:", 17)) {
        cur += 17;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->authHeader != NULL)
	    xmlFree(ctxt->authHeader);
	ctxt->authHeader = xmlMemStrdup(cur);
    } else if (!xmlStrncasecmp(BAD_CAST line, BAD_CAST"Proxy-Authenticate:", 19)) {
        cur += 19;
	while ((*cur == ' ') || (*cur == '\t')) cur++;
	if (ctxt->authHeader != NULL)
	    xmlFree(ctxt->authHeader);
	ctxt->authHeader = xmlMemStrdup(cur);
    }
}

/**
 * RxmlNanoHTTPConnectAttempt:
 * @addr:  a socket address structure
 *
 * Attempt a connection to the given IP:port endpoint. It forces
 * non-blocking semantic on the socket, and allow 60 seconds for
 * the host to answer.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

static int
RxmlNanoHTTPConnectAttempt(struct sockaddr *addr)
{
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    fd_set wfd, rfd;
    struct timeval tv;
    int status = 0;
    double used = 0.0;

    if (s==-1) {
#ifdef DEBUG_HTTP
	perror("socket");
#endif
	return(-1);
    }

#ifdef _WINSOCKAPI_
    {
	u_long one = 1;

	status = ioctlsocket(s, FIONBIO, &one) == SOCKET_ERROR ? -1 : 0;
    }
#else /* _WINSOCKAPI_ */
#if defined(VMS)
    {
	int enable = 1;
	status = ioctl(s, FIONBIO, &enable);
    }
#else /* VMS */
#ifdef HAVE_FCNTL
    if ((status = fcntl(s, F_GETFL, 0)) != -1) {
#ifdef O_NONBLOCK
	status |= O_NONBLOCK;
#else /* O_NONBLOCK */
#ifdef F_NDELAY
	status |= F_NDELAY;
#endif /* F_NDELAY */
#endif /* !O_NONBLOCK */
	status = fcntl(s, F_SETFL, status);
    }
#endif
    if (status < 0) {
#ifdef DEBUG_HTTP
	perror("nonblocking");
#endif
	closesocket(s);
	return(-1);
    }
#endif /* !VMS */
#endif /* !_WINSOCKAPI_ */


    if ((connect(s, addr, sizeof(*addr))==-1)) {

	switch (socket_errno()) {
	case EINPROGRESS:
	case EWOULDBLOCK:
	    break;
	default:
	    perror("connect");
	    closesocket(s);
	    return(-1);
	}
    }

    while(1) {
	int maxfd = 0;
#ifdef Unix
	InputHandler *what;

	if(R_wait_usec > 0) {
	    R_PolledEvents();
	    tv.tv_sec = 0;
	    tv.tv_usec = R_wait_usec;
	} else {
	    tv.tv_sec = timeout;
	    tv.tv_usec = 0;
	}
#elif defined(Win32)
	tv.tv_sec = 0;
	tv.tv_usec = 2e5;
	R_ProcessEvents();
#else
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
#endif


#ifdef Unix
	maxfd = setSelectMask(R_InputHandlers, &rfd);
#else
	FD_ZERO(&rfd);
#endif
	FD_ZERO(&wfd);
	FD_SET(s, &wfd);
	if(maxfd < s) maxfd = s;

	switch(R_SelectEx(maxfd+1, &rfd, &wfd, NULL, &tv, NULL))
	{
	case 0:
	    /* Time out */
	    used += tv.tv_sec + 1e-6 * tv.tv_usec;
	    if(used < timeout) continue;
	    closesocket(s);
	    return(-1);
	case -1:
	    /* Ermm.. ?? */
#ifdef DEBUG_HTTP
	    perror("select");
#endif
	    closesocket(s);
	    return(-1);
	}

	if ( FD_ISSET(s, &wfd) ) {
	    SOCKLEN_T len;
	    len = sizeof(status);
	    if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&status, &len) < 0){
		/* Solaris error code */
		return (-1);
	    }
	    if ( status ) {
		closesocket(s);
		errno = status;
		return (-1);
	    } else return(s);
#ifdef Unix
	} else { /* some other handler needed */
	    what = getSelectedHandler(R_InputHandlers, &rfd);
	    if(what != NULL) what->handler((void*) NULL);
	    continue;
#endif
	}
    }
    /* not reached */
    return(-1);
}

/**
 * RxmlNanoHTTPConnectHost:
 * @host:  the host name
 * @port:  the port number
 *
 * Attempt a connection to the given host:port endpoint. It tries
 * the multiple IP provided by the DNS if available.
 *
 * Returns -1 in case of failure, the file descriptor number otherwise
 */

static int
RxmlNanoHTTPConnectHost(const char *host, int port)
{
    struct hostent *h;
    struct sockaddr *addr;
    struct in_addr ia;
    struct sockaddr_in sockin;
#ifdef SUPPORT_IP6
    struct in6_addr ia6;
    struct sockaddr_in6 sockin6;
#endif
    int i;
    int s;

#if defined(SUPPORT_IP6) && defined(RES_USE_INET6)
    if (!(_res.options & RES_INIT))
	res_init();
    _res.options |= RES_USE_INET6;
#endif
    h=gethostbyname(host);
    if (h==NULL)
    {
	RxmlMessage(2, _("unable to resolve '%s'."), host);
	return(-1);
    }

    for(i=0; h->h_addr_list[i]; i++)
    {
	if (h->h_addrtype == AF_INET) {
	    /* A records (IPv4) */
	    memcpy(&ia, h->h_addr_list[i], h->h_length);
	    sockin.sin_family = h->h_addrtype;
	    sockin.sin_addr   = ia;
	    sockin.sin_port   = htons(port);
	    addr = (struct sockaddr *)&sockin;
#ifdef SUPPORT_IP6
	} else if (h->h_addrtype == AF_INET6) {
	    /* AAAA records (IPv6) */
	    memcpy(&ia6, h->h_addr_list[i], h->h_length);
	    sockin6.sin_family = h->h_addrtype;
	    sockin6.sin_addr   = ia6;
	    sockin6.sin_port   = htons(port);
	    addr = (struct sockaddr *)&sockin6;
#endif
	} else
	    break; /* for */

	s = RxmlNanoHTTPConnectAttempt(addr);
	if (s != -1) {
	    RxmlMessage(1, _("connected to '%s' on port %d."), host, port);
	    return(s);
	}
    }

    RxmlMessage(2, _("unable to connect to '%s' on port %d."), host, port);
    return(-1);
}


/**
 * RxmlNanoHTTPOpen:
 * @URL:  The URL to load
 * @contentType:  if available the Content-Type information will be
 *                returned at that location
 *
 * This function try to open a connection to the indicated resource
 * via HTTP GET.
 *
 * Returns NULL in case of failure, otherwise a request handler.
 *     The contentType, if provided must be freed by the caller
 */

void*
RxmlNanoHTTPOpen(const char *URL, char **contentType, int cacheOK)
{
    if (contentType != NULL) *contentType = NULL;
    return RxmlNanoHTTPMethod(URL, NULL, NULL, contentType, NULL, cacheOK);
}

/**
 * RxmlNanoHTTPRead:
 * @ctx:  the HTTP context
 * @dest:  a buffer
 * @len:  the buffer length
 *
 * This function tries to read @len bytes from the existing HTTP connection
 * and saves them in @dest. This is a blocking call.
 *
 * Returns the number of byte read. 0 is an indication of an end of connection.
 *         -1 indicates a parameter error.
 */
int
RxmlNanoHTTPRead(void *ctx, void *dest, int len)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;

    if (ctx == NULL) return(-1);
    if (dest == NULL) return(-1);
    if (len <= 0) return(0);

    while (ctxt->inptr - ctxt->inrptr < len) {
        if (RxmlNanoHTTPRecv(ctxt) == 0) break;
    }
    if (ctxt->inptr - ctxt->inrptr < len)
        len = ctxt->inptr - ctxt->inrptr;
    memcpy(dest, ctxt->inrptr, len);
    ctxt->inrptr += len;
    return(len);
}

static void base64_encode(char *proxyUser, char *out)
{
    /* Conversion table.  */
    static char tbl[64] = {
	'A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X',
	'Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n',
	'o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3',
	'4','5','6','7','8','9','+','/'
    };
    char *s = proxyUser;
    int i, length;
    unsigned char *p = (unsigned char *)out;

    length = strlen(s);
    for (i = 0; i < length; i += 3) {
	*p++ = tbl[s[i] >> 2];
	*p++ = tbl[((s[i] & 3) << 4) + (s[i+1] >> 4)];
	*p++ = tbl[((s[i+1] & 0xf) << 2) + (s[i+2] >> 6)];
	*p++ = tbl[s[i+2] & 0x3f];
    }
    /* Pad the result if necessary...  */
    if (i == length + 1) *(p - 1) = '=';
    else if (i == length + 2) *(p - 1) = *(p - 2) = '=';
    /* ...and zero-terminate it.  */
    *p = '\0';
}

/**
 * RxmlNanoHTTPClose:
 * @ctx:  the HTTP context
 *
 * This function closes an HTTP context, it ends up the connection and
 * free all data related to it.
 */
void
RxmlNanoHTTPClose(void *ctx)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;

    if (ctx == NULL) return;

    RxmlNanoHTTPFreeCtxt(ctxt);
}

/**
 * RxmlNanoHTTPMethod:
 * @URL:  The URL to load
 * @method:  the HTTP method to use
 * @input:  the input string if any
 * @contentType:  the Content-Type information IN and OUT
 * @headers:  the extra headers
 *
 * This function try to open a connection to the indicated resource
 * via HTTP using the given @method, adding the given extra headers
 * and the input buffer for the request content.
 *
 * Returns NULL in case of failure, otherwise a request handler.
 *     The contentType, if provided must be freed by the caller
 */

void*
RxmlNanoHTTPMethod(const char *URL, const char *method, const char *input,
                  char **contentType, const char *headers, const int cacheOK)
{
    RxmlNanoHTTPCtxtPtr ctxt;
    char *bp, *p;
    int blen, ilen, ret;
    int head;
    int nbRedirects = 0;
#ifdef Win32
    int nAuthenticate = 0;
#endif
    char *redirURL = NULL;
    char buf[1000];

    if (URL == NULL) return(NULL);
    if (method == NULL) method = "GET";
    RxmlNanoHTTPInit();

 retry:
    if (redirURL == NULL)
	ctxt = RxmlNanoHTTPNewCtxt(URL);
    else {
	ctxt = RxmlNanoHTTPNewCtxt(redirURL);
	xmlFree(redirURL);
	redirURL = NULL;
    }

    if ((ctxt->protocol == NULL) || (strcmp(ctxt->protocol, "http"))) {
        RxmlNanoHTTPFreeCtxt(ctxt);
	if (redirURL != NULL) xmlFree(redirURL);
        return(NULL);
    }
    if (ctxt->hostname == NULL) {
        RxmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    if (proxy) {
	blen = strlen(ctxt->hostname) * 2 + 16;
	ret = RxmlNanoHTTPConnectHost(proxy, proxyPort);
    }
    else {
	blen = strlen(ctxt->hostname);
	ret = RxmlNanoHTTPConnectHost(ctxt->hostname, ctxt->port);
    }
    if (ret < 0) {
        RxmlNanoHTTPFreeCtxt(ctxt);
        return(NULL);
    }
    ctxt->fd = ret;

    if (input != NULL) {
	ilen = strlen(input);
	blen += ilen + 32;
    }
    else
	ilen = 0;
    if (!cacheOK)
	blen += 20;
    if (headers != NULL)
	blen += strlen(headers);
    if (contentType && *contentType)
	blen += strlen(*contentType) + 16;
    if (proxy && proxyUser) {
	base64_encode(proxyUser,  buf);
	blen +=strlen(buf) + 50;
    }
    blen += strlen(method) + strlen(ctxt->path) + 23;
    bp = xmlMalloc(blen);
    if (proxy) {
	if (ctxt->port != 80) {
	    sprintf(bp, "%s http://%s:%d%s", method, ctxt->hostname,
		    ctxt->port, ctxt->path);
	}
	else
	    sprintf(bp, "%s http://%s%s", method, ctxt->hostname, ctxt->path);
    }
    else
	sprintf(bp, "%s %s", method, ctxt->path);
    p = bp + strlen(bp);
    sprintf(p, " HTTP/1.0\r\nHost: %s\r\n", ctxt->hostname);
    p += strlen(p);
    if(!cacheOK) {
	sprintf(p, "Pragma: no-cache\r\n");
	p += strlen(p);
    }
    if(proxy && proxyUser) {
	sprintf(p, "Proxy-Authorization: Basic %s\r\n", buf);
	p += strlen(p);
    }
    if (contentType != NULL && *contentType) {
	sprintf(p, "Content-Type: %s\r\n", *contentType);
	p += strlen(p);
    }
    if (headers != NULL) {
	strcpy(p, headers);
	p += strlen(p);
    }
    if (input != NULL)
	sprintf(p, "Content-Length: %d\r\n\r\n%s", ilen, input);
    else
	strcpy(p, "\r\n");
    RxmlMessage(0, "-> %s%s", proxy? "(Proxy) " : "", bp);
    if ((blen -= strlen(bp)+1) < 0)
	RxmlMessage(0, "ERROR: overflowed buffer by %d bytes\n", -blen);
    ctxt->outptr = ctxt->out = bp;
    ctxt->state = XML_NANO_HTTP_WRITE;
    RxmlNanoHTTPSend(ctxt);
    ctxt->state = XML_NANO_HTTP_READ;
    head = 1;

    while ((p = RxmlNanoHTTPReadLine(ctxt)) != NULL) {
        if (head && (*p == 0)) {
	    head = 0;
	    ctxt->content = ctxt->inrptr;
	    xmlFree(p);
	    break;
	}
	RxmlNanoHTTPScanAnswer(ctxt, p);

	RxmlMessage(0, "<- %s\n", p);
        xmlFree(p);
    }

#ifdef Win32
    /* Prompt for username/password again if status was proxy
       authentication failure */
    if(proxy && !nAuthenticate && ctxt->returnValue == 407) {
	char *env;
	REprintf("%s\n%s\n", "Proxy authentication failed:",
		"\tplease re-enter the credentials or hit Cancel");
	R_FlushConsole(); R_ProcessEvents();
	env = askUserPass("Proxy Authentication");
	if(strlen(env)) {
	    if (proxyUser != NULL) {xmlFree(proxyUser); proxyUser = NULL;}
	    proxyUser = xmlMemStrdup(env);
	    nAuthenticate++;
	    goto retry;
	}
    }
#endif

    if ((ctxt->location != NULL) && (ctxt->returnValue >= 300) &&
        (ctxt->returnValue < 400)) {
	RxmlMessage(1, _("redirect to: '%s'"), ctxt->location);
	while (RxmlNanoHTTPRecv(ctxt)) ;
        if (nbRedirects < XML_NANO_HTTP_MAX_REDIR) {
	    nbRedirects++;
	    redirURL = xmlMemStrdup(ctxt->location);
	    RxmlNanoHTTPFreeCtxt(ctxt);
	    goto retry;
	}
	RxmlNanoHTTPFreeCtxt(ctxt);
	RxmlMessage(2, _("too many redirects, aborting ..."));
	return(NULL);

    }

    if (contentType != NULL) {
	if (ctxt->contentType != NULL)
	    *contentType = xmlMemStrdup(ctxt->contentType);
	else
	    *contentType = NULL;
    }

    if (ctxt->contentType != NULL)
	RxmlMessage(1, "Code %d, content-type '%s'",
		    ctxt->returnValue, ctxt->contentType);
    else
	RxmlMessage(1, "Code %d, no content-type",
		    ctxt->returnValue);

    return((void *) ctxt);
}



/**
 * RxmlNanoHTTPReturnCode:
 * @ctx:  the HTTP context
 *
 * Returns the HTTP return code for the request.
 */
int
RxmlNanoHTTPReturnCode(void *ctx)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;

    if (ctxt == NULL) return(-1);

    return(ctxt->returnValue);
}

char *
RxmlNanoHTTPStatusMsg(void *ctx)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;
    return(ctxt->statusMsg);
}

int
RxmlNanoHTTPContentLength(void *ctx)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;

    if (ctxt == NULL) return(-1);

    return(ctxt->contentLength);
}

char *
RxmlNanoHTTPContentType(void *ctx)
{
    RxmlNanoHTTPCtxtPtr ctxt = (RxmlNanoHTTPCtxtPtr) ctx;
    return(ctxt->contentType);
}

#endif /* !Unix or BSD_NETWORKING */
