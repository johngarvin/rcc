source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# true dependence prevents CBN->CBV optimization

global <- 0

foo <- function(n) {
  global <<- 10
  print(n)
}

g <- function() {
  print(global)
}

foo(g())
print(global)
