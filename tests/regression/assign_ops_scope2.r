foo <- function() {
  n <<- 10
}

bar <- function() {
  n <<- 5
  foo()
  n
}

bar()
