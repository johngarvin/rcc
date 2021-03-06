\name{Round}
\title{Rounding of Numbers}
\usage{
ceiling(x)
floor(x)
round(x, digits = 0)
signif(x, digits = 6)
trunc(x)
zapsmall(x, digits = getOption("digits"))
}
\alias{ceiling}
\alias{floor}
\alias{round}
\alias{signif}
\alias{trunc}
\alias{zapsmall}
\description{
  \code{ceiling} takes a single numeric argument \code{x} and returns a
  numeric vector containing the smallest integers not less than the
  corresponding elements of \code{x}.

  \code{floor} takes a single numeric argument \code{x} and returns a
  numeric vector containing the largest integers not greater than the
  corresponding elements of \code{x}.

  \code{round} rounds the values in its first argument to the specified
  number of decimal places (default 0).
  Note that for rounding off a 5, the IEEE standard is used,
  \dQuote{\emph{go to the even digit}}.
  Therefore \code{round(0.5)} is \code{0} and \code{round(-1.5)} is \code{-2}.

  \code{signif} rounds the values in its first argument to the specified
  number of significant digits.

  \code{trunc} takes a single numeric argument \code{x} and returns a
  numeric vector containing the integers by truncating the values in
  \code{x} toward \code{0}.

  \code{zapsmall} determines a \code{digits} argument \code{dr} for
  calling \code{round(x, digits = dr)} such that values \dQuote{close to
    zero} (compared with the maximal absolute one) are \dQuote{zapped},
  i.e., treated as \code{0}.
}
\arguments{
  \item{x}{a numeric vector.}
  \item{digits}{integer indicating the precision to be used.}
}
\details{
  All but \code{zapsmall} are generic functions: methods can be
  defined for them individually or via the \code{\link{Math}} group generic.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth \& Brooks/Cole. (except \code{zapsmall}.)

  Chambers, J. M. (1998)
  \emph{Programming with Data. A Guide to the S Language}.
  Springer. (\code{zapsmall}.)
}
\seealso{
  \code{\link{as.integer}}.
}
\examples{
round(.5 + -2:4) # IEEE rounding: -2  0  0  2  2  4  4
( x1 <- seq(-2, 4, by = .5) )
round(x1)#-- IEEE rounding !
x1[trunc(x1) != floor(x1)]
x1[round(x1) != floor(x1 + .5)]
(non.int <- ceiling(x1) != floor(x1))

x2 <- pi * 100^(-1:3)
round(x2, 3)
signif(x2, 3)

print   (x2 / 1000, digits=4)
zapsmall(x2 / 1000, digits=4)
zapsmall(exp(1i*0:4*pi/2))
}
\keyword{arith}
