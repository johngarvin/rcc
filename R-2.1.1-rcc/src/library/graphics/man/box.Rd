\name{box}
\title{Draw a Box around a Plot}
\usage{
box(which = "plot", lty = "solid", \dots)
}
\alias{box}
\arguments{
\item{which}{character, one of \code{"plot"}, \code{"figure"},
  \code{"inner"} and \code{"outer"}.}
\item{lty}{line type of the box.}
\item{\dots}{further graphical parameters, such as
  \code{bty}, \code{col}, or \code{lwd}, see \code{\link{par}}.}
}
\description{
This function draws a box around the current plot in the given color
and linetype.  The \code{bty} parameter determines
the type of box drawn.  See \code{\link{par}} for details.
}
\references{
  Becker, R. A., Chambers, J. M. and Wilks, A. R. (1988)
  \emph{The New S Language}.
  Wadsworth \& Brooks/Cole.
}
\seealso{
\code{\link{rect}} for drawing of arbitrary rectangles.
}
\examples{
plot(1:7,abs(rnorm(7)), type='h', axes = FALSE)
axis(1, labels = letters[1:7])
box(lty='1373', col = 'red')
}
\keyword{aplot}
