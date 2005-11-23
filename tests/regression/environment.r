rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

bar <- function() {
  foo <- 34
  foo
}

foo <- 12
bar()
foo
