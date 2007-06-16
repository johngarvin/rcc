source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# anonymous function call

f <- function(a) {
x <- a 
(function() {
  x+1
})()
}
f(6)
