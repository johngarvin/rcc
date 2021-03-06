\name{MethodWithNext-class}
\docType{class}
\alias{MethodWithNext-class}
\title{Class MethodWithNext }
\description{ Class of method definitions set up for callNextMethod }
\section{Objects from the Class}{
  Objects from this class are generated as a side-effect of calls to
  \code{\link{callNextMethod}}.
}
\section{Slots}{
  \describe{
    \item{\code{.Data}:}{Object of class \code{"function"}; the actual
      function definition.}
    \item{\code{nextMethod}:}{Object of class \code{"PossibleMethod"}
      the method to use in response to a \code{\link{callNextMethod}()}
      call.}
    \item{\code{excluded}:}{Object of class \code{"list"}; one or more
      signatures excluded in finding the next method. }
    \item{\code{target}:}{Object of class \code{"signature"}, from class
      \code{"MethodDefinition"}}
    \item{\code{defined}:}{Object of class \code{"signature"}, from
      class \code{"MethodDefinition"}} 
  }
}
\section{Extends}{
  Class \code{"MethodDefinition"}, directly.\cr
  Class \code{"function"}, from data part.\cr
  Class \code{"PossibleMethod"}, by class \code{"MethodDefinition"}.\cr
  Class \code{"OptionalMethods"}, by class \code{"MethodDefinition"}.
}
\section{Methods}{
  \describe{
    \item{findNextMethod}{\code{signature(method = "MethodWithNext")}:
      used internally by method dispatch. }
    \item{loadMethod}{\code{signature(method = "MethodWithNext")}: used
      internally by method dispatch. }
    \item{show}{\code{signature(object = "MethodWithNext")} }
  }
}

\seealso{  \code{\link{callNextMethod}}, and 
 \code{\link{MethodDefinition-class}}. }

\keyword{classes}
