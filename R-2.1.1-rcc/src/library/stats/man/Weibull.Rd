\name{Weibull}
\alias{Weibull}
\alias{dweibull}
\alias{pweibull}
\alias{qweibull}
\alias{rweibull}
\title{The Weibull Distribution}
\description{
  Density, distribution function, quantile function and random
  generation for the Weibull distribution with parameters \code{shape}
  and \code{scale}.
}
\usage{
dweibull(x, shape, scale = 1, log = FALSE)
pweibull(q, shape, scale = 1, lower.tail = TRUE, log.p = FALSE)
qweibull(p, shape, scale = 1, lower.tail = TRUE, log.p = FALSE)
rweibull(n, shape, scale = 1)
}
\arguments{
  \item{x, q}{vector of quantiles.}
  \item{p}{vector of probabilities.}
  \item{n}{number of observations. If \code{length(n) > 1}, the length
    is taken to be the number required.}
  \item{shape, scale}{shape and scale parameters, the latter defaulting to 1.}
  \item{log, log.p}{logical; if TRUE, probabilities p are given as log(p).}
  \item{lower.tail}{logical; if TRUE (default), probabilities are
    \eqn{P[X \le x]}{P[X <= x]}, otherwise, \eqn{P[X > x]}{P[X > x]}.}
}
\value{
  \code{dweibull} gives the density,
  \code{pweibull} gives the distribution function,
  \code{qweibull} gives the quantile function, and
  \code{rweibull} generates random deviates.
}
\details{
  The Weibull distribution with \code{shape} parameter \eqn{a} and
  \code{scale} parameter \eqn{\sigma}{b} has density given by
  \deqn{f(x) = (a/\sigma) {(x/\sigma)}^{a-1} \exp (-{(x/\sigma)}^{a})}{%
        f(x) = (a/b) (x/b)^(a-1) exp(- (x/b)^a)}
  for \eqn{x > 0}.
  The cumulative is
  \eqn{F(x) = 1 - \exp(-{(x/\sigma)}^a)}{F(x) = 1 - exp(- (x/b)^a)}, the
  mean is \eqn{E(X) = \sigma \Gamma(1 + 1/a)}{E(X) = b Gamma(1 + 1/a)}, and
  the \eqn{Var(X) = \sigma^2(\Gamma(1 + 2/a)-(\Gamma(1 + 1/a))^2)}{%
           Var(X) = b^2 * (gamma(1 + 2/a) - (gamma(1 + 1/a))^2)}.
}
\note{
  The cumulative hazard \eqn{H(t) = - \log(1 - F(t))}{H(t) = - log(1 - F(t))}
  is \code{-pweibull(t, a, b, lower = FALSE, log = TRUE)} which is just
  \eqn{H(t) = {(t/b)}^a}.
}
\seealso{
  \code{\link{dexp}} for the Exponential which is a special case of a
  Weibull distribution.
}
\examples{
x <- c(0,rlnorm(50))
all.equal(dweibull(x, shape = 1), dexp(x))
all.equal(pweibull(x, shape = 1, scale = pi), pexp(x, rate = 1/pi))
## Cumulative hazard H():
all.equal(pweibull(x, 2.5, pi, lower=FALSE, log=TRUE), -(x/pi)^2.5, tol=1e-15)
all.equal(qweibull(x/11, shape = 1, scale = pi), qexp(x/11, rate = 1/pi))
}
\keyword{distribution}
