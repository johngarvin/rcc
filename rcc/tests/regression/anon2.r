source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# anonymous function call with return value

f <- function(a) {
x <- a 
z <- (function() { x+1})()
z
}
f(6)
