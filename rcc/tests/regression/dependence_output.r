source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# output dependence prevents CBN->CBV optimization

global <- 0

foo <- function(n) {
  global <<- 10
  print(n)
}

g <- function() {
  global <<- 20
  return(100)
}

foo(g())
print(global)
