/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2001-3 Paul Murrell
 *                2003 The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  A copy of the GNU General Public License is available via WWW at
 *  http://www.gnu.org/copyleft/gpl.html.  You can also obtain it by
 *  writing to the Free Software Foundation, Inc., 59 Temple Place,
 *  Suite 330, Boston, MA  02111-1307  USA.
 */

#include "grid.h"

extern int gridRegisterIndex;

/* Some access methods for gpars */
SEXP gpFontSizeSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_FONTSIZE);
}

double gpFontSize(SEXP gp, int i) {
    SEXP fontsize = gpFontSizeSXP(gp);
    return REAL(fontsize)[i % LENGTH(fontsize)];
}

SEXP gpLineHeightSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LINEHEIGHT);
}

double gpLineHeight(SEXP gp, int i) {
    SEXP lineheight = gpLineHeightSXP(gp);
    return REAL(lineheight)[i % LENGTH(lineheight)];
}

int gpCol(SEXP gp, int i) {
    SEXP col = VECTOR_ELT(gp, GP_COL);
    int result;
    if (isNull(col))
	result = R_TRANWHITE;
    else
	result = RGBpar(col, i % LENGTH(col));
    return result;
}

SEXP gpFillSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_FILL);
}

int gpFill(SEXP gp, int i) {
    SEXP fill = gpFillSXP(gp);
    int result;
    if (isNull(fill))
	result = R_TRANWHITE;
    else
	result = RGBpar(fill, i % LENGTH(fill));
    return result;
}

SEXP gpGammaSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_GAMMA);
}

double gpGamma(SEXP gp, int i) {
    SEXP gamma = gpGammaSXP(gp);
    return REAL(gamma)[i % LENGTH(gamma)];
}

SEXP gpLineTypeSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LTY);
}

int gpLineType(SEXP gp, int i) {
    SEXP linetype = gpLineTypeSXP(gp);
    return LTYpar(linetype, i % LENGTH(linetype));
}

SEXP gpLineWidthSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LWD);
}

double gpLineWidth(SEXP gp, int i) {
    SEXP linewidth = gpLineWidthSXP(gp);
    return REAL(linewidth)[i % LENGTH(linewidth)];
}

SEXP gpCexSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_CEX);
}

double gpCex(SEXP gp, int i) {
    SEXP cex = gpCexSXP(gp);
    return REAL(cex)[i % LENGTH(cex)];
}

SEXP gpFontSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_FONT);
}

int gpFont(SEXP gp, int i) {
    SEXP font = gpFontSXP(gp);
    return INTEGER(font)[i % LENGTH(font)];
}

SEXP gpFontFamilySXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_FONTFAMILY);
}

char* gpFontFamily(SEXP gp, int i) {
    SEXP fontfamily = gpFontFamilySXP(gp);
    return CHAR(STRING_ELT(fontfamily, i % LENGTH(fontfamily)));
}

SEXP gpAlphaSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_ALPHA);
}

double gpAlpha(SEXP gp, int i) {
    SEXP alpha = gpAlphaSXP(gp);
    return REAL(alpha)[i % LENGTH(alpha)];
}

SEXP gpLineEndSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LINEEND);
}

R_GE_lineend gpLineEnd(SEXP gp, int i) {
    SEXP lineend = gpLineEndSXP(gp);
    return LENDpar(lineend, i % LENGTH(lineend));
}

SEXP gpLineJoinSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LINEJOIN);
}

R_GE_linejoin gpLineJoin(SEXP gp, int i) {
    SEXP linejoin = gpLineJoinSXP(gp);
    return LJOINpar(linejoin, i % LENGTH(linejoin));
}

SEXP gpLineMitreSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LINEMITRE);
}

double gpLineMitre(SEXP gp, int i) {
    SEXP linemitre = gpLineMitreSXP(gp);
    return REAL(linemitre)[i % LENGTH(linemitre)];
}

SEXP gpLexSXP(SEXP gp) {
    return VECTOR_ELT(gp, GP_LEX);
}

double gpLex(SEXP gp, int i) {
    SEXP lex = gpLexSXP(gp);
    return REAL(lex)[i % LENGTH(lex)];
}

/*
 * Never access fontface because fontface values are stored in font
 * Historical reasons ...
 */

/*
 * Combine gpar alpha with alpha level stored in colour 
 *
 * finalAlpha = gpAlpha*(R_ALPHA(col)/255)
 *
 * Based on my reading of how group alpha and individual
 * object alphas are combined in the SVG 1.0 docs
 *
 * Also has nice properties:
 *  (i)   range of finalAlpha is 0 to 1.
 *  (ii)  if either of gpAlpha or R_ALPHA(col) are 0 then finalAlpha = 0
 *        (i.e., can never make fully transparent colour less transparent).
 *  (iii) in order to get finalAlpha = 1, both gpAlpha and R_ALPHA(col)
 *        must be 1 (i.e., only way to get fully opaque is if both
 *        alpha levels are fully opaque).
 */
static unsigned int combineAlpha(double alpha, int col) 
{
    unsigned int newAlpha = (alpha*(R_ALPHA(col)/255.0))*255;
    return R_RGBA(R_RED(col), R_GREEN(col), R_BLUE(col), newAlpha);
}

/* 
 * Generate an R_GE_gcontext from a gpar
 */
void gcontextFromgpar(SEXP gp, int i, R_GE_gcontext *gc) 
{
    /* 
     * Combine gpAlpha with col and fill
     */
    gc->col = combineAlpha(gpAlpha(gp, i), gpCol(gp, i));
    gc->fill = combineAlpha(gpAlpha(gp, i), gpFill(gp, i));
    gc->gamma = gpGamma(gp, i);
    /*
     * Combine gpLex with lwd
     */
    gc->lwd = gpLineWidth(gp, i) * gpLex(gp, i);
    gc->lty = gpLineType(gp, i);
    gc->lend = gpLineEnd(gp, i);
    gc->ljoin = gpLineJoin(gp, i);
    gc->lmitre = gpLineMitre(gp, i);
    gc->cex = gpCex(gp, i);
    gc->ps = gpFontSize(gp, i);
    gc->lineheight = gpLineHeight(gp, i);
    gc->fontface = gpFont(gp, i);
    strcpy(gc->fontfamily, gpFontFamily(gp, i));
}

SEXP L_setGPar(SEXP gpars) 
{
    /* Set the value of the current gpars on the current device
     * Need to do this in here so that redrawing via R BASE display
     * list works 
     */
    /* Get the current device 
     */
    GEDevDesc *dd = getDevice();
    setGridStateElement(dd, GSS_GPAR, gpars);
    return R_NilValue;
}

SEXP L_getGPar(void) 
{
    /* Get the value of the current gpars on the current device
     * Need to do this in here so that redrawing via R BASE display
     * list works 
     */
    /* Get the current device 
     */
    GEDevDesc *dd = getDevice();
    return gridStateElement(dd, GSS_GPAR);
}

SEXP L_getGPsaved() 
{
    /* Get the current device 
     */
    GEDevDesc *dd = getDevice();
    return gridStateElement(dd, GSS_GPSAVED);
}

SEXP L_setGPsaved(SEXP gpars) 
{
    /* Get the current device 
     */
    GEDevDesc *dd = getDevice();
    setGridStateElement(dd, GSS_GPSAVED, gpars);
    return R_NilValue;
}

void initGPar(GEDevDesc *dd)
{
    NewDevDesc *dev = dd->dev;
    SEXP gpar, gparnames, class;
    SEXP gpfill, gpcol, gpgamma, gplty, gplwd, gpcex, gpfs, gplh, gpfont;
    SEXP gpfontfamily, gpalpha, gplineend, gplinejoin, gplinemitre, gplex;
    SEXP gsd = (SEXP) dd->gesd[gridRegisterIndex]->systemSpecific;
    PROTECT(gpar = allocVector(VECSXP, 15));
    PROTECT(gparnames = allocVector(STRSXP, 15));
    SET_STRING_ELT(gparnames, GP_FILL, mkChar("fill"));
    SET_STRING_ELT(gparnames, GP_COL, mkChar("col"));
    SET_STRING_ELT(gparnames, GP_GAMMA, mkChar("gamma"));
    SET_STRING_ELT(gparnames, GP_LTY, mkChar("lty"));
    SET_STRING_ELT(gparnames, GP_LWD, mkChar("lwd"));
    SET_STRING_ELT(gparnames, GP_CEX, mkChar("cex"));
    SET_STRING_ELT(gparnames, GP_FONTSIZE, mkChar("fontsize"));
    SET_STRING_ELT(gparnames, GP_LINEHEIGHT, mkChar("lineheight"));
    SET_STRING_ELT(gparnames, GP_FONT, mkChar("font"));
    SET_STRING_ELT(gparnames, GP_FONTFAMILY, mkChar("fontfamily"));
    SET_STRING_ELT(gparnames, GP_ALPHA, mkChar("alpha"));
    SET_STRING_ELT(gparnames, GP_LINEEND, mkChar("lineend"));
    SET_STRING_ELT(gparnames, GP_LINEJOIN, mkChar("linejoin"));
    SET_STRING_ELT(gparnames, GP_LINEMITRE, mkChar("linemitre"));
    SET_STRING_ELT(gparnames, GP_LEX, mkChar("lex"));
    setAttrib(gpar, R_NamesSymbol, gparnames);
    /* FIXME:  Need to export col2name via (probably) GraphicsEngine.h
     * In the meantime I just have to override the device settings
     */
    PROTECT(gpfill = allocVector(STRSXP, 1));
    /* SET_STRING_ELT(gpfill, 0, mkChar(col2name(dev->startfill))); */
    SET_STRING_ELT(gpfill, 0, mkChar("transparent"));
    SET_VECTOR_ELT(gpar, GP_FILL, gpfill);
    PROTECT(gpcol = allocVector(STRSXP, 1));
    /* SET_STRING_ELT(gpcol, 0, mkChar(col2name(dev->startcol))); */
    SET_STRING_ELT(gpcol, 0, mkChar("black"));
    SET_VECTOR_ELT(gpar, GP_COL, gpcol);
    PROTECT(gpgamma = allocVector(REALSXP, 1));
    REAL(gpgamma)[0] = dev->startgamma;
    SET_VECTOR_ELT(gpar, GP_GAMMA, gpgamma);
    PROTECT(gplty = LTYget(dev->startlty));
    SET_VECTOR_ELT(gpar, GP_LTY, gplty);
    PROTECT(gplwd = allocVector(REALSXP, 1));
    REAL(gplwd)[0] = 1;
    SET_VECTOR_ELT(gpar, GP_LWD, gplwd);
    PROTECT(gpcex = allocVector(REALSXP, 1));
    REAL(gpcex)[0] = 1;
    SET_VECTOR_ELT(gpar, GP_CEX, gpcex);
    PROTECT(gpfs = allocVector(REALSXP, 1));
    REAL(gpfs)[0] = dev->startps;
    SET_VECTOR_ELT(gpar, GP_FONTSIZE, gpfs);
    PROTECT(gplh = allocVector(REALSXP, 1));
    REAL(gplh)[0] = 1.2;
    SET_VECTOR_ELT(gpar, GP_LINEHEIGHT, gplh);
    PROTECT(gpfont = allocVector(INTSXP, 1));
    INTEGER(gpfont)[0] = dev->startfont;
    SET_VECTOR_ELT(gpar, GP_FONT, gpfont);
    PROTECT(gpfontfamily = allocVector(STRSXP, 1));
    /* 
     * A font family of "" means that the default font
     * set up by the device will be used.
     */
    SET_STRING_ELT(gpfontfamily, 0, mkChar(""));
    SET_VECTOR_ELT(gpar, GP_FONTFAMILY, gpfontfamily);
    PROTECT(gpalpha = allocVector(REALSXP, 1));
    REAL(gpalpha)[0] = 1;
    SET_VECTOR_ELT(gpar, GP_ALPHA, gpalpha);
    PROTECT(gplineend = allocVector(STRSXP, 1));
    SET_STRING_ELT(gplineend, 0, mkChar("round"));
    SET_VECTOR_ELT(gpar, GP_LINEEND, gplineend);
    PROTECT(gplinejoin = allocVector(STRSXP, 1));
    SET_STRING_ELT(gplinejoin, 0, mkChar("round"));
    SET_VECTOR_ELT(gpar, GP_LINEJOIN, gplinejoin);
    PROTECT(gplinemitre = allocVector(REALSXP, 1));
    REAL(gplinemitre)[0] = 10;
    SET_VECTOR_ELT(gpar, GP_LINEMITRE, gplinemitre);
    PROTECT(gplex = allocVector(REALSXP, 1));
    REAL(gplex)[0] = 1;
    SET_VECTOR_ELT(gpar, GP_LEX, gplex);
    PROTECT(class = allocVector(STRSXP, 1));
    SET_STRING_ELT(class, 0, mkChar("gpar"));
    classgets(gpar, class);
    SET_VECTOR_ELT(gsd, GSS_GPAR, gpar);
    UNPROTECT(18);
}
