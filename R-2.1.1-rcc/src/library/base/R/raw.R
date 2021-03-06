raw <- function(length = 0) vector("raw", length)

as.raw <- function(x) as.vector(x, "raw")

charToRaw <- function(x) .Internal(charToRaw(x))
rawToChar <- function(x, multiple=FALSE) .Internal(rawToChar(x, multiple))

rawShift <- function(x, n) .Internal(rawShift(x, n))

rawToBits <- function(x) .Internal(rawToBits(x))
intToBits <- function(x) .Internal(intToBits(x))

packBits <- function(x, type=c("raw", "integer"))
{
    type <- match.arg(type)
    .Internal(packBits(x, type))
}
