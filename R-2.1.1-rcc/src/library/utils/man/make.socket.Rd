\name{make.socket}
\alias{make.socket}
\alias{print.socket}
\title{Create a Socket Connection}
\usage{
make.socket(host = "localhost", port, fail = TRUE, server = FALSE)
}
\arguments{
  \item{host}{name of remote host}
  \item{port}{port to connect to/listen on}
  \item{fail}{failure to connect is an error?}
  \item{server}{a server socket?}
}
\description{
  With \code{server = FALSE} attempts to open a client socket to the
  specified port and host. With \code{server = TRUE} listens on the
  specified port for a connection and then returns a server socket. It is
  a good idea to use \code{\link{on.exit}} to ensure that a socket is
  closed, as you only get 64 of them.
}
\value{
  An object of class \code{"socket"}.
  \item{socket}{socket number. This is for internal use}
  \item{port}{port number of the connection}
  \item{host}{name of remote computer}
}
\author{Thomas Lumley}
\references{
  Adapted from Luke Tierney's code for \code{XLISP-Stat}, in turn
  based on code from Robbins and Robbins "Practical UNIX Programming"
}
\section{Warning}{
  I don't know if the connecting host name returned
  when \code{server = TRUE} can be trusted. I suspect not.
} 

\seealso{
  \code{\link{close.socket}}, \code{\link{read.socket}}
}

\examples{
daytime <- function(host = "localhost"){
    a <- make.socket(host, 13)
    on.exit(close.socket(a))
    read.socket(a)
}
## Offical time (UTC) from US Naval Observatory
\dontrun{daytime("tick.usno.navy.mil")}
}
\keyword{misc}
