a <- 5
foo <- function() {
  a <- 10
}
bar <- function() {
  a <<- 12
}
print(a)
foo()
print(a)
bar()
print(a)
