source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

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
