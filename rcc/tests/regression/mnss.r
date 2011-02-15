source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

mnss<-function(mean2, mean.alt, mean = 0, sd1 = 1, sd2 = sd1)
{
  build.power.table<-function(disp2)
  {
    print(disp2)
  }
  build.power.table(sd2)
}

mnss(1001, 1)
