lgamma <- function(x).Internal(lgamma(x))

## This program is to compute the probability to stop/continue trials after some patients
## are evaluated
## Description: Two-arm trial, a total patients in each group are total1 and total2, respectively.
##              After n1 and n2 patients in two groups were treated espectively,
##              r1 and r2 patients in each group responsed. The left total1-n1 and total2-n2 patiens
##              are waiting for treatments in each group. Now the question is if we should stop or
##              continue the trial if the goal of study is testing H0:p1=p2 vs H1: p1>p2. Or what is 
##              probability that the trial will be positive(accept H1) if continuing the trial?
##  We use Bayesian method with Beta(a,b) prior.
#   Input parameter: group1:  total1 n1  r1     group2: total2 n2 r2 
#            Beta  prior1:  a1 b1             prior2: a2 b2
#            Type I error:  alpha
#  Output: prob: probablity that the trial is positive after having observed (r1, r2) responses.
#
# Example: Two-arm trial
#          Group 1: total patients total1=125, # of patients having been treated n1=80
#                   where r1=33 responsed, using uniform(0,1) prior(i.e. Beta(1,1)
#          Group 2: total patients total1=125, # of patients having been treated n1=81
#                   where r1=20 responsed, using uniform(0,1) prior(i.e. Beta(1,1)
#          Alpha=0.05 as significant level(Type I error)
#          > stoporcontfn(125,80,33,1,1,125,81,20,1,1,0.05)
#          > 0.9338538
#	Written By:  Ying Yang

stoporcontfn <- function(total1,n1,r1,a1,b1,total2,n2,r2,a2,b2,alpha) {

m1 <-total1-n1; m2 <-total2-n2
  
## Compute the hypothesis test matrix
reject <- matrix(0,nrow=m2+1,ncol=m1+1)
for ( j in 0:m2) {
	for ( i in 0:m1) {
		phat <- (i+j+r1+r2)/(total1+total2)
		if ( phat==1) reject[j+1,i+1] <- 0
		else {
			p1hat <- (i+r1)/total1; p2hat <- (j+r2)/total2
			ztest <- (p1hat-p2hat)/sqrt(phat*(1-phat)*(1/total1+1/total2))
			pvalue <-1-pnorm(ztest)
			if (pvalue<alpha) reject[j+1,i+1] <- 1
				else reject[j+1,i+1] <- 0
		}
	}
} 
cat (reject);

## Compute predictive probability conditional on the observed data
predprob1 <- rep(0,length(m1+1)) # predictive probability for group 1
for ( z in 0:m1) {
	k1 <-lgamma(m1+1)+lgamma(a1+b1+n1)+lgamma(r1+a1+z)+lgamma(m1+n1+b1-r1-z)-
	       lgamma(z+1)-lgamma(m1-z+1)-lgamma(r1+a1)-lgamma(n1-r1+b1)-lgamma(m1+n1+a1+b1)
	predprob1[z+1] <-exp(k1)
}
cat (predprob1);

predprob2 <- rep(0,length(m2+1)) # predictive probability for group 2
for ( z in 0:m2) {
	k1 <-lgamma(m2+1)+lgamma(a2+b2+n2)+lgamma(r2+a2+z)+lgamma(m2+n2+b2-r2-z)-
	       lgamma(z+1)-lgamma(m2-z+1)-lgamma(r2+a2)-lgamma(n2-r2+b2)-lgamma(m2+n2+a2+b2)
	predprob2[z+1] <-exp(k1)
}
cat (predprob2);

## Compute the probability of the final trial(if continue) will be positive after 
## observed(r1,r2) out of (n1,n2)
prob <- predprob2%*%reject%*%predprob1

return(prob)
}
Rprof("prof-output", interval=0.001)
for (unk in 1:1) stoporcontfn(1000,80,33,1,1,1001,81,20,1,1,0.05)
Rprof(NULL)
