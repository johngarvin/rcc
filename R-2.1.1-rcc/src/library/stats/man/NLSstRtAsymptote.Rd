% $Id: NLSstRtAsymptote.Rd,v 1.2 2005/08/15 16:11:28 johnmc Exp $
\name{NLSstRtAsymptote}
\title{Horizontal Asymptote on the Right Side}
\alias{NLSstRtAsymptote}
\alias{NLSstRtAsymptote.sortedXyData}
\description{
  Provide an initial guess at the horizontal asymptote on the right side
  (i.e., large values of \code{x}) of the graph of \code{y} versus
  \code{x} from the \code{xy} object.  Primarily used within
  \code{initial} functions for self-starting nonlinear regression
  models.
}
\usage{
NLSstRtAsymptote(xy)
}
\arguments{
 \item{xy}{a \code{sortedXyData} object}
}
\value{
  A single numeric value estimating the horizontal asymptote for large
  \code{x}.
}
\author{Jose Pinheiro and Douglas Bates}
\seealso{
  \code{\link{sortedXyData}},
  \code{\link{NLSstClosestX}},
  \code{\link{NLSstRtAsymptote}},
  \code{\link{selfStart}}
}
\examples{
DNase.2 <- DNase[ DNase$Run == "2", ]
DN.srt <- sortedXyData( expression(log(conc)), expression(density), DNase.2 )
NLSstRtAsymptote( DN.srt )
}
\keyword{manip}
