\name{uniroot}
\title{One Dimensional Root (Zero) Finding}
\usage{
uniroot(f, interval, lower = min(interval), upper = max(interval),
        tol = .Machine$double.eps^0.25, maxiter = 1000, \dots)
}
\alias{uniroot}
\arguments{
  \item{f}{the function for which the root is sought.}
  \item{interval}{a vector containing the end-points of the interval
    to be searched for the root.}
  \item{lower}{the lower end point of the interval to be searched.}
  \item{upper}{the upper end point of the interval to be searched.}
  \item{tol}{the desired accuracy (convergence tolerance).}
  \item{maxiter}{the maximum number of iterations.}
  \item{\dots}{additional arguments to \code{f}.}
}
\description{
  The function \code{uniroot} searches the interval from \code{lower}
  to \code{upper} for a root (i.e., zero) of the function \code{f} with
  respect to its first argument.
}
\details{
  Either \code{interval} or both \code{lower} and \code{upper} must be
  specified.  The function uses Fortran subroutine \file{"zeroin"} (from
  Netlib) based on algorithms given in the reference below.

  If the algorithm does not converge in \code{maxiter} steps, a warning
  is printed and the current approximation is returned.
}
\value{
  A list with four components: \code{root} and \code{f.root} give the
  location of the root and the value of the function evaluated at that
  point. \code{iter} and \code{estim.prec} give the number of iterations
  used and an approximate estimated precision for \code{root}.
}
\references{
  Brent, R. (1973)
  \emph{Algorithms for Minimization without Derivatives.}
  Englewood Cliffs, NJ: Prentice-Hall.
}
\seealso{
  \code{\link{polyroot}} for all complex roots of a polynomial;
  \code{\link{optimize}}, \code{\link{nlm}}.
}
\examples{
f <- function (x,a) x - a
str(xmin <- uniroot(f, c(0, 1), tol = 0.0001, a = 1/3))
str(uniroot(function(x) x*(x^2-1) + .5, low = -2, up = 2, tol = 0.0001),
    dig = 10)
str(uniroot(function(x) x*(x^2-1) + .5, low = -2, up =2 , tol = 1e-10 ),
    dig = 10)

## Find the smallest value x for which exp(x) > 0 (numerically):
r <- uniroot(function(x) 1e80*exp(x) -1e-300,,-1000,0, tol=1e-20)
str(r, digits= 15)##> around -745.1332191

exp(r$r)	# = 0, but not for r$r * 0.999...
minexp <- r$r * (1 - .Machine$double.eps)
exp(minexp)	# typically denormalized
}
\keyword{optimize}
