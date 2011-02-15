source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# input dependence; CBN->CBV optimization is still possible

global <- 0

foo <- function(n) {
  print(global)
  print(n)
}

g <- function() {
  print(global)
  return(100)
}

foo(g())
print(global)
