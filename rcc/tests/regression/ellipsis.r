source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

foo <- function(a, b, ...) a + b
print(foo(1,2))
print(foo(3,4,5))
print(foo(6,7,8,9,10))
