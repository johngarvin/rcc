source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# This tests if strictness analysis correctly accounts for defs that
# kill a formal argument.

# "A function is strict in v if there is a use of v before a def of v
# on all paths."
# This is the correct definition.

# "a function is strict in v if there is a use of v on all paths" This
# is an incorrect definition. Counterexample: foo has a use of x on
# all paths, but foo is nonstrict in x. (It doesn't cause the actual
# argument to be lazily evaluated.)

a <- 0

foo <- function(x) {
  x <- 10
  x + 2
}

print(foo(a <<- 100))
print(a)
