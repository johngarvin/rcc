Box.test <- function (x, lag = 1, type=c("Box-Pierce", "Ljung-Box"))
{
    if (NCOL(x) > 1)
        stop ("x is not a vector or univariate time series")
    DNAME <- deparse(substitute(x))
    type <- match.arg(type)
    cor <- acf (x, lag.max = lag, plot = FALSE, na.action = na.pass)
    n <- sum(!is.na(x))
    PARAMETER <- lag
    obs <- cor$acf[2:(lag+1)]
    if (type=="Box-Pierce")
    {
        METHOD <- "Box-Pierce test"
        STATISTIC <- n*sum(obs^2)
        PVAL <- 1-pchisq(STATISTIC,lag)
    }
    else
    {
        METHOD <- "Box-Ljung test"
        STATISTIC <- n*(n+2)*sum(1/seq(n-1,n-lag)*obs^2)
        PVAL <- 1-pchisq(STATISTIC,lag)
    }
    names(STATISTIC) <- "X-squared"
    names(PARAMETER) <- "df"
    structure(list(statistic = STATISTIC,
                   parameter = PARAMETER,
                   p.value = PVAL,
                   method = METHOD,
                   data.name = DNAME),
              class = "htest")
}

PP.test <- function (x, lshort = TRUE)
{
    if (NCOL(x) > 1)
        stop ("x is not a vector or univariate time series")
    DNAME <- deparse(substitute(x))
    z <- embed (x, 2)
    yt <- z[,1]
    yt1 <- z[,2]
    n <- length (yt)
    tt <- (1:n)-n/2
    res <- lm (yt~1+tt+yt1)
    if (res$rank < 3)
        stop ("singularities in regression")
    res.sum <- summary (res)
    tstat <- (res.sum$coefficients[3,1]-1)/res.sum$coefficients[3,2]
    u <- residuals (res)
    ssqru <- sum(u^2)/n
    if (lshort)
        l <- trunc(4*(n/100)^0.25)
    else
        l <- trunc(12*(n/100)^0.25)
    ssqrtl <- .C ("R_pp_sum", as.vector(u,mode="double"), as.integer(n),
                  as.integer(l), trm=as.double(ssqru), PACKAGE="stats")
    ssqrtl <- ssqrtl$trm
    n2 <- n^2
    trm1 <- n2*(n2-1)*sum(yt1^2)/12
    trm2 <- n*sum(yt1*(1:n))^2
    trm3 <- n*(n+1)*sum(yt1*(1:n))*sum(yt1)
    trm4 <- (n*(n+1)*(2*n+1)*sum(yt1)^2)/6
    Dx <- trm1-trm2+trm3-trm4
    STAT <- sqrt(ssqru)/sqrt(ssqrtl)*tstat-(n^3)/(4*sqrt(3)*sqrt(Dx)*sqrt(ssqrtl))*(ssqrtl-ssqru)
    table <- cbind(c(4.38,4.15,4.04,3.99,3.98,3.96),
                   c(3.95,3.80,3.73,3.69,3.68,3.66),
                   c(3.60,3.50,3.45,3.43,3.42,3.41),
                   c(3.24,3.18,3.15,3.13,3.13,3.12),
                   c(1.14,1.19,1.22,1.23,1.24,1.25),
                   c(0.80,0.87,0.90,0.92,0.93,0.94),
                   c(0.50,0.58,0.62,0.64,0.65,0.66),
                   c(0.15,0.24,0.28,0.31,0.32,0.33))
    table <- -table
    tablen <- dim(table)[2]
    tableT <- c(25,50,100,250,500,100000)
    tablep <- c(0.01,0.025,0.05,0.10,0.90,0.95,0.975,0.99)
    tableipl <- numeric(tablen)
    for (i in (1:tablen))
        tableipl[i] <- approx (tableT,table[,i],n,rule=2)$y
    PVAL <- approx (tableipl,tablep,STAT,rule=2)$y
    PARAMETER <- l
    METHOD <- "Phillips-Perron Unit Root Test"
    names(STAT) <- "Dickey-Fuller"
    names(PARAMETER) <- "Truncation lag parameter"
    structure(list(statistic = STAT, parameter = PARAMETER,
                   p.value = PVAL, method = METHOD, data.name = DNAME),
              class = "htest")
}
