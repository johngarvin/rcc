\name{dimnames}
\title{Dimnames of an Object}
\alias{dimnames}
\alias{dimnames<-}
\alias{dimnames.data.frame}
\alias{dimnames<-.data.frame}
\description{
  Retrieve or set the dimnames of an object.
}
\usage{
dimnames(x)
dimnames(x) <- value
}
\arguments{
  \item{x}{an \R object, for example a matrix, array or data frame.}
  \item{value}{a possible value for \code{dimnames(x)}: see
    \dQuote{Value}.}
}
\details{
  The functions \code{dimnames} and \code{dimnames<-} are generic.

  For an \code{\link{array}} (and hence in particular, for a
  \code{\link{matrix}}), they retrieve or set the \code{dimnames}
  attribute (see \link{attributes}) of the object.  The list
  \code{value} can have names, and these will be used to label the
  dimensions of the array where appropriate.

  Both have methods for data frames.  The dimnames of a data frame are
  its \code{row.names} attribute and its \code{\link{names}}.

  As from \R 1.8.0 factor components of \code{value} will be coerced to
  character. 
}
\value{
  The dimnames of a matrix or array can be \code{NULL} or a list of the
  same length as \code{dim(x)}.  If a list, its components are either
  \code{NULL} or a character vector the length of the appropriate
  dimension of \code{x}.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth \& Brooks/Cole.
}
\seealso{
  \code{\link{rownames}}, \code{\link{colnames}};
  \code{\link{array}}, \code{\link{matrix}}, \code{\link{data.frame}}.
}
\examples{
## simple versions of rownames and colnames
## could be defined as follows
rownames0 <- function(x) dimnames(x)[[1]]
colnames0 <- function(x) dimnames(x)[[2]]
}
\keyword{array}
\keyword{manip}
