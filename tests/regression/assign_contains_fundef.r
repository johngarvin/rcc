rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

f <- if (1 == 2) { function(x) x + 1 } else { function(x) x + 2 }
f(10)
