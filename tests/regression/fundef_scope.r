rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

for(i in 1:3) {
  if (i == 1) {
    foo <- function() print(i)
  } else if (i == 2) {
    bar <- function() print(10*i)
  } else if (i == 3) {
    foo()
    bar()
  }
}
