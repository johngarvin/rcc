rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

foo <- function(n, m) n + m + 1
bar <- 50
foo(40, bar)
