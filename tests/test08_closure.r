foo <- function(n) {
  bar <- function (n) {
    n + 1
  }
  bar(n) + 3
}
foo(10)
