rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

f <- function() 10
f()
body(f) <- list(100)
f()
