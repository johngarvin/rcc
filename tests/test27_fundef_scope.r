Rprof("prof-output")
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
Rprof(NULL)
