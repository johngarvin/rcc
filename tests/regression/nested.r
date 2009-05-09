source(file.path(Sys.getenv("RCC_R_INCLUDE_PATH"), "well_behaved.r"))

f<-function(x)
{
	g<-function()
        {
		if (x == 123) print("foo")
	}

   g()
   print("bar")
}

f(1)
