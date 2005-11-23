rcc.include <- Sys.getenv("RCC_R_INCLUDE_PATH")
source(file.path(rcc.include, "well_behaved.r"))

bar <- 0
for(i in 1:1000) {
  foo <- function(a,b) 3*a - 4*b + i
  bar <- bar + foo(20,15)
}
print(bar)
