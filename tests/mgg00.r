#######
#
# Gibbs sampling for the one dimensional mixture model.
#
# Model f(Z) = p0 * f0(Z) + p1 * f1(Z)
# We assume different base measure for f0 and f1
#
#######

myRgamma <- function (n, alpha, beta) {
    return (rgamma(n, alpha, beta))
}


geneInits <- list(mustar.rstar = cbind(c(0, -2, 2), c(0, 1, 1)),
                  ss = c(rep(1, 720), rep(2, 40), rep(3, 40)),
                  r  = c(rep(0, 720), rep(1, 80)),
                  p0 = 0.8,
                  p0.lb = 0.2,
                  p0.ub = 0.99999,
                  alpha = 2,
                  beta  = 2,
                  sigmaSsquare = 1,
                  as = 2,
                  bs = 1,
                  b  = 0.0,
                  sigmaBsquare  = 1,
                  ab = 1,
                  bb = 0.2,
                  b1 = 1.0,
                  sigmaBsquare1 = 1,
                  ab1 = 1,
                  bb1 = 0.2,
                  b0 = 0,
                  b1.1 = 1.0,
                  sigmaCsquare = 16,
                  M = 1,
                  am = 2,
                  bm = 0.5)
 

#Zscore <- c(rnorm(320, 0, 1), rnorm(40, -2, 1), rnorm(40, 2, 1))
#zscore <- rnorm(400, 0, 1)
#zZscore <- c(zscore, Zscore)
#source("mixGibbs.s") mixGibbs.s is the name of s file.
#simSim1 <- mixGibbsGene.fun(10000, zZscore, 400, 400, geneInits)
#
#
# Input of function mixGibbsGene.fun: nsim:      number of simulation,
#                                     zZscore:   combined low z and cap z score
#                                     n, N:      sample size of low z and cap z respectively
#                                     geneInits: Initial value for all the hyperparameters
#
mixGibbsGene.fun <- function(nsim, zZscore, n, N, inits) {

  mustar.rstar <- inits$mustar.rstar
  mu.star      <- mustar.rstar[,1]
  r.star       <- mustar.rstar[,2]
  ss           <- inits$ss
  r            <- inits$r
  
  p0    <- inits$p0
  p0.lb <- inits$p0.lb
  p0.ub <- inits$p0.ub
  alpha <- inits$alpha
  beta  <- inits$beta

  sigmaSsquare <- inits$sigmaSsquare
  as  <- inits$as
  bs  <- inits$bs
  
  b  <- inits$b
  sigmaBsquare  <- inits$sigmaBsquare
  ab  <- inits$ab
  bb  <- inits$bb
  b1  <- inits$b1
  sigmaBsquare1 <- inits$sigmaBsquare1
  ab1 <- inits$ab
  bb1 <- inits$bb
  b0  <- inits$b0
  b1.1<- inits$b1.1
  sigmaCsquare <- inits$sigmaCsquare
  
  M  <- inits$M
  am <- inits$am
  bm <- inits$bm

  kk <- length(mu.star)
  nj <- rep(0, kk)
  for (j in 1:kk) {
    nj[j] <- sum(ss == j)
  }

  kk0 <- sum(mustar.rstar[,2]==0)                     #kk0 stands for the number of clusters with rstar = 0
  kk1 <- kk - kk0                                     #kk1 stands for the number of clusters with rstar = 1

  ######  simulation storage  ######
 
  p0.vec  <- vector("numeric", nsim)
  alpha.vec <- vector("numeric", nsim)
  beta.vec  <- vector("numeric", nsim)
  kk0.vec <- vector("numeric", nsim)
  kk.vec  <- vector("numeric", nsim)
  M.vec   <- vector("numeric", nsim)
  r.mat   <- matrix(0, nsim, length(zZscore))
  sigmaSsquare.vec  <- vector("numeric", nsim)
  sigmaBsquare.vec  <- vector("numeric", nsim)
  sigmaBsquare1.vec <- vector("numeric", nsim)
  b.vec             <- vector("numeric", nsim)
  b1.vec            <- vector("numeric", nsim)
  mustar.rstar.list <- vector("list",    nsim)

  ######  end of simulation storage  #####

  ######  start simulation  #####

  for (isim in 1:nsim) {
    cat("isim = ", isim, "\n")

    ####         1. First to simulate the indicator vector ss, r    ####

    mu.ss.sim <- sim.ss.fun(isim, p0, mu.star, r.star, r, ss, kk0, kk, nj, M, zZscore, b, b1, sigmaBsquare, sigmaBsquare1, sigmaSsquare, n, N)
    mu.star   <- mu.ss.sim$mu.star
    r.star    <- mu.ss.sim$r.star
    ss        <- mu.ss.sim$ss
    r         <- mu.ss.sim$r
    nj        <- mu.ss.sim$nj
    kk0       <- mu.ss.sim$kk0
    kk        <- mu.ss.sim$kk

    kk0.vec[isim] <- kk0
    kk.vec[isim]  <- kk
    r.mat[isim,]  <- r
    cat("kk0, kk is ", kk0, kk, "\n")
     
    ####         2. Second step is to simulate p0                   ####
    p0.sim <- sim.p0.fun(alpha, beta, r, n, N, p0.lb, p0.ub)
    p0.vec[isim] <- p0.sim$p0
    alpha.vec[isim] <- p0.sim$alpha
    beta.vec[isim]  <- p0.sim$beta
    
    ####         3. Third step is to simulate mu.star              ####

    mu.star <- sim.mustar.fun(mu.star, r.star, ss, kk0, kk, nj, zZscore, b, b1, sigmaBsquare, sigmaBsquare1, sigmaSsquare)
    mustar.rstar.list[[isim]] <- cbind(mu.star, r.star, nj)
    
    ####         4. 4th step to simulate sigmaSsquare               ####
    sigmaSsquare           <- sim.sigmaSsquare.fun(zZscore, mu.star, ss, kk, as, bs)
    sigmaSsquare.vec[isim] <- sigmaSsquare

    ####         5. 5th step to simulate b
    b <- sim.b.fun(mu.star, kk0, sigmaBsquare, b0, sigmaCsquare)
    b.vec[isim] <- b

    ####         6. 6th step to simulate sigmaBsquare
    sigmaBsquare           <- sim.sigmaBsquare.fun(mu.star, kk0, b, ab, bb)
    sigmaBsquare.vec[isim] <- sigmaBsquare

    ####         7. 7th step to simulate indicator II
    II <- rep(2, (kk-kk0))
    II <- sim.II.fun(mu.star, kk0, kk, b1, sigmaBsquare1)
    
    ####         8. 8th step to simulate b1    
    b1 <- sim.b1.fun(mu.star, kk0, kk, II, sigmaBsquare1, b1.1, sigmaCsquare)
    b1.vec[isim] <- b1
    
    ####         9. 9th step to simulate sigmaBsquare1
    sigmaBsquare1           <- sim.sigmaBsquare1.fun(mu.star, kk0, kk, II, b1, ab1, bb1)
    sigmaBsquare1.vec[isim] <- sigmaBsquare1
    
    ####         9. 9th step to simulate M
    #M <- sim.M.fun(M, ss, kk, am, bm)
    M.vec[isim] <- M
  }
  return(p0=p0.vec, alpha = alpha.vec, beta = beta.vec, mustar.rstar.list=mustar.rstar.list, r = r.mat, kk0 = kk0.vec, kk = kk.vec, sigmaSsquare = sigmaSsquare.vec, b = b.vec, b1 = b1.vec, sigmaBsquare = sigmaBsquare.vec, sigmaBsquare1 = sigmaBsquare1.vec, M = M.vec)
}

sim.ss.fun <- function(isim, p0, mu.star, r.star, r, ss, kk0, kk, nj, M, zZscore, b, b1, sigmaBsquare, sigmaBsquare1, sigmaSsquare, n, N) {
  tauB <- 1/sigmaBsquare
  tauB1 <- 1/sigmaBsquare1
  tauS <- 1/sigmaSsquare
  tempSigmaSquare <- 1/(tauB + tauS)
  tempSigmaSquare1 <- 1/(tauB1 + tauS)
  sigmaTemp  <- sigmaBsquare + sigmaSsquare
  sigmaTemp1 <- sigmaBsquare1 + sigmaSsquare
  
  p1 <- (1 - p0)
  
  for (i in 1:(n+N)) {
    if ((nj[ss[i]] - 1) == 0) {
      if (r[i] == 0) {
        mu.star[ss[i]] <- mu.star[kk0]
        r.star[ss[i]]  <- r.star[kk0]
        ss[ss == kk0]  <- ss[i]
        nj[ss[i]]      <- nj[kk0]
        mu.star        <- mu.star[-kk0]
        r.star         <- r.star[-kk0]
        nj             <- nj[-kk0]
        ss[r==1]       <- ss[r==1] - 1
        kk0            <- kk0 - 1
        kk             <- kk - 1
      } else {
        mu.star[ss[i]] <- mu.star[kk]
        r.star[ss[i]]  <- r.star[kk]
        ss[ss == kk]   <- ss[i]
        nj[ss[i]]      <- nj[kk]
        mu.star        <- mu.star[-kk]
        r.star         <- r.star[-kk]
        nj             <- nj[-kk]
        kk             <- kk - 1
      }
    } else {
      nj[ss[i]] <- nj[ss[i]] - 1
    }

    
    if (1 <= i && i <= n) { ### 1 <= i <= n

      n0       <- sum(r==0)
      n0.minus <- n0-1
      pMnomial <- vector("numeric", kk0 + 1)

      for (j in 1:kk0) {
        pMnomial[j] <- nj[j]/(M+n0.minus) * dnorm(zZscore[i], mu.star[j], sqrt(sigmaSsquare), log = FALSE)
      }
      pMnomial[kk0+1] <- M/(M+n0.minus) * dnorm(zZscore[i], b, sqrt(sigmaTemp), log = FALSE)
      pMnomial <- pMnomial/sum(pMnomial)
      ss[i]    <- sample(c(1:(kk0 + 1)), 1, replace = F, prob = pMnomial)

      if (ss[i] == kk0+1) {
        tempMean <- tempSigmaSquare * (tauB * b + tauS * zZscore[i])
        newMu    <- rnorm(1, tempMean, sqrt(tempSigmaSquare))
        mu.star  <- c(mu.star[1:kk0], newMu, mu.star[(kk0+1):kk])
        r.star   <- c(r.star[1:kk0], 0, r.star[(kk0+1):kk])
        nj       <- c(nj[1:kk0], 1, nj[(kk0+1):kk])
        kk0      <- kk0 + 1
        kk       <- kk  + 1
        ss[r==1] <- ss[r==1] + 1
      } else {
        nj[ss[i]] <- nj[ss[i]] + 1
      }

    } else { ### (n+1) <= i <= (n+N)
      n0 <- sum(r == 0)
      n1 <- sum(r == 1)
      if (r[i] == 0) {
        n0.minus <- n0 - 1
        n1.minus <- n1
      } else {
        n0.minus <- n0
        n1.minus <- n1 - 1
      }

      pMnomial               <- vector("numeric", kk+2)
      pMnomial[1:kk0]        <- p0 * nj[1:kk0]/(M+n0.minus) * dnorm(zZscore[i], mu.star[1:kk0], sqrt(sigmaSsquare), log = FALSE)
      pMnomial[(kk0+1):(kk)] <- p1 * nj[(kk0+1):kk] /(M+n1.minus) * dnorm(zZscore[i], mu.star[(kk0+1):kk], sqrt(sigmaSsquare), log = FALSE)
      pMnomial[kk + 1]       <- p0 * M/(M+n0.minus) * dnorm(zZscore[i], b, sqrt(sigmaTemp), log = FALSE)
      pMnomial[kk + 2]       <- p1 * M/(M+n1.minus) * 1/2.0 * (dnorm(zZscore[i], b1, sqrt(sigmaTemp1)) + dnorm(zZscore[i], b1, sqrt(sigmaTemp1)))
      pMnomial <- pMnomial/sum(pMnomial)
      ss[i] <- sample(c(1:(kk+2)), 1, replace = F, prob = pMnomial)

      if (ss[i] <= kk0) {
        r[i] <- 0
        nj[ss[i]] <- nj[ss[i]] + 1
      }

      if ((kk0 < ss[i]) && (ss[i] <= kk)) {
        r[i] <- 1
        nj[ss[i]] <- nj[ss[i]] + 1
      }
      
      if (ss[i] == (kk+1)) {
        r[i]     <- 0
        ss[i]    <- kk0 + 1
        ss[r==1] <- ss[r==1] + 1
        tempMean <- tempSigmaSquare * (tauB * b + tauS * zZscore[i])
        newMu    <- rnorm(1, tempMean, sqrt(tempSigmaSquare))
        mu.star  <- c(mu.star[1:kk0], newMu, mu.star[(kk0+1):kk])
        r.star   <- c(r.star[1:kk0], 0, r.star[(kk0+1):kk])
        nj       <- c(nj[1:kk0], 1, nj[(kk0+1):kk])
        kk0      <- kk0 + 1
        kk       <- kk + 1
      }
      
      if (ss[i] == (kk+2)) {
        r[i]     <- 1
        ss[i]    <- kk + 1
        tempMean1 <- tempSigmaSquare1 * (tauB1 * (-b1) + tauS * zZscore[i])
        tempMean2 <- tempSigmaSquare1 * (tauB1 * (b1) + tauS * zZscore[i])
        w1 <- dnorm(zZscore[i], -b1, sqrt(sigmaTemp1))
        w2 <- dnorm(zZscore[i], b1, sqrt(sigmaTemp1))
        Indicator <- rbinom(1, 1, w1/(w1+w2))
        if (Indicator ==1) {
          newMu <- rnorm(1, tempMean1, sqrt(tempSigmaSquare1))
        } else {
          newMu <- rnorm(1, tempMean2, sqrt(tempSigmaSquare1))
        }
        mu.star  <- c(mu.star, newMu)
        r.star   <- c(r.star, 1)
        nj       <- c(nj, 1)
        kk       <- kk + 1
      }
    }
  }
  cat("finish simulating ss \n")
  return(mu.star, r.star, r, ss, kk0, kk, nj)
}

sim.p0.fun <- function(alpha, beta, r, n, N, p0.lb, p0.ub) {
  #p0.lb and p0.ub stand for lower and upper bound of p0

  # The posterior of p0 is a truncated beta,
  # we will use inverse CDF method to simulate p0

  m0 <- sum(r[(n+1):(n+N)] == 0)
  m1 <- sum(r[(n+1):(n+N)] == 1)
  
  new.alpha <- alpha + m0
  new.beta  <- beta  + m1

  pbeta.lb <- pbeta(p0.lb, new.alpha, new.beta)
  pbeta.ub <- pbeta(p0.ub, new.alpha, new.beta)
  newpbeta <- runif(1, pbeta.lb, pbeta.ub)

  p0.new <- qbeta(newpbeta, new.alpha, new.beta)
  cat("finish simulating p0", p0.new, "\n")
  return(p0 = p0.new, alpha = new.alpha, beta = new.beta)
}

sim.mustar.fun <- function(mu.star, r.star, ss, kk0, kk, nj, zZscore, b, b1, sigmaBsquare, sigmaBsquare1, sigmaSsquare) {

  tauB  <- 1/sigmaBsquare
  tauB1 <- 1/sigmaBsquare1
  tauS  <- 1/sigmaSsquare
    
  for (j in 1:kk) {
    if (r.star[j] == 0) {
      SjInv <- tauB + nj[j] * tauS
      Sj    <- 1/(SjInv)
      meanj <- Sj * (tauB * b + tauS * sum(zZscore[ss==j]))
      mu.star[j] <- rnorm(1, meanj, sqrt(Sj))
    } else {
      SjInv1 <- tauB1 + nj[j] * tauS
      Sj1    <- 1/SjInv1
      zZmean <- mean(zZscore[ss==j])
      meanj1 <- Sj1 * (tauB1 * (-b1) + tauS * nj[j] * zZmean)
      meanj2 <- Sj1 * (tauB1 * b1    + tauS * nj[j] * zZmean)
      w1     <- dnorm(zZmean, -b1, sqrt(sigmaBsquare1 + 1/nj[j] * sigmaSsquare))
      w2     <- dnorm(zZmean,  b1, sqrt(sigmaBsquare1 + 1/nj[j] * sigmaSsquare))
      Indicator <- rbinom(1, 1, w1/(w1+w2))
      if (Indicator == 1) {
        mu.star[j] <- rnorm(1, meanj1, sqrt(Sj1))
      } else {
        mu.star[j] <- rnorm(1, meanj2, sqrt(Sj1))
      }
    }
  }
  cat("finish simulating mustar \n")
  return(mu.star)
}

sim.sigmaSsquare.fun <- function(zZscore, mu.star, ss, kk, as, bs) {
  
  ngene <- length(zZscore)
  as.new <- as + 1/2 * ngene

  temp <- 0

  for (i in 1:ngene) {
    temp <- temp + (zZscore[i] - mu.star[ss[i]])^2
    }
  bs.new <- bs + temp/2
  tempTauS <- myRgamma(1, as.new, bs.new)
  sigmaSsquare <- 1/tempTauS

  cat("finish simulating sigmaSsquare \n")
  return(sigmaSsquare)
}

sim.b.fun <- function(mu.star, kk0, sigmaBsquare, b0, sigmaCsquare) { 
  tauC <- 1/sigmaCsquare
  tauB <- 1/sigmaBsquare
  tempSigma <- 1/(tauC + kk0 * tauB)
  tempMean  <- tempSigma * (tauC * b0 + kk0 * tauB * mean(mu.star[1:kk0]))
  b <- rnorm(1, tempMean, sqrt(tempSigma))
  cat("finish simulating b", b, "\n")
  return(b)
}

sim.sigmaBsquare.fun <- function(mu.star, kk0, b, ab, bb) {

  ab.new <- ab + kk0/2
  temp <- (sum((mu.star[1:kk0] - b)^2))/2
  bb.new <- bb + temp
  tempTauB <- myRgamma(1, ab.new, bb.new)
  sigmaBsquare <- 1 / tempTauB

  cat("finish simulating sigmaBsquare", sigmaBsquare, "\n")
  return(sigmaBsquare)
}

sim.II.fun <- function(mu.star, kk0, kk, b1, sigmaBsquare1) {
  
  kk1 <- (kk - kk0)

  prob     <- matrix(0, kk1, 2)
  prob[,1] <- dnorm(mu.star[(kk0+1):kk], -b1, sqrt(sigmaBsquare1))
  prob[,2] <- dnorm(mu.star[(kk0+1):kk],  b1, sqrt(sigmaBsquare1))
  prob[,1] <- prob[,1] /(prob[,1] + prob[,2])
  prob[,2] <- 1 - prob[,1]

  II <- rep(2, kk1)
  for (i in 1:kk1) {
    II[i] <- rbinom(1, 1, prob[i,2])
    if (II[i] == 0) {
      II[i] <- -1
    }
  }
  cat("finish simulating II \n")
  return(II)
}

  
sim.b1.fun <- function(mu.star, kk0, kk, II, sigmaBsquare1, b1.1, sigmaCsquare) { #prior b1 ~ N(b1.1, sigmaCsquare)I(b1 > 0)
  
  tauC      <- 1/sigmaCsquare
  tauB1     <- 1/sigmaBsquare1
  tempTau   <- tauC + t(II) %*% II * tauB1
  tempSigma <- 1/tempTau
  tempMean  <- tempSigma * (t(II) %*% mu.star[(kk0 + 1):kk] * tauB1 + tauC * b1.1)
  b1 <- rnorm(1, tempMean, sqrt(tempSigma))
  
  cat("finish simulating b1", b1, "\n")
  return(b1)
}


sim.sigmaBsquare1.fun <- function(mu.star, kk0, kk, II, b1, ab1, bb1) {

  kk1 <- kk - kk0
  ab1.new <- ab1 + kk1 /2

  temp1 <- (sum((mu.star[(kk0+1):kk] - II * b1)^2))/2
  bb1.new <- bb1 + temp1
  tempTauB1 <- myRgamma(1, ab1.new, bb1.new)
  sigmaBsquare1 <- 1/tempTauB1
  cat("finish simulating sigmaBsquare1", sigmaBsquare1, "\n")
  return(sigmaBsquare1)  
}

sim.M.fun <- function(M, ss, kk, am, bm) {
  ngene <- length(ss)
  eta <- rbeta(1, M + 1, ngene)
  prob.pi <- (am + kk - 1)/(am + kk - 1 + ngene * (bm - log(eta)))
  
  bm.new  <- bm - log(eta)
  randBer <- rbinom(1, 1, prob.pi)
  if (randBer == 1) {
    am.new <- am + kk
  } else {
    am.new <- am + kk - 1
  }
   
  M <- myRgamma(1, am.new, bm.new)
  cat("finish simulating M \n")
  return(M)
}


Zscore <- c(rnorm(320,0,1),rnorm(40,-2,1), rnorm(40,2,1))
zscore <- rnorm(400,0,1)
zZscore <- c(zscore,Zscore)

Rprof("sim00.out")
simSim1 <- mixGibbsGene.fun (30, zZscore, 400, 400, geneInits)
Rprof(NULL)

source("analyze.r")

#####

rnorm <- function(n, mean=0, sd=1) .Internal(rnorm(n, mean, sd))
