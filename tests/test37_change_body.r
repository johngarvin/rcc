f <- function() 10
f()
body(f) <- list(100)
f()
