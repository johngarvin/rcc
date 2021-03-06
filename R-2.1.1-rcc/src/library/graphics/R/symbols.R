symbols <-
function (x, y = NULL, circles, squares, rectangles, stars,
	  thermometers, boxplots, inches = TRUE, add = FALSE,
	  fg = 1, bg = NA, xlab = NULL, ylab = NULL, main = NULL,
	  xlim=NULL, ylim=NULL, ...)
{
    count <- 0
    if (!missing(circles)) {
	count <- count + 1
	data <- circles
	type <- 1
    }
    if (!missing(squares)) {
	count <- count + 1
	data <- squares
	type <- 2
    }
    if (!missing(rectangles)) {
	count <- count + 1
	data <- rectangles
	type <- 3
    }
    if (!missing(stars)) {
	count <- count + 1
	data <- stars
	type <- 4
    }
    if (!missing(thermometers)) {
	count <- count + 1
	data <- thermometers
	type <- 5
    }
    if (!missing(boxplots)) {
	count <- count + 1
	data <- boxplots
	type <- 6
    }
    if (count != 1)
	stop("exactly one symbol type must be specified")
    xy <- xy.coords(x, y, xlab = deparse(substitute(x)),
                    ylab = deparse(substitute(y)))
    x <- xy$x; y <- xy$y
    if (!add) {
	if(is.null(xlab)) xlab <- xy$xlab
	if(is.null(ylab)) ylab <- xy$ylab
	if(is.null(xlim)) {
	    ## Expand the range by 20% : wild guess !
	    ## FIXME: better guess: use size of largest symbol...
	    ##	      really would need	 (x, y, type, data, inches) ->
	    ##	      rather an internal symbols.limits()
	    xlim <- range(x, na.rm = TRUE)
	    xlim <- xlim + c(-1, 1) * .10 * diff(xlim)
	}
	if(is.null(ylim)) {
	    ylim <- range(y, na.rm = TRUE)
	    ylim <- ylim + c(-1, 1) * .10 * diff(ylim)
	}
	plot(NA, NA, type="n", xlim=xlim, ylim=ylim,
	     xlab=xlab, ylab=ylab, main=main, ...)
    }
    .Internal(symbols(x, y, type, data, inches, bg, fg, ...))
}
