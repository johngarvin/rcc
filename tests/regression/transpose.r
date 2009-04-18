source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

# simplified from spline.r: transpose and matrix multiplication

x <- matrix(c(1,2,3,4), nrow=2)

print(x)
print(t(x)%*%x+x*t(x)%*%x)
