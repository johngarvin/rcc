# This file defines R functions used to communicate unevaluated
# logical expressions to the rcc compiler.

.rcc.assert     <- function(...) {}
.rcc.assert.sym <- function(...) {}
.rcc.assert.exp <- function(x, ...) x
