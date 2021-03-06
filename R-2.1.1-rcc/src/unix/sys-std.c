/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2004  Robert Gentleman, Ross Ihaka
 *                            and the R Development Core Team
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

/* <UTF8> char here is mainly handled as a whole string.
   Does need readline to support it.
   Appending \n\0 is OK in UTF-8, not general MBCS.
   Removal of \r is OK on UTF-8.
   ? use of isspace OK?
 */


/* See system.txt for a description of functions */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_STRINGS_H
   /* may be needed to define bzero in FD_ZERO (eg AIX) */
  #include <strings.h>
#endif

#include "Defn.h"
#include "Fileio.h"
#include "Rdevices.h"		/* for KillAllDevices */
#include "Runix.h"
#include "Startup.h"
#include <R_ext/Riconv.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>		/* for unlink */
#endif

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>		/* for struct timeval */
#endif

extern SA_TYPE SaveAction;
extern Rboolean UsingReadline;

/*
 *  1) FATAL MESSAGES AT STARTUP
 */

void Rstd_Suicide(char *s)
{
    REprintf("Fatal error: %s\n", s); 
    /* Might be called before translation is running */
    R_CleanUp(SA_SUICIDE, 2, 0);
}

/*
 *  2. CONSOLE I/O
 */



	/*--- I/O Support Code ---*/

	/* These routines provide hooks for supporting console I/O.
	 * Under raw Unix these routines simply provide a
	 * connection to the stdio library.
	 * Under a Motif interface the routines would be
	 * considerably more complex.
	 */

#define __SYSTEM__
#include <R_ext/eventloop.h>
#undef __SYSTEM__

/*
  The following provides a version of select() that catches interrupts
  and handles them using the supplied interrupt handler or the default
  one if NULL is supplied.  The interrupt handler must exit using a
  longjmp.  If the supplied timout value os zero, select is called
  without setting up an error handler since it should return
  immediately.
 */

static SIGJMP_BUF seljmpbuf;

static RETSIGTYPE (*oldSigintHandler)(int) = SIG_DFL;

typedef void (*sel_intr_handler_t)(void);

static RETSIGTYPE handleSelectInterrupt(int dummy)
{
    signal(SIGINT, oldSigintHandler);
    SIGLONGJMP(seljmpbuf, 1);
}

int R_SelectEx(int  n,  fd_set  *readfds,  fd_set  *writefds,
	       fd_set *exceptfds, struct timeval *timeout,
	       void (*intr)(void))
{
    if (timeout != NULL && timeout->tv_sec == 0 && timeout->tv_usec == 0)
	return select(n, readfds, writefds, exceptfds, timeout);
    else {
	volatile sel_intr_handler_t myintr = intr != NULL ? intr : onintr;
	if (SIGSETJMP(seljmpbuf, 1)) {
	    myintr();
	    error(_("interrupt handler must not return"));
	    return 0; /* not reached */
	}
	else {
	    int val;

	    /* install a temporary signal handler for breaking out of
	       a blocking select */
	    oldSigintHandler = signal(SIGINT, handleSelectInterrupt);

	    /* once the new sinal handler is in place we need to check
	       for and handle any pending interrupt registered by the
	       standard handler. */
	    if (R_interrupts_pending)
		myintr();

	    /* now do the (possibly blocking) select, restore the
	       signal handler, and return the result of the select. */
	    val = select(n, readfds, writefds, exceptfds, timeout);
	    signal(SIGINT, oldSigintHandler);
	    return val;
	}
    }
}


/*
   This object is used for the standard input and its file descriptor
   value is reset by setSelectMask() each time to ensure that it points
   to the correct value of stdin.
 */
static InputHandler BasicInputHandler = {StdinActivity, -1, NULL};

/*
   This can be reset by the initialization routines which
   can ignore stdin, etc..
*/
InputHandler *R_InputHandlers = &BasicInputHandler;

/*
  Initialize the input source handlers used to check for input on the
  different file descriptors.
 */
InputHandler * initStdinHandler(void)
{
    InputHandler *inputs;

    inputs = addInputHandler(R_InputHandlers, fileno(stdin), NULL,
			     StdinActivity);
    /* Defer the X11 registration until it is loaded and actually used. */

    return(inputs);
}

/*
  Creates and registers a new InputHandler with the linked list `handlers'.
  This sets the global variable InputHandlers if it is not already set.
  In the standard interactive case, this will have been set to be the
  BasicInputHandler object.
 */
InputHandler *
addInputHandler(InputHandler *handlers, int fd, InputHandlerProc handler,
		int activity)
{
    InputHandler *input, *tmp;
    input = (InputHandler*) calloc(1, sizeof(InputHandler));

    input->activity = activity;
    input->fileDescriptor = fd;
    input->handler = handler;

    tmp = handlers;

    if(handlers == NULL) {
	R_InputHandlers = input;
	return(input);
    }

    /* Go to the end of the list to append the new one.  */
    while(tmp->next != NULL) {
	tmp = tmp->next;
    }
    tmp->next = input;

    return(handlers);
}

/*
  Removes the specified handler from the linked list.

  See getInputHandler() for first locating the target handler instance.
 */
int
removeInputHandler(InputHandler **handlers, InputHandler *it)
{
    InputHandler *tmp;

    /* If the handler is the first one in the list, move the list to point
       to the second element. That's why we use the address of the first
       element as the first argument.
    */

    if (it == NULL) return(0);

    if(*handlers == it) {
	*handlers = (*handlers)->next;
	return(1);
    }

    tmp = *handlers;

    while(tmp) {
	if(tmp->next == it) {
	    tmp->next = it->next;
	    return(1);
	}
	tmp = tmp->next;
    }

    return(0);
}


InputHandler *
getInputHandler(InputHandler *handlers, int fd)
{
    InputHandler *tmp;
    tmp = handlers;

    while(tmp != NULL) {
	if(tmp->fileDescriptor == fd)
	    return(tmp);
	tmp = tmp->next;
    }

    return(tmp);
}

/*
 Arrange to wait until there is some activity or input pending
 on one of the file descriptors to which we are listening.

 We could make the file descriptor mask persistent across
 calls and change it only when a listener is added or deleted.
 Later.


 This replaces the previous version which looked only on stdin and the X11
 device connection.  This allows more than one X11 device to be open on a different
 connection. Also, it allows connections a la S4 to be developed on top of this
 mechanism. The return type of this routine has changed.
*/

/* A package can enable polled event handling by making R_PolledEvents
   point to a non-dummy routine and setting R_wait_usec to a suitable
   timeout value (e.g. 100000) */

static void nop(void){}

void (* R_PolledEvents)(void) = nop;

int R_wait_usec = 0; /* 0 means no timeout */

static int setSelectMask(InputHandler *, fd_set *);


fd_set *R_checkActivityEx(int usec, int ignore_stdin, void (*intr)(void))
{
    int maxfd;
    struct timeval tv;
    static fd_set readMask;

    if (R_interrupts_pending) {
	if (intr != NULL) intr();
	else onintr();
    }

    tv.tv_sec = 0;
    tv.tv_usec = usec;
    maxfd = setSelectMask(R_InputHandlers, &readMask);
    if (ignore_stdin)
	FD_CLR(fileno(stdin), &readMask);
    if (R_SelectEx(maxfd+1, &readMask, NULL, NULL,
		   (usec >= 0) ? &tv : NULL, intr))
	return(&readMask);
    else
	return(NULL);
}

fd_set *R_checkActivity(int usec, int ignore_stdin)
{
    return R_checkActivityEx(usec, ignore_stdin, NULL);
}

/*
  Create the mask representing the file descriptors select() should
  monitor and return the maximum of these file descriptors so that
  it can be passed directly to select().

  If the first element of the handlers is the standard input handler
  then we set its file descriptor to the current value of stdin - its
  file descriptor.
 */

static int
setSelectMask(InputHandler *handlers, fd_set *readMask)
{
    int maxfd = -1;
    InputHandler *tmp = handlers;
    FD_ZERO(readMask);

    /* If we are dealing with BasicInputHandler always put stdin */
    if(handlers == &BasicInputHandler)
	handlers->fileDescriptor = fileno(stdin);

    while(tmp) {
	FD_SET(tmp->fileDescriptor, readMask);
	maxfd = maxfd < tmp->fileDescriptor ? tmp->fileDescriptor : maxfd;
	tmp = tmp->next;
    }

    return(maxfd);
}

void R_runHandlers(InputHandler *handlers, fd_set *readMask)
{
    InputHandler *tmp = handlers;

    if (readMask == NULL)
	R_PolledEvents();
    else
	while(tmp) {
	    if(FD_ISSET(tmp->fileDescriptor, readMask)
	       && tmp->handler != NULL)
		tmp->handler((void*) NULL);
	    tmp = tmp->next;
	}
}

/* The following routine is still used by the internet routines, but
 * it should eventually go away. */

InputHandler *
getSelectedHandler(InputHandler *handlers, fd_set *readMask)
{
    InputHandler *tmp = handlers;

    /*
      Temporarily skip the first one if a) there is another one, and
      b) this is the BasicInputHandler.
    */
    if(handlers == &BasicInputHandler && handlers->next)
	tmp = handlers->next;

    while(tmp) {
	if(FD_ISSET(tmp->fileDescriptor, readMask))
	    return(tmp);
	tmp = tmp->next;
    }
    /* Now deal with the first one. */
    if(FD_ISSET(handlers->fileDescriptor, readMask))
	return(handlers);

    return((InputHandler*) NULL);
}



#ifdef HAVE_LIBREADLINE

# ifdef HAVE_READLINE_READLINE_H
#  include <readline/readline.h>
/* For compatibility with pre-readline4.2 systems: */
#  if !defined (_RL_FUNCTION_TYPEDEF)
typedef void rl_vcpfunc_t (char *);
#  endif /* _RL_FUNCTION_TYPEDEF */
# else
typedef void rl_vcpfunc_t (char *);
extern void rl_callback_handler_install(const char *, rl_vcpfunc_t *);
extern void rl_callback_handler_remove(void);
extern void rl_callback_read_char(void);
extern char *tilde_expand (const char *);
# endif

char *R_ExpandFileName_readline(char *s, char *buff)
{
    char *s2 = tilde_expand(s);

    strncpy(buff, s2, PATH_MAX);
    if(strlen(s2) >= PATH_MAX) buff[PATH_MAX-1] = '\0';
    free(s2);
    return buff;
}


# ifdef HAVE_READLINE_HISTORY_H
#  include <readline/history.h>
# endif


/* callback for rl_callback_read_char */


/*

There has been a general problem with asynchonous calls to browser and
anything that uses the standard console reading facilties asynchronously
(e.g. scan(), parse(), menu()).  The basic problem is as follows.  We
are in the usual input loop awaiting characters typed by the user.  Then
asynchronously, we enter the browser due to a callback that is invoked
from the background event loop that is active while waiting for the user
input.  At this point, we essentially are starting a new readline
session and it is important that we restore the old one when we complete
the browse-related one. But unfortunately, we are using global variables
and restoring it is not currently being done.
So this is an attempt to a) remove the global variables (which will
help with threading), and b) ensure that the relevant readline handlers
are restored when an asynchronous reader completes its task.

Cleaning up after errors is still an issue that needs investigation
and whether the current setup does the correct thing.
Related to this is whether nested calls (e.g. within a browser, we
do other calls to browser() or scan and whether these i)
accumulate on our readline stack, and ii) are unwound correctly.
If they don't accumulate, we need only keep  function pointers on
this stack. 10 seems safe for most use and is an improvement
over the abort's that we were getting due to the lack of
a readline handler being registered.
DTL.
*/

typedef struct _R_ReadlineData R_ReadlineData;

struct _R_ReadlineData {

 int readline_gotaline;
 int readline_addtohistory;
 int readline_len;
 int readline_eof;
 unsigned char *readline_buf;
 R_ReadlineData *prev;

};

R_ReadlineData *rl_top = NULL;

#define MAX_READLINE_NESTING 10

static struct {
  int current;
  int max;
  rl_vcpfunc_t *fun[MAX_READLINE_NESTING];
} ReadlineStack = {-1, MAX_READLINE_NESTING - 1};


/*
  Registers the specified routine and prompt with readline
  and keeps a record of it on the top of the R readline stack.
 */
void
pushReadline(char *prompt, rl_vcpfunc_t f)
{
   if(ReadlineStack.current >= ReadlineStack.max) {
     warning(_("An unusual circumstance has arisen in the nesting of readline input. Please report using bug.report()"));
   } else
     ReadlineStack.fun[++ReadlineStack.current] = f;

   rl_callback_handler_install(prompt, f);
   /* flush stdout in case readline wrote the prompt, but didn't flush
      stdout to make it visible. (needed for Apple's rl in OS X 10.4-pre) */
   fflush(stdout);
}

/*
  Unregister the current readline handler and pop it from R's readline
  stack, followed by re-registering the previous one.
*/
void
popReadline()
{
  if(ReadlineStack.current > -1) {
     rl_callback_handler_remove();
     ReadlineStack.fun[ReadlineStack.current--] = NULL;
     if(ReadlineStack.current > -1 && ReadlineStack.fun[ReadlineStack.current])
        rl_callback_handler_install("", ReadlineStack.fun[ReadlineStack.current]);
  }
}

static void readline_handler(char *line)
{
    int l;

    popReadline();

    if ((rl_top->readline_eof = !line)) /* Yes, I don't mean ==...*/
	return;
    if (line[0]) {
# ifdef HAVE_READLINE_HISTORY_H
	if (strlen(line) && rl_top->readline_addtohistory)
	    add_history(line);
# endif
	l = (((rl_top->readline_len-2) > strlen(line))?
	     strlen(line): (rl_top->readline_len-2));
	strncpy((char *)rl_top->readline_buf, line, l);
	rl_top->readline_buf[l] = '\n';
	rl_top->readline_buf[l+1] = '\0';
    }
    else {
	rl_top->readline_buf[0] = '\n';
	rl_top->readline_buf[1] = '\0';
    }
    rl_top->readline_gotaline = 1;
}

/*
 An extension or override for the standard interrupt handler (Ctrl-C)
 that pops the readline stack and then calls the regular/standard
 interrupt handler. This could be done in a nicer and more general way.
 It may be necessary for embedding, etc. although it may not be an issue
 there (as the host application will presumably handle signals).
 by allowing us to add C routines to be called
 at the conclusion of the context. At the moment there is only one such routine
 allowed, and so we would have to chain them. This just leads to a different set of
 maintenance problems when we rely on the authors of individual routines to
 not break the chain!
 Note that the readline stack is not popped when a SIGUSR1 or SIGUSR2 occurs
 during the select. But of course, we are about to terminate the R session at
 that point so it shouldn't be relevant except in the embedded case. But
 the host application will probably not let things get that far and trap the
 signals itself.
*/
static void
handleInterrupt(void)
{
    popReadline();
    onintr();
}
#else
static void
handleInterrupt(void)
{
    onintr();
}
#endif /* HAVE_LIBREADLINE */

/* Fill a text buffer from stdin or with user typed console input. */
static void *cd = NULL;

int Rstd_ReadConsole(char *prompt, unsigned char *buf, int len,
		     int addtohistory)
{
    if(!R_Interactive) {
	int ll, err = 0;
	if (!R_Slave)
	    fputs(prompt, stdout);
	if (fgets((char *)buf, len, stdin) == NULL)
	    return 0;
	ll = strlen((char *)buf);
	/* remove CR in CRLF ending */
	if (ll >= 2 && buf[ll - 1] == '\n' && buf[ll - 2] == '\r') {
	    buf[ll - 2] = '\n';
	    buf[--ll] = '\0';
	}
	/* translate if necessary */
	if(strlen(R_StdinEnc) && strcmp(R_StdinEnc, "native.enc")) {
#ifdef HAVE_DECL_ICONV
	    size_t res, inb = strlen((char *)buf), onb = len;
	    char obuf[1001];
	    char *ib = (char *)buf, *ob = obuf;
	    if(!cd) {
		cd = Riconv_open("", R_StdinEnc);
		if(!cd) error(_("encoding '%s' is not recognised"), R_StdinEnc);
	    }
	    res = Riconv(cd, &ib, &inb, &ob, &onb);
	    *ob = '\0';
	    err = res == (size_t)(-1);
	    /* errors lead to part of the input line being ignored */
	    if(err) fputs(_("<ERROR: invalid input in encoding> "), stdout);
	    strncpy((char *)buf, obuf, len);
#else
	    if(!cd) {
		warning(_("re-encoding is not available on this system"));
		cd = (void *)1;
	    }
#endif
    	}
/* according to system.txt, should be terminated in \n, so check this
   at eof and error */
	if ((err || feof(stdin)) 
	    && (ll == 0 || buf[ll - 1] != '\n') && ll < len) {
	    buf[ll++] = '\n'; buf[ll] = '\0';
	}
	if (!R_Slave)
	    fputs((char *)buf, stdout);
	return 1;
    }
    else {
#ifdef HAVE_LIBREADLINE
        R_ReadlineData rl_data;
	if (UsingReadline) {
	    rl_data.readline_gotaline = 0;
	    rl_data.readline_buf = buf;
	    rl_data.readline_addtohistory = addtohistory;
	    rl_data.readline_len = len;
	    rl_data.readline_eof = 0;
	    rl_data.prev = rl_top;
	    rl_top = &rl_data;
	    pushReadline(prompt, readline_handler);
	}
	else
#endif /* HAVE_LIBREADLINE */
	{
	    fputs(prompt, stdout);
	    fflush(stdout);
	}

	if(R_InputHandlers == NULL)
	    initStdinHandler();

	for (;;) {
	    fd_set *what;

	    what = R_checkActivityEx(R_wait_usec ? R_wait_usec : -1, 0,
				     handleInterrupt);
	    /* This is slightly clumsy. We have advertised the
	     * convention that R_wait_usec == 0 means "wait forever",
	     * but we also need to enable R_checkActivity to return
	     * immediately. */

	    R_runHandlers(R_InputHandlers, what);
	    if (what == NULL)
		continue;
	    if (FD_ISSET(fileno(stdin), what)) {
		/* We could make this a regular handler, but we need
		 * to pass additional arguments. */
#ifdef HAVE_LIBREADLINE
		if (UsingReadline) {
		    rl_callback_read_char();
		    if(rl_data.readline_eof || rl_data.readline_gotaline) {
			rl_top = rl_data.prev;
			return(rl_data.readline_eof ? 0 : 1);
		    }
		}
		else
#endif /* HAVE_LIBREADLINE */
		{
		    if(fgets((char *)buf, len, stdin) == NULL)
			return 0;
		    else
			return 1;
		}
	    }
	}
    }
}

	/* Write a text buffer to the console. */
	/* All system output is filtered through this routine. */

void Rstd_WriteConsole(char *buf, int len)
{
    printf("%s", buf);
}


	/* Indicate that input is coming from the console */

void Rstd_ResetConsole()
{
}


	/* Stdio support to ensure the console file buffer is flushed */

void Rstd_FlushConsole()
{
    /* fflush(stdin);  really work on Solaris on pipes */
}

	/* Reset stdin if the user types EOF on the console. */

void Rstd_ClearerrConsole()
{
    clearerr(stdin);
}

/*
 *  3) ACTIONS DURING (LONG) COMPUTATIONS
 */

void Rstd_Busy(int which)
{
}

/*
 *  4) INITIALIZATION AND TERMINATION ACTIONS
 */

/*
   R_CleanUp is invoked at the end of the session to give the user the
   option of saving their data.
   If ask == SA_SAVEASK the user should be asked if possible (and this
   option should not occur in non-interactive use).
   If ask = SA_SAVE or SA_NOSAVE the decision is known.
   If ask = SA_DEFAULT use the SaveAction set at startup.
   In all these cases run .Last() unless quitting is cancelled.
   If ask = SA_SUICIDE, no save, no .Last, possibly other things.
 */


void Rstd_CleanUp(SA_TYPE saveact, int status, int runLast)
{
    unsigned char buf[1024];
    char * tmpdir;

    if(saveact == SA_DEFAULT) /* The normal case apart from R_Suicide */
	saveact = SaveAction;

    if(saveact == SA_SAVEASK) {
	if(R_Interactive) {
	qask:
	    R_ClearerrConsole();
	    R_FlushConsole();
	    R_ReadConsole("Save workspace image? [y/n/c]: ",
			  buf, 128, 0);
	    switch (buf[0]) {
	    case 'y':
	    case 'Y':
		saveact = SA_SAVE;
		break;
	    case 'n':
	    case 'N':
		saveact = SA_NOSAVE;
		break;
	    case 'c':
	    case 'C':
		jump_to_toplevel();
		break;
	    default:
		goto qask;
	    }
	} else
	    saveact = SaveAction;
    }
    switch (saveact) {
    case SA_SAVE:
	if(runLast) R_dot_Last();
	if(R_DirtyImage) R_SaveGlobalEnv();
#ifdef HAVE_LIBREADLINE
# ifdef HAVE_READLINE_HISTORY_H
	if(R_Interactive && UsingReadline) {
	    R_setupHistory(); /* re-read the history size and filename */
	    stifle_history(R_HistorySize);
	    write_history(R_HistoryFile);
	}
# endif /* HAVE_READLINE_HISTORY_H */
#endif /* HAVE_LIBREADLINE */
	break;
    case SA_NOSAVE:
	if(runLast) R_dot_Last();
	break;
    case SA_SUICIDE:
    default:
        break;
    }
    R_RunExitFinalizers();
    CleanEd();
    if(saveact != SA_SUICIDE) KillAllDevices();
    if((tmpdir = getenv("R_SESSION_TMPDIR"))) {
	snprintf((char *)buf, 1024, "rm -rf %s", tmpdir);
	R_system((char *)buf);
    }
    if(saveact != SA_SUICIDE && R_CollectWarnings)
	PrintWarnings();	/* from device close and .Last */
    fpu_setup(FALSE);

    exit(status);
}

/*
 *  7) PLATFORM DEPENDENT FUNCTIONS
 */

int Rstd_ShowFiles(int nfile, 		/* number of files */
		   char **file,		/* array of filenames */
		   char **headers,	/* the `headers' args of file.show.
					   Printed before each file. */
		   char *wtitle,	/* title for window
					   = `title' arg of file.show */
		   Rboolean del,	/* should files be deleted after use? */
		   char *pager)		/* pager to be used */

{
/*
	This function can be used to display the named files with the
	given titles and overall title.	 On GUI platforms we could
	use a read-only window to display the result.  Here we just
	make up a temporary file and invoke a pager on it.
*/

    int c, i, res;
    char *filename;
    FILE *fp, *tfp;
    char buf[1024];

    if (nfile > 0) {
        if (pager == NULL || strlen(pager) == 0) pager = "more";
	filename = R_tmpnam(NULL, R_TempDir); /* mallocs result */
        if ((tfp = fopen(filename, "w")) != NULL) {
	    for(i = 0; i < nfile; i++) {
		if (headers[i] && *headers[i])
		    fprintf(tfp, "%s\n\n", headers[i]);
		if ((fp = R_fopen(R_ExpandFileName(file[i]), "r"))
		    != NULL) {
		    while ((c = fgetc(fp)) != EOF)
			fputc(c, tfp);
		    fprintf(tfp, "\n");
		    fclose(fp);
		    if(del)
			unlink(R_ExpandFileName(file[i]));
		}
		else
		    fprintf(tfp, "NO FILE %s\n\n", file[i]);
	    }
	    fclose(tfp);
	}
	snprintf(buf, 1024, "%s < %s", pager, filename);
	res = R_system(buf);
	unlink(filename);
	free(filename);
	return (res != 0);
    }
    return 1;
}


    /*
       Prompt the user for a file name.  Return the length of
       the name typed.  On Gui platforms, this should bring up
       a dialog box so a user can choose files that way.
    */



int Rstd_ChooseFile(int new, char *buf, int len)
{
    int namelen;
    char *bufp;
    R_ReadConsole("Enter file name: ", (unsigned char *)buf, len, 0);
    namelen = strlen(buf);
    bufp = &buf[namelen - 1];
    while (bufp >= buf && isspace((int)*bufp))
	*bufp-- = '\0';
    return strlen(buf);
}


void Rstd_ShowMessage(char *s)
{
    REprintf("%s\n", s);
}


void Rstd_read_history(char *s)
{
#ifdef HAVE_LIBREADLINE
# ifdef HAVE_READLINE_HISTORY_H
    if(R_Interactive && UsingReadline) {
	read_history(s);
    }
# endif /* HAVE_READLINE_HISTORY_H */
#endif /* HAVE_LIBREADLINE */
}

void Rstd_loadhistory(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP sfile;
    char file[PATH_MAX], *p;

    sfile = CAR(args);
    if (!isString(sfile) || LENGTH(sfile) < 1)
	errorcall(call, _("invalid 'file' argument"));
    p = R_ExpandFileName(CHAR(STRING_ELT(sfile, 0)));
    if(strlen(p) > PATH_MAX - 1)
	errorcall(call, _("'file' argument is too long"));
    strcpy(file, p);
#if defined(HAVE_LIBREADLINE) && defined(HAVE_READLINE_HISTORY_H)
    if(R_Interactive && UsingReadline) {
	clear_history();
	read_history(file);
    } else errorcall(call, _("no history mechanism available"));
#else
    errorcall(call, _("no history mechanism available"));
#endif
}

void Rstd_savehistory(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP sfile;
    char file[PATH_MAX], *p;

    sfile = CAR(args);
    if (!isString(sfile) || LENGTH(sfile) < 1)
	errorcall(call, _("invalid 'file' argument"));
    p = R_ExpandFileName(CHAR(STRING_ELT(sfile, 0)));
    if(strlen(p) > PATH_MAX - 1)
	errorcall(call, _("'file' argument is too long"));
    strcpy(file, p);
#if defined(HAVE_LIBREADLINE) && defined(HAVE_READLINE_HISTORY_H)
    if(R_Interactive && UsingReadline) {
	write_history(file);
#ifdef HAVE_HISTORY_TRUNCATE_FILE
	R_setupHistory(); /* re-read the history size */
	history_truncate_file(file, R_HistorySize);
#endif
    } else errorcall(call, _("no history available to save"));
#else
    errorcall(call, _("no history available to save"));
#endif
}




#ifdef _R_HAVE_TIMING_
# include <time.h>
# ifdef HAVE_SYS_TIMES_H
#  include <sys/times.h>
# endif
# ifndef CLK_TCK
/* this is in ticks/second, generally 60 on BSD style Unix, 100? on SysV
 */
#  ifdef HZ
#   define CLK_TCK HZ
#  else
#   define CLK_TCK 60
#  endif
# endif /* not CLK_TCK */



#define MIN(a, b) ((a) < (b) ? (a) : (b))

SEXP do_syssleep(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    int Timeout;
    double tm;
    struct tms timeinfo;
    double timeint, start, elapsed;

    checkArity(op, args);
    timeint = asReal(CAR(args));
    if (ISNAN(timeint) || timeint < 0)
	errorcall(call, _("invalid 'time' value"));
    tm = timeint * 1e6;

    start = times(&timeinfo);
    for (;;) {
	fd_set *what;
        Timeout = R_wait_usec ? MIN(tm, R_wait_usec) : tm;
	what = R_checkActivity(Timeout, 1);

	/* Time up? */
	elapsed = (times(&timeinfo) - start) / (double)CLK_TCK;
	if(elapsed >= timeint) break;

	/* Nope, service pending events */
	R_runHandlers(R_InputHandlers, what);

	/* Servicing events might take some time, so recheck: */
	elapsed = (times(&timeinfo) - start) / (double)CLK_TCK;
	if(elapsed >= timeint) break;

	tm = 1e6*(timeint - elapsed); /* old code had "+ 10000;" */
    }

    return R_NilValue;
}

#else /* not _R_HAVE_TIMING_ */
SEXP do_syssleep(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    error(_("Sys.sleep is not implemented on this system"));
    return R_NilValue;		/* -Wall */
}
#endif /* not _R_HAVE_TIMING_ */
