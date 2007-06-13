source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# function definitions with the same name in different lexical scopes

f <- function(x) print(x + 10)
s1 <- function() {
  f(300)
  f <- function(x) print(x + 11)
  s2 <- function() {
    f(100)
    f <- function(x) print(x + 12)
    f(200)
  }
  s2()
  f(400)
}
f(500)
s1()
f(600)
