####################################################################
#You can start R by typing R in unix, but typing (not just typing R):
#
#            R --vsize=40M --nsize=2500k
#
#to obtain more memory and space, for example.
######################################################################

#########################################################################
# Main program shave() and 4 subroutines: pcs(),showclust(),gene.colors()
##########################################################################
shave <-
function(A, NumClusts, NumPerm)
{
        ##this program will show all plots on one sheet...  
        alpha <- 0.9
        N <- nrow(A)
        P <- ncol(A)
        A <- as.matrix(A)
        par(mfrow = c(1, 1), cex = 0.3)
        showclust(A, 1)
        par(mfrow = c(4, 3), cex = 0.3)
        ### A is row-centered:
        A <- sweep(A, 1, apply(A, 1, mean))
        CurrentMatrix <- A
        ##then, compute how many shaving times are needed if N>10, after
        ##which we shave just one gene at a time:
        account <- 0
        SizeNested <- N
        ##save some space:
        N.shave <- max(40,  - log(N)/log(alpha))
        size.after.shaving <- numeric(N.shave)
        while(SizeNested > 2) {
                SizeNested <- floor(SizeNested * alpha)
                account <- account + 1
                size.after.shaving[account] <- SizeNested
        }
        ##thus, we need "account" times of shaving to reach 2 genes
        Dstar <- matrix(0, account, NumPerm)
        ## R-square will be saved in Dstar in each permutation
        clust <- 1
        while(clust < (NumClusts + 1)) {
                gapsize <- pcs(CurrentMatrix, account, alpha)
                ##R-square and between-variance:
                Dk <- gapsize$R2
                VB <- gapsize$vb
                ###permute CurrentMatrix, then repeat the above steps: 
                perm <- 0
                while(perm < NumPerm) {
                        perm <- perm + 1
                        Z <- t(apply(CurrentMatrix, 1, sample))
                        ## permute within each row
                        Dstar[, perm] <- pcs(Z, account, alpha)$R2
                }
                Dk.R <- apply(Dstar, 1, mean)
                ##Dk.R is mean of R-square of the permuted data
                Gap <- Dk - Dk.R
                if(clust == 5) {
                        X11()
                        par(mfrow = c(4, 3))
                }
                x <- size.after.shaving[size.after.shaving != 0]
                subtitl <- paste("Cluster #", clust)
                plot(x, Gap, log = "x", xlab = "Cluster Size")
                title(main = subtitl)
                matplot(x, cbind(Dk,Dk.R), type="l",lty=c(1,2), log = "x", 
                        xlab = "Cluster Size",ylab ="Variance", 
                        col=c(1,2),ylim = c(0, 1))
                xy_locator(1) 
                legend(xy, c("Real Data", "Randomized Data"),lty=c(1,2),
                        bty="n",col=c(1,2))
                ##now, esitmate the optimal cluster in which  the label of
                ##genes is denoted by xx:
                xx <- gapsize$IndGen[[order( - Gap)[1]]]
                print(xx)
                showclust(A[xx,  ], 2)
                ## then, show %variance,VB,VT,VW,...:
                variance <- round(Dk[Gap == max(Gap)], 3)
                vbetween <- round(VB[Gap == max(Gap)], 2)
                vtotal <- round(vbetween/variance, 2)
                vbvw <- round(Dk[Gap == max(Gap)]/(1 - Dk[Gap == max(Gap)]),
                        2)
                subtitl <- paste("%variance=", variance * 100, "VB/VW=", vbvw,
                        "VB=", vbetween, "VT=", vtotal)
                title(main = subtitl)
                ## start to search for another cluster:
                clust <- clust + 1
                if(clust > NumClusts) {
                        break
                }
                xbar <- gapsize$z.bar[order( - Gap)[1],  ]
                CurrentMatrix <- cbind(CurrentMatrix, matrix(rep(xbar, N),
                        N, P, byrow = T))
                ##orth. CurrentMatrix:
                CurrentMatrix <- t(apply(CurrentMatrix, 1, lmfit))
                dimnames(CurrentMatrix) <- dimnames(A)
                CurrentMatrix <- sweep(CurrentMatrix, 1, apply(CurrentMatrix,
                        1, mean))
        }
        return()
}
pcs <-
function(z, acc, alpha){
        R2 <- vb <- rep(0, acc)
        IndGen <- list(acc)
        z.bar <- matrix(0, acc, dim(z)[[2]])
        i <- 0
        repeat {
                i <- i + 1
                #pca <- as.vector(abs(svd(z)$u[, 1]))
                pca <- as.vector(svd(z)$u[, 1])
                genesign <- ifelse(pca > 0, 1, -1)
                z1 <- z * genesign
                pca <- abs(pca)
                if(length(pca) > 2) {
                        k <- length(pca) - floor(alpha * length(pca))
                }
                if(length(pca) == 2)
                        break
                z <- z[ - (order(pca)[1:k]),  ]
                z.sign <- z1[ - (order(pca)[1:k]),  ]
                z.bar[i,  ] <- apply(z.sign, 2, mean)
                d <- dim(z)
                IndGen[[i]] <- dimnames(z)[[1]]
                vb[i] <- ((d[[2]] - 1) * var(apply(z.sign, 2,mean)))/d[[2]]
                vw <- ((d[[1]] - 1) * mean(diag(var(z.sign))))/d[[1]]
                bw <- vb[i]/vw
                R2[i] <- bw/(1 + bw)
        }
        return(R2, vb, IndGen, z.bar)
}
showclust <-
function(z, flag)
{
        if(flag == 1) {
                #graph <- image(t(z), axes = F)
                x <- c(1:dim(z)[1])
                y <- c(1:dim(z)[2])
                image(x, y, z, axes =F,col=gene.colors(20),
                           xlab="",ylab="")
                axis(1, at = x, labels = dimnames(z)[[1]], las=2, adj = 1,
                         las=2)
                axis(2, at = y, labels = dimnames(z)[[2]], adj = 1,
                        las=2)
        }
        else {
                y <- c(1:dim(z)[1])
                x <- c(1:dim(z)[2])
                tmp <- rbind(z, apply(z, 2, mean))
                tmp <- tmp[1:(dim(z)[1] + 1), order(tmp[(dim(z)[1] + 1),  ])]
                z <- tmp[ - (1 + dim(z)[1]),  ]
                image(x, y, t(z), axes =F,col=gene.colors(20),
                               xlab="",ylab="")
                axis(1, at = x, labels = dimnames(t(z))[[1]],adj=1,las=2)
                axis(2, at = y, labels = dimnames(t(z))[[2]],adj=1,las=2)
        }
}
gene.colors <-
function (n) 
{
    if ((n <- as.integer(n[1])) > 0) {
        even.n <- n%%2 == 0
        k <- n%/%2
        l1 <- k + 1 - even.n
        l2 <- n - k + even.n
        c(if (l1 > 0) hsv(h = 0.4, s = seq(0.5, ifelse(even.n, 
            0.5/k, 0), length = l1), v = 1), if (l2 > 1) hsv(h = 1, 
            s = seq(0, 0.5, length = l2)[-1], v = 1))
    }
    else character(0)
}
 
############################################################################
#Main program super() and subroutines: pcs.sup(),super.id(),showclust() 
#########################################################################
super <-
function(A, clf, lam, NumClusts, NumPerm)
{
        #######################################################################
        # clf is a vector which indicates the cell classification, for example, 
        # if the first 4 cells and last 3 cells (7 cells in total,say) are two 
        # different groups, then clf is (1,1,1,1,2,2,2). An example of running 
        # this program for gulob data in Splus will be:  
        #>super.new(read.table("golubnew.tsv",header=T),scan("golub.clf"),2,18)
        ######################################################################## 
        alpha <- 0.9
        N <- nrow(A)
        P <- ncol(A)
        A <- as.matrix(A)
        par(mfrow = c(1, 1), cex = 0.4)
        showclust(A, 1)
        par(mfrow = c(4, 3), cex = 0.3)
        A <- sweep(A, 1, apply(A, 1, mean))
        ### A is row-centered
        CurrentMatrix <- rbind(A, clf)
        CurrentMatrix <- CurrentMatrix[, order(as.numeric(CurrentMatrix[(N +
                1),  ]))]
        Q <- super.id(CurrentMatrix[(N + 1),  ], lam)
        ## a function 'super.id' produces a matrix Q such that AQ is 
        ## the working matrix (not A) in supervised shaving
        A <- CurrentMatrix <- CurrentMatrix[(1:N),  ]
        ##Compute how many shaving times are needed if N > 10, after which we
        ##shave just one gene at a time:
        account <- 0
        SizeNested <- N
        N.shave <- max(40,  - log(N)/log(alpha))
        size.after.shaving <- numeric(N.shave)
        while(SizeNested > 2) {
                SizeNested <- floor(SizeNested * alpha)
                account <- account + 1
                size.after.shaving[account] <- SizeNested
        }
        ###That is, we need "account" times of shaving to reach 2 genes
        Dstar <- matrix(0, account, NumPerm)
        clust <- 1
        while(clust < (NumClusts + 1)) {
                gapsize <- pcs.sup(CurrentMatrix, Q, account, alpha)
                Dk <- gapsize$R2
                VB <- gapsize$vb
                # R^2
                ###permute CurrentMatrix, then repeat the above steps: 
                perm <- 0
                while(perm < NumPerm) {
                        perm <- perm + 1
                        Z <- t(apply(CurrentMatrix, 1, sample))
                        ## permute within each row
                        Dstar[, perm] <- pcs.sup(Z, Q, account, alpha)$R2
                }
                Dk.R <- apply(Dstar, 1, mean)
                Gap <- Dk - Dk.R
                if(clust == 5) {
                        X11()
                        par(mfrow = c(4, 3))
                }
                x <- size.after.shaving[size.after.shaving != 0]
                plot(x, Gap, log = "x", xlab = "Cluster Size")
                subtitl <- paste("Cluster #", clust)
                title(main = subtitl)
                matplot(x, cbind(Dk,Dk.R), type="l",lty=c(1,2), log = "x",
                        xlab = "Cluster Size",ylab ="Variance",
                        col=c(1,2),ylim = c(0, 1))
                xy_locator(1)
                legend(xy, c("Real Data", "Randomized Data"),lty=c(1,2),
                        bty="n",col=c(1,2))
                xx <- gapsize$IndGen[[order( - Gap)[1]]]
                print(xx)
                #write(xx, "GClabel", ncolumns = length(xx), append = T)
                #write(length(xx), "GClength", append = T)
                showclust(A[xx,  ], 2)
                variance <- round(Dk[Gap == max(Gap)], 3)
                vbetween <- round(VB[Gap == max(Gap)], 2)
                vtotal <- round(vbetween/variance, 2)
                vbvw <- round(Dk[Gap == max(Gap)]/(1 - Dk[Gap == max(Gap)]),
                        2)
                subtitl <- paste("%variance=", variance * 100, "VB/VW=", vbvw,
                        "VB=", vbetween, "VT=", vtotal)
                title(main = subtitl)
                clust <- clust + 1
                if(clust > NumClusts) {
                        break
                }
                xbar <- gapsize$z.bar[order( - Gap)[1],  ]
                CurrentMatrix <- cbind(CurrentMatrix, matrix(rep(xbar, N),
                        N, P, byrow = T))
                CurrentMatrix <- t(apply(CurrentMatrix, 1, lmfit))
                ##orth
                dimnames(CurrentMatrix) <- dimnames(A)
                #showclust(CurrentMatrix, 1)
                CurrentMatrix <- sweep(CurrentMatrix, 1, apply(CurrentMatrix,
                        1, mean))
        }
        return()
}
pcs.sup <-
function(z, Q, acc, alpha)
{
        R2 <- vb <- rep(0, acc)
        IndGen <- list(acc)
        z.bar <- matrix(0, acc, dim(z)[[2]])
        i <- 0
        repeat {
                i <- i + 1
                pca <- as.vector(svd(z %*% Q)$u[, 1])
                #tmp <- eigen(t(Q) %*% t(z) %*% z %*% Q)
                #pca <- as.vector(z %*% Q %*% tmp$vectors %*% diag(1/tmp$values^
                #       0.5)[, 1])
                genesign <- ifelse(pca > 0, 1, -1)
                z1 <- z * genesign
                pca <- abs(pca)
                if(length(pca) > 2) {
                        k <- length(pca) - floor(alpha * length(pca))
                }
                if(length(pca) == 2)
                        break
                z <- z[ - (order(pca)[1:k]),  ]
                z.sign <- z1[ - (order(pca)[1:k]),  ]
                z.bar[i,  ] <- apply(z.sign, 2, mean)
                d <- dim(z)
                IndGen[[i]] <- dimnames(z)[[1]]
                vb[i] <- ((d[[2]] - 1) * var(apply(z.sign, 2,mean)))/d[[2]]
                vw <- ((d[[1]] - 1) * mean(diag(var(z.sign))))/d[[1]]
                bw <- vb[i]/vw
                R2[i] <- bw/(1 + bw)
        }
        return(R2, vb, IndGen, z.bar)
}
super.id <-
function(y, lam)
{
        k <- max(y)
        x <- rep(0, k)
        for(i in 1:k) {
                x[i] <- length(y[y == i])
        }
        d <- 0
        Q <- matrix(rep(0, sum(x) * k), sum(x), k)
        Q[, 1] <- c(rep(1, x[1]), rep(0, (sum(x) - x[1])))/x[1]
        for(i in 2:k) {
                d <- d + x[i - 1]
                tmp <- (d + 1):(d + x[i])
                Q[tmp, i] <- rep(1, length(tmp))/x[i]
        }
        Q <- lam * Q + (1 - lam) * ifelse(Q != 0, 1, 0)
        Q <- Q^0.5
        return(Q)
}
####################################################################### 
# Finally, the function input() can read a data set (***.tsv, say) from 
# the working directory in Unix. For example, using
#
# > shave(input("data.tsv"),2,15)  #unsupervised   
# > super(input("data.tsv"),scan("data.clf"),0.5,2,15)#partial supervised
#
# to do gene-shaving for data.tsv (in the working directory in Unix).
# (Note: input() is not needed in Splus)
#######################################################################
input <-
function(x)
{
	dat <- read.table(as.character(x), header = T)
	dat
	name <- dat[,1]
	dat <- dat[,-1]
	dimnames(dat)[[1]] <- name
	return(dat)
}

shave(input("tests/data/alon2000unique.tsv"),2,15)
