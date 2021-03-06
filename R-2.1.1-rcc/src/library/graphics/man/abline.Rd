\name{abline}
\alias{abline}
\title{Add a Straight Line to a Plot}
\description{
  This function adds one or more straight lines through the current plot.
}
\synopsis{
abline(a = NULL, b = NULL, h = NULL, v = NULL, reg = NULL,
       coef = NULL, untf = FALSE, col = par("col"),
       lty = par("lty"), lwd = NULL, ...)
}
\usage{
abline(a, b, untf = FALSE, \dots)
abline(h=, untf = FALSE, \dots)
abline(v=, untf = FALSE, \dots)
abline(coef=, untf = FALSE, \dots)
abline(reg=, untf = FALSE, \dots)
}
\arguments{
  \item{a,b}{the intercept and slope.}
  \item{untf}{logical asking to \emph{untransform}.  See Details.}
  \item{h}{the y-value for a horizontal line.}
  \item{v}{the x-value for a vertical line.}
  \item{coef}{a vector of length two giving the intercept and slope.}
  \item{reg}{an object with a \code{coef} component. See Details.}
  \item{\dots}{graphical parameters.}
}
\details{
  The first form specifies the line in intercept/slope form
  (alternatively \code{a} can be specified on its own and is taken to
  contain the slope and intercept in vector form).

  The \code{h=} and \code{v=} forms draw horizontal and vertical lines
  at the specified coordinates.

  The \code{coef} form specifies the line by a vector containing the
  slope and intercept.

  \code{reg} is a regression object which contains \code{reg$coef}.  If
  it is of length 1 then the value is taken to be the slope of a line
  through the origin, otherwise, the first 2 values are taken to be the
  intercept and slope.

  If \code{untf} is true, and one or both axes are log-transformed, then
  a curve is drawn corresponding to a line in original coordinates,
  otherwise a line is drawn in the transformed coordinate system. The
  \code{h} and \code{v} parameters always refer to original coordinates.

  The graphical parameters \code{col} and \code{lty} can be specified as
  arguments to \code{abline}; see \code{par} for details.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth \& Brooks/Cole.
}
\seealso{
  \code{\link{lines}} and \code{\link{segments}} for connected and
  arbitrary lines given by their \emph{endpoints}.
  \code{\link{par}}.
}
\examples{
z <- lm(dist ~ speed, data = cars)
plot(cars)
abline(z)
}
\keyword{aplot}
