\name{predict.bs}
\title{Evaluate a Spline Basis}
\alias{predict.bs}
\alias{predict.ns}
\description{
  Evaluate a predefined spline basis at given values.
}
\usage{
\method{predict}{bs}(object, newx, \dots)
%% the 'ns' one now looks identical
}
% \synopsis{
% predict.bs(object, newx, \dots)
% predict.ns(object, newx, \dots)
% }
\arguments{
  \item{object}{the result of a call to \code{\link{bs}} or
    \code{\link{ns}} having attributes describing \code{knots},
    \code{degree}, etc.}
  \item{newx}{the \code{x} values at which evaluations are required.}
  \item{\dots}{Optional additional arguments.  Presently no additional
    arguments are used.}
}
\value{
  An object just like \code{object}, except evaluated at the new values
  of \code{x}.

  These are methods for the generic function \code{\link{predict}} for
  objects inheriting from classes \code{"bs"} or \code{"ns"}.  See
  \code{\link{predict}} for the general behavior of this function.
}
\seealso{
  \code{\link{bs}}, \code{\link{ns}}, \code{\link{poly}}.
  %%-- FIXME: These are not in R (and wont ever ?!): \code{lo}, \code{s}
}
\examples{
basis <- ns(women$height, df = 5)
newX <- seq(58, 72, len = 51)
# evaluate the basis at the new data
predict(basis, newX)
}
\keyword{smooth}
