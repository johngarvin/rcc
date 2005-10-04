outer <- function(a){
x <- a 
function() { 
x+1
} ()
}
outer(6)
