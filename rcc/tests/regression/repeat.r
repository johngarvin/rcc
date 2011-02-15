source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

i <- 2
repeat {
  print("foo")
  if (i == 6) break
  print("bar")
  i <- i + 2
}
print("baz")
