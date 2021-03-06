\name{base-internal}
\alias{.subset}
\alias{.subset2}
\alias{.find.package}
\alias{.path.package}
\alias{.getRequiredPackages}
\alias{.getRequiredPackages2}
\alias{testPlatformEquivalence}

\title{Internal Functions in the Base Package}
\description{
  Internal functions in the base package, which are only user-visible
  because of the special nature of the base namespace.
}
\usage{
.subset(x, \dots)
.subset2(x, \dots)

.find.package(package, lib.loc = NULL, quiet = FALSE,
              verbose = getOption("verbose"))

.path.package(package, quiet = FALSE)

.getRequiredPackages(file = "DESCRIPTION", quietly = FALSE,
                     useImports = FALSE)
.getRequiredPackages2(pkgInfo, quietly = FALSE, lib.loc = NULL,
                      useImports = FALSE)

testPlatformEquivalence(built, run)
}
\arguments{
  \item{x}{object from which to extract elements.}
  \item{package}{the name of a package.}
  \item{lib.loc}{a character vector describing the location of \R
    library trees to search through, or \code{NULL}.  The default value
    of \code{NULL} corresponds to all libraries currently known.}
  \item{quiet}{logical.  Should this not give warnings or an error
    if the package(s) are not found?}
  \item{useImports}{logical.  Should  the \code{Imports} field be processed?}
  \item{verbose}{a logical.  If \code{TRUE}, additional diagnostics are
    printed.}
  \item{file}{The full path to the \file{DESCRIPTION} file of a package.}
  \item{quietly}{logical: should progress messages be given.}
  \item{pkginfo}{A processed \file{DESCRIPTION} file of class
    \code{"packageDescription2"}.}
  \item{built, run}{platform strings of the form \code{"cpu-vendor-os"}.}
}
\details{
  The functions \code{.subset} and \code{.subset2} are essentially
  equivalent to the \code{\link{[}} and \code{\link{[[}} operators,
  except methods that dispatch does not take place.  This is to avoid
  expensive unclassing when applying the default method to an object.  They
  should not normally be invoked by end users.

  \code{.find.package} returns the paths to the locations where the
  given packages can be found.  If \code{lib.loc} is \code{NULL}, then
  then attached packages are searched before the libraries.  If a
  package is found more than once, the first match is used.  Unless
  \code{quiet = TRUE} a warning will be given about the named packages
  which are not found, and an error if none are.  If \code{verbose} is
  true, warnings about packages found more than once are given.  It does
  know about versioned installs of packages, and for a package to be
  returned it must have a \file{DESCRIPTION} file containing a valid
  \code{version} field.

  \code{.path.package} returns the paths from which the named packages
  were loaded, or if none were named, for all currently loaded packages.
  Unless \code{quiet = TRUE} it will warn if some of the packages named
  are not loaded, and given an error if none are.  This function is not
  meant to be called by users, and its interface might change in future
  versions.  Note that currently \code{package} must include the
  version mumber if versioned installs are used.

  \code{.getRequiredPackages2} attaches all the packages mentioned in the
  \code{Depends} field: failure to find a package is an error.  It also
  checks the versions of the packages found against the \code{Depends} field.

  \code{.getRequiredPackages} is a wrapper to
  \code{.getRequiredPackages2} using a \file{DESCRIPTION} file.

  \code{testPlatformEquivalence} is used to test if a package with 
  compiled code can safely be loaded into a build of \R, which it does by
  comparing their \sQuote{canonical} \code{"cpu-vendor-os"} descriptions.
  The default function omits the \code{vendor} part and allows small 
  mismatches elsewhere.  Cognescenti can replace this function in the
  \code{base} namespace.
}
\value{
  \code{.find.package} and \code{.path.packages} return a character
  vector of paths to installed packages.
  
  \code{.getRequiredPackages} and \code{.getRequiredPackages2}
  return \code{invisible()}.

  \code{testPlatformEquivalence}: \code{TRUE} or \code{FALSE}.
}
\keyword{internal}
