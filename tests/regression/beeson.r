source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

real <- 0
raw <- 0

set.seed(3)  # initialize RNG for reproducible results

TWOPI <<- 2*pi

dist <- function (x1,y1,x2,y2) {
  sqrt((x2-x1)^2+(y2-y1)^2)
}

deg_from_rad <- function(th) {
  th*180/pi
}

find_action_model <- function(num_models=1,num_burnin_samps=1000,num_final_samps=2000) {
  models <<- num_models
  burnin <<- num_burnin_samps
  final <<- num_final_samps
#  read_files()
#  align_data()
  create_fake_data()
  find_params()
  find_drift_error()
#  find_translate_error()
#  find_rotational_error()
}

read_files <-function() {
  skip <- 3
  
  raw <<- scan("raw_odo.data")
  
  ## sample every skip/3 timesteps
  raw <<- c(raw[seq(1,length(raw),by=skip)],
            raw[seq(2,length(raw),by=skip)],
            raw[seq(3,length(raw),by=skip)])

  dim(raw) <<- c(length(raw) / 3, 3)
  
  raw <<- t(raw)
  
  real <<- scan("corrected_odo.data")

  real <<- c(real[seq(1,length(real),by=skip)],
             real[seq(2,length(real),by=skip)],
             real[seq(3,length(real),by=skip)])
  
  if (length(real) != length(raw))
    print ("ERROR: Data files not the same dimension")
  
  dim(real) <<- c(length(raw) / 3,3)

  real <<- t(real)
  
}

find_transform <- function(c1,c2) {

  rotation <- c2[3] - c1[3]
  
  cs <- cos(rotation)
  sn <- sin(rotation)
  
  rotated_x <- c1[1] * cs + c1[2] * -sn
  rotated_y <- c1[1] * sn + c1[2] * cs

  offset_x <- c2[1] - rotated_x
  offset_y <- c2[2] - rotated_y

  t <- c(offset_x,offset_y,rotation)
}

apply_transform <- function(t, c1) {
  
  rotation <- t[3]

  cs <- cos(rotation)
  sn <- sin(rotation)
  rotated_x <- c1[1] * cs + c1[2] * -sn
  rotated_y <- c1[1] * sn + c1[2] * cs
  
  c2 <- c(rotated_x + t[1],
          rotated_y + t[2],
          c1[3]+rotation)  
}

align_data <- function() {
  sz <- length(raw[1,])

  new_raw <- real[,1]
  
  
  for (i in 2:sz) {
    dim(new_raw) <- c(3,length(new_raw)/3)
    t<-find_transform(raw[,i-1],real[,i-1])
    new_raw <- append(new_raw,
                      new_raw[,i-1]+
                      apply_transform(t,raw[,i]))
  }

  raw <<- new_raw
  dim(raw) <<- c(3,sz)
}

create_fake_data <- function() {
  raw <<- c(0,0,0)
  
  dim(raw) <<- c(3,1)
  index <- 1
  for (i in 1:5000) {
    forward <- runif(1,5,10)
    theta <- runif(1,-30*pi/180,30*pi/180)

    theta2 <- 0
    raw <<- append(raw,c(raw[1,index]+(forward*cos(theta+raw[3,index])),
                         raw[2,index]+(forward*sin(theta+raw[3,index])),
                         theta_normalize(raw[3,index]+theta+theta2)))
    index <- index +1
    dim(raw) <<- c(3,index)
  }

  c1 <- 0.2
  l11 <- 0.5
  l21 <- 0
  d1 <- 0
  k11 <- k21 <- 0
  
  c2 <- l12 <- l22 <- 0
  d2 <- k12 <- k22 <- 0
  
  c3 <- l13 <- l23 <- l33 <- 0
  d3 <- k13 <- k23 <- k33 <- 0
  
  sz<-length(raw[1,])
  
  delta_x  <- raw[1,2:sz] - raw[1,1:(sz-1)]
  delta_y  <- raw[2,2:sz] - raw[2,1:(sz-1)]
  delta_th <- raw[3,2:sz] - raw[3,1:(sz-1)]
  
  rho <- dist(0,0,delta_x,delta_y)
  
  alpha <- atan2(delta_y,delta_x) - raw[3,1:(sz-1)]
  alpha <- theta_normalize(alpha * (rho != 0))
  
  large_turn <- (alpha > pi/2)
  alpha <- alpha - pi*(large_turn)
  rho <- rho - 2*rho*(large_turn)

  large_turn <- (alpha < -pi/2)
  alpha <- alpha + pi*(large_turn)
  rho <- rho - 2*rho*(large_turn)
  
  beta <- theta_normalize(delta_th-alpha)

  drift_err       <- rnorm(sz-1, d1 + k11 * alpha + k21 * rho,
                           sqrt(c1^2 + (l11 * alpha)^2 +
                                (l21 * rho)^2))
  
  translation_err <- rnorm(sz-1, d2 + k12 * alpha + k22 * rho,
                           sqrt(c2^2 + (l12 * alpha)^2 +
                                (l22 * rho)^2))
  
  turn_err        <- rnorm(sz-1, d3 + k13 * alpha +
                           k23 * rho + k33 * beta,
                           sqrt(c3^2 + (l13 * alpha)^2 +
                                (l23 * rho)^2 + (l33 * beta)^2))
  
  real_x  <- raw[1,1]
  real_y  <- raw[2,1]
  real_th <- raw[3,1]
  
  for (i in 1:(sz-1)) {
    
    real_x <- append(real_x,
                     real_x[i] + ((rho[i] + translation_err[i]) *
                                  cos(real_th[i] + alpha[i] + drift_err[i])))
    
    real_y  <- append(real_y,
                      real_y[i] + ((rho[i] + translation_err[i]) *
                                   sin(real_th[i] + alpha[i] + drift_err[i])))
    real_th <- append(real_th,
                      theta_normalize(real_th[i] + delta_th[i] + turn_err[i]))
  }
  
  real <<- c(real_x,real_y,real_th)
  dim(real) <<- c(length(real)/3,3)
  real <<- t(real)
  
}

theta_normalize <- function(th) {
  while (max(th > pi))
    th <- th - TWOPI * (th > pi)
  while (max(th < -pi))
    th <- th + TWOPI * (th < -pi)
  th
}

get_params <- function(data) {
  
  sz<-length(data[1,])

  delta_x  <- data[1,2:sz] - data[1,1:(sz-1)]
  delta_y  <- data[2,2:sz] - data[2,1:(sz-1)]
  delta_th <- data[3,2:sz] - data[3,1:(sz-1)]
  
  rho <- dist(0,0,delta_x,delta_y)
  
  alpha <- atan2(delta_y,delta_x) - data[3,1:(sz-1)]
  alpha <- theta_normalize(alpha * (rho != 0))
  
  large_turn <- (alpha > pi/2)
  alpha <- alpha - pi*(large_turn)
  rho <- rho - 2*rho*(large_turn)
  
  large_turn <- (alpha < -pi/2)
  alpha <- alpha + pi*(large_turn)
  rho <- rho - 2*rho*(large_turn)
  
  beta <- theta_normalize(delta_th-alpha)
  
  z <- c(alpha,rho,beta)
  dim(z) <- c(sz-1,3)
  z <- t(z)
}

find_params <- function() {
  raw_params <<- get_params(raw)
  real_params <<- get_params(real)
}

find_drift_error <- function() {
  print("Drift")
  print ("Param 1: alpha coefficient (mean)")
  print ("Param 2: rho coefficient (mean)")
  print ("Param 3: constant (mean)")
  print ("Param 4: alpha coefficient (variance)")
  print ("Param 5: rho coefficient (variance)")
  print ("Param 6: constant (variance)")

  inp_err <- theta_normalize(raw_params[1,]-real_params[1,])
  drift <<- samplen(inp=raw_params[1:2,],err=inp_err,n=burnin)
}


samplen <- function(inp,err,n=5000,loop=1) {
  if (loop == 1) {
    num_params <<-(dim(inp)[1]+1)*2
    inputs <<- inp
    errors <<- err
    new_params <- NULL
    params <<- NULL
    for (i in 1:(num_params/2))
      new_params <- append(new_params,0)
    for (i in 1:(num_params/2))
      new_params <- append(new_params,1)
    for (i in 1:models)
      params <<- append(params,new_params)

    dim(params) <<- c(length(params)/models,models)
    params <<- t(params)
  }
  
  param_hits <<- NULL
  
  accepts <<-0
  
  for (i in 1:n) {
    if (i%%1000 == 0)
      print(i)
    sample1(i)
  }

  print("Round")
  print(loop)

  new_params <- NULL
  
  for (j in 1:models) {
    print('MODEL')
    print (j)
    for (i in 1:num_params) {
      print('Param')
      print(i)
      q<-quantile(param_hits[j,i,],c(0.1,0.9))
      tmp <- param_hits[j,i,][param_hits[j,i,]>=q[1] & param_hits[j,i,]<=q[2]]
      new_params <- append(new_params,median(tmp))
      print(median(tmp))
    }
  }
  dim(new_params) <- c(num_params,models)
  params <<- t(new_params)

  p <- evaluate_params(errors,params)

  print('Likelihood')
  print(p[1])

  print('Model Priors')
  print(p[2:(1+models)])

  print('Model Weights')
  print(p[(2+models):length(p)])
  
  print('Accept percentage')
  print(accepts/n)

  #First loop is burnin to get near the correct answer.
  #Second loop finds the correct answers.
  if (loop < 2) 
    samplen(n=final,loop=2)
  print("")
  param_hits
}


sample1 <- function(j) {
  
  new_params <- params

  index <- j  %% (num_params * models)
  
  model_index <- (index %/% num_params) + 1
  param_index <- (index %% num_params) + 1

  if (param_index <= num_params/2)
    new_params[model_index,param_index] <- params[model_index,param_index] +
      runif(1,-0.01,0.01)
  else {
    new_params[model_index,param_index] <- params[model_index,param_index] +
      runif(1,-0.05,0.05)
    while (new_params[model_index,param_index] < 0) #variance is positive
      new_params[model_index,param_index] <- params[model_index,param_index] +
        runif(1,-0.05,0.05)
  }

  p1 <- evaluate_params(errors,params)
  p2 <- evaluate_params(errors,new_params)
  
  p1 <- p1[1] + sum(p1[2:(models+1)])
  p2 <- p2[1] + sum(p2[2:(models+1)])

  diff <- min(0,p2-p1)
  if (is.nan(diff))
    diff <- 0 #Catches -Inf+Inf = NaN

  
  u <- log(runif(1,0,1))
  
  if (u < diff) {
    params <<- new_params
    accepts <<- accepts+1
  }
  param_hits <<- append(param_hits,params)

  dim(param_hits) <<- c(models,num_params,
                        length(param_hits)/(num_params*models))
}

evaluate_params <- function(errors,params) {
  old_mus <- NULL
  old_vars <- NULL
  
  
  for (j in 1:models) {
    mu1 <- 0
    for (i in 1:(num_params/2))
      if (i < num_params/2)  {
        mu1 <- mu1 + params[j,i] * inputs[i,]
      } else {
        mu1 <- mu1 + params[j,i]
      }
    old_mus <- append(old_mus,mu1)
    
    var1 <- 0
    
    index <- 1+(num_params/2)
    
    for (i in index:num_params) 
      if (i < num_params) {
        var1 <- var1 + (params[j,i]     * inputs[i-(num_params/2),])^2
      } else {
        var1 <- var1 + (params[j,i])^2
      }
    old_vars <- append(old_vars,var1)
  }


  dim(old_mus) <- c(length(old_mus)/models,models)
  dim(old_vars) <- dim(old_mus)

  old_mus <- t(old_mus)
  old_vars <- t(old_vars)

  old_stds <- sqrt(old_vars)

  old_probs <- NULL

  old_sum <- 0
  
  for (j in 1:models) {
    
    old_prob<-dnorm(errors,old_mus[j,],old_stds[j,])

    old_sum <- old_sum + old_prob
    
    old_probs <- append(old_probs,old_prob)
  }

  dim(old_probs) <- c(length(old_prob),models)
  old_weights <- old_probs / old_sum

  #catches zero sum
  old_weights[!is.finite(old_weights)] <- 1/models
  
  old_probs <- t(old_probs)
  old_weights <-t(old_weights) 
  
  old_probs <- old_probs * old_weights

  old_sum <- 0
  
  for (j in 1:models) {
    old_sum <- old_sum + old_probs[j,]
  }
  
    
  ret <- sum(log(old_sum))
  
  for (j in 1:models) {
    ret <- append(ret, -
                  0) # Flat prior
  }

  
  weights <- (1:models)*0
  
  for (j in 1:models)
    weights[j] <- sum(old_weights[j,])
  
  weights <- weights / sum(weights)


  
  ret <- append(ret,weights)
}

find_action_model(1,10,20)
