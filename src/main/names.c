/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2002  Robert Gentleman, Ross Ihaka and the
 *                            R Development Core Team
 *  Copyright (C) 2003, 2004  The R Foundation
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* <UTF8> char here is either ASCII or handled as a whole */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define __R_Names__
#include "Defn.h"
#include "Print.h"
#include "arithmetic.h"

#include <R_ext/RConverters.h>

/* Table of  .Internal(.) and .Primitive(.)  R functions
 * =====     =========	      ==========
 *
 * Each entry is a line with
 *
 * printname	c-entry		offset	 eval	arity	points	escape	pp-kind	precedence	rightassoc
 *  ---------	-------		------	 ----	-----	------	------	-------	----------	----------
 *2 name	cfun		code	 eval	arity	points	escape	gram.kind gram.precedence gram.rightassoc
 *3 PRIMNAME	PRIMFUN		PRIMVAL [*] PRIMARITY PRIMPOINTS PRIMESCAPE PPINFO PPINFO	PPINFO
 *
 * where "2" are the component names of the FUNTAB struct (Defn.h)
 * and	 "3" are the accessor macros. [*]: PRIMPRINT(.) uses the eval component
 *
 * printname:	The function name in R
 *
 * c-entry:	The name of the corresponding C function,
 *		actually declared in ../include/Internal.h .
 *		Convention:
 *		 - all start with "do_",
 *		 - all return SEXP.
 *		 - all have argument list
 *			 (SEXP call, SEXP op, SEXP args, SEXP env)
 *
 * offset:	the 'op' (offset pointer) above; used for C functions
 *		which deal with more than one R function...
 *
 * eval:	= XYZ (three digits)  [New Apr 9/96, before only had "YZ"].
 *		  --- where e.g. '1' means '001'
 *		X=1 says that we should switch R_Visible off
 *		    (the least common situation).
 *		Y=1 says that this is an internal function which must
 *		    be accessed with a	.Internal(.) call, any other value is
 *		    accessible directly and printed in R as ".Primitive(..)".
 *		Z=1 says evaluate arguments before calling and
 *		Z=0 says don't evaluate.
 *
 * arity:	How many arguments are required/allowed;  "-1"	meaning ``any''
 *
 * points:	bit field: for each argument, whether it may escape via
 *		return value (that is, the return value may point to it)
 *		-1 means return may point to any argument (conservative)
 *              for example, 0x6 means the first and second arguments may escape
 * 
 * escape:	bit field like points: for each argument, whether it may escape via assignment
 *
 * pp-kind:	Deparsing Info (-> PPkind in ../include/Defn.h )
 *
 * precedence: Operator precedence (-> PPprec in ../include/Defn.h )
 *
 * rightassoc: Right (1) or left (0) associative operator
 *
 */
FUNTAB R_FunTab[] =
{

#define CF(a) #a , a

/* Language Related Constructs */

/* printname	c-entry		offset	eval	arity	points	escape	pp-kind	precedence	rightassoc
 * ---------	-------		------	----	-----	------	------	------- ----------	----------*/
{"if",		CF(do_if),	0,	0,	-1,	-1,	-1,	{PP_IF,	     PREC_FN,	  1}},
{"while",	CF(do_while),	0,	0,	-1,	-1,	-1,	{PP_WHILE,   PREC_FN,	  0}},
{"for",		CF(do_for),	0,	0,	-1,	-1,	-1,	{PP_FOR,     PREC_FN,	  0}},
{"repeat",	CF(do_repeat),	0,	0,	-1,	-1,	-1,	{PP_REPEAT,  PREC_FN,	  0}},
{"break",	CF(do_break), CTXT_BREAK,0,	-1,	-1,	-1,	{PP_BREAK,   PREC_FN,	  0}},
{"next",	CF(do_break), CTXT_NEXT,0,	-1,	-1,	-1,	{PP_NEXT,    PREC_FN,	  0}},
{"return",	CF(do_return),	0,	0,	-1,	-1,	-1,	{PP_RETURN,  PREC_FN,	  0}},
{"stop",	CF(do_stop),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"warning",	CF(do_warning),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"gettext",	CF(do_gettext),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"ngettext",	CF(do_ngettext),0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"bindtextdomain",CF(do_bindtextdomain),0,11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
#ifdef NEW_CONDITION_HANDLING
{".addCondHands",CF(do_addCondHands),0,111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".resetCondHands",CF(do_resetCondHands),0,111,1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".signalCondition",CF(do_signalCondition),0,11,3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".dfltStop",CF(do_dfltStop),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".dfltWarn",CF(do_dfltWarn),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".addRestart",CF(do_addRestart),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".getRestart",CF(do_getRestart),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".invokeRestart",CF(do_invokeRestart),0,11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
#endif
{".addTryHandlers",CF(do_addTryHandlers),0,111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"geterrmessage",CF(do_geterrmessage),0,11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"restart",	CF(do_restart),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"function",	CF(do_function),0,	0,	-1,	-1,	-1,	{PP_FUNCTION,PREC_FN,	  0}},
{"as.function.default",CF(do_asfunction),0,11,	2,	-1,	-1,	{PP_FUNCTION,PREC_FN,	  0}},
{"<-",		CF(do_set),	1,	100,	-1,	-1,	-1,	{PP_ASSIGN,  PREC_LEFT,	  1}},
{"=",		CF(do_set),	3,	100,	-1,	-1,	-1,	{PP_ASSIGN,  PREC_EQ,	  1}},
{"<<-",		CF(do_set),	2,	100,	-1,	-1,	-1,	{PP_ASSIGN2, PREC_LEFT,	  1}},
{"{",		CF(do_begin),	0,	0,	-1,	-1,	-1,	{PP_CURLY,   PREC_FN,	  0}},
{"(",		CF(do_paren),	0,	1,	1,	-1,	-1,	{PP_PAREN,   PREC_FN,	  0}},
{".subset",	CF(do_subset_dflt), 1,	1,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".subset2",	CF(do_subset2_dflt), 2,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"[",		CF(do_subset),	1,	0,	-1,	-1,	-1,	{PP_SUBSET,  PREC_SUBSET, 0}},
{"[[",		CF(do_subset2),	2,	0,	2,	-1,	-1,	{PP_SUBSET,  PREC_SUBSET, 0}},
{"$",		CF(do_subset3),	3,	0,	2,	-1,	-1,	{PP_DOLLAR,  PREC_DOLLAR, 0}},
/*{"::",	CF(do_NS_get),	?,	?,	2,	-1,	-1,	{PP_BINARY2, PREC_NS,	  0}},*/
{"@",		CF(do_AT),	0,	0,	2,	-1,	-1,	{PP_DOLLAR,  PREC_DOLLAR, 0}},
{"[<-",		CF(do_subassign),0,	0,	3,	-1,	-1,	{PP_SUBASS,  PREC_LEFT,	  1}},
{"[[<-",	CF(do_subassign2),1,	100,	3,	-1,	-1,	{PP_SUBASS,  PREC_LEFT,	  1}},
{"$<-",		CF(do_subassign3),1,	0,	3,	-1,	-1,	{PP_SUBASS,  PREC_LEFT,	  1}},
{"switch",	CF(do_switch),	0,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"browser",	CF(do_browser),	0,	100,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"debug",	CF(do_debug),	0,	101,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"undebug",	CF(do_debug),	1,	101,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".primTrace",	CF(do_trace),	0,	101,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".primUntrace",CF(do_trace),	1,	101,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{".Internal",	CF(do_internal),0,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"on.exit",	CF(do_onexit),	0,	100,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"Recall",	CF(do_recall),	0,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"delay",	CF(do_delay),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"delayedAssign",CF(do_delayed),0,	111,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
/*{".Alias",	CF(do_alias),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},*/
{".Primitive",	CF(do_primitive),0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},
{"identical",	CF(do_ident),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	  0}},


/* Binary Operators */
{"+",		CF(do_arith),	PLUSOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_SUM,	  0}},
{"-",		CF(do_arith),	MINUSOP,1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_SUM,	  0}},
{"*",		CF(do_arith),	TIMESOP,1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_PROD,	  0}},
{"/",		CF(do_arith),	DIVOP,	1,	2,	0x0,	0x0,	{PP_BINARY2, PREC_PROD,	  0}},
{"^",		CF(do_arith),	POWOP,	1,	2,	0x0,	0x0,	{PP_BINARY2, PREC_POWER,  1}},
{"%%",		CF(do_arith),	MODOP,	1,	2,	0x0,	0x0,	{PP_BINARY2, PREC_PERCENT,0}},
{"%/%",		CF(do_arith),	IDIVOP,	1,	2,	0x0,	0x0,	{PP_BINARY2, PREC_PERCENT,0}},
{"%*%",		CF(do_matprod),	0,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_PERCENT,0}},
{"crossprod",	CF(do_matprod),	1,	11,	2,	0x0,	0x0,	{PP_FUNCALL, PREC_FN,	  0}},
{"==",		CF(do_relop),	EQOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{"!=",		CF(do_relop),	NEOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{"<",		CF(do_relop),	LTOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{"<=",		CF(do_relop),	LEOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{">=",		CF(do_relop),	GEOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{">",		CF(do_relop),	GTOP,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_COMPARE,0}},
{"&",		CF(do_logic),	1,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_AND,	  0}},
{"|",		CF(do_logic),	2,	1,	2,	0x0,	0x0,	{PP_BINARY,  PREC_OR,	  0}},
{"!",		CF(do_logic),	3,	1,	1,	0x0,	0x0,	{PP_UNARY,   PREC_NOT,	  0}},
{"&&",		CF(do_logic2),	1,	0,	2,	0x0,	0x0,	{PP_BINARY,  PREC_AND,	  0}},
{"||",		CF(do_logic2),	2,	0,	2,	0x0,	0x0,	{PP_BINARY,  PREC_OR,	  0}},
{":",		CF(do_seq),	0,	1,	2,	0x0,	0x0,	{PP_BINARY2, PREC_COLON,  0}},
{"~",		CF(do_tilde),	0,	0,	2,	0x0,	0x0,	{PP_BINARY,  PREC_TILDE,  0}},


/* Logic Related Functions */

{"all",		CF(do_logic3),	1,	11,	-1,	0x0,	0x0,	{PP_FUNCALL, PREC_FN,	  0}},
{"any",		CF(do_logic3),	2,	11,	-1,	0x0,	0x0,	{PP_FUNCALL, PREC_FN,	  0}},


/* Vectors, Matrices and Arrays */

/* printname	c-entry		offset	eval	arity	points	escape	pp-kind	precedence	rightassoc
 * ---------	-------		------	----	-----	------	------	------- ----------	----------*/
{"vector",	CF(do_makevector),0,	11,	2,	0x0,	0x0,	{PP_FUNCALL, PREC_FN,	0}},
{"complex",	CF(do_complex),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN, 	0}},
{"matrix",	CF(do_matrix),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/*MM(98/4/22){"array",	CF(do_array),0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/
{"length",	CF(do_length),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"length<-",	CF(do_lengthgets),0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"row",		CF(do_rowscols),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"col",		CF(do_rowscols),2,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"c",		CF(do_c),	0,	0,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}}, /* in bind.c */
{"unlist",	CF(do_unlist),	0,	10,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cbind",	CF(do_bind),	1,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rbind",	CF(do_bind),	2,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"drop",	CF(do_drop),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"oldClass",	CF(do_class),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"oldClass<-",	CF(do_classgets),0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT, 1}},
{"class",	CF(R_do_data_class),0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"class<-",	CF(R_do_set_class),0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"unclass",	CF(do_unclass),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"names",	CF(do_names),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"names<-",	CF(do_namesgets),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"dimnames",	CF(do_dimnames),0,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dimnames<-",	CF(do_dimnamesgets),0,	0,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"all.names",	CF(do_allnames),0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dim",		CF(do_dim),	0,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dim<-",	CF(do_dimgets),	0,	0,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"attributes",	CF(do_attributes),0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"attributes<-",CF(do_attributesgets),0,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"attr",	CF(do_attr),	0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"attr<-",	CF(do_attrgets),0,	0,	3,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"comment",	CF(do_comment),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"comment<-",	CF(do_commentgets),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"get",		CF(do_get),	1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"mget",	CF(do_mget),    1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"exists",	CF(do_get),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"assign",	CF(do_assign),	0,	111,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"remove",	CF(do_remove),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"duplicated",	CF(do_duplicated),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"unique",	CF(do_duplicated),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"which.min",	CF(do_first_min),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"which.max",	CF(do_first_max),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"match",	CF(do_match),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pmatch",	CF(do_pmatch),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"charmatch",	CF(do_charmatch),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"match.call",	CF(do_matchcall),0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"complete.cases",CF(do_compcases),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"attach",	CF(do_attach),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"detach",	CF(do_detach),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"search",	CF(do_search),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},


/* Mathematical Functions */
{"round",	CF(do_Math2),	10001,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"atan",	CF(do_atan),	10002,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"log",		CF(do_log),	10003,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"signif",	CF(do_Math2),	10004,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"abs",		CF(do_abs),	6,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* KH(1999/09/12)-> complex: {"abs", CF(do_math1, 0, 1, 1,	-1,	-1, {PP_FUNCALL, PREC_FN,	0}}, */
{"floor",	CF(do_math1),	1,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"ceiling",	CF(do_math1),	2,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sqrt",	CF(do_math1),	3,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sign",	CF(do_math1),	4,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"trunc",	CF(do_math1),	5,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"exp",		CF(do_math1),	10,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"expm1",	CF(do_math1),	11,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"log1p",	CF(do_math1),	12,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"cos",		CF(do_math1),	20,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sin",		CF(do_math1),	21,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"tan",		CF(do_math1),	22,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"acos",	CF(do_math1),	23,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"asin",	CF(do_math1),	24,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"cosh",	CF(do_math1),	30,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sinh",	CF(do_math1),	31,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"tanh",	CF(do_math1),	32,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"acosh",	CF(do_math1),	33,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"asinh",	CF(do_math1),	34,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"atanh",	CF(do_math1),	35,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"lgamma",	CF(do_math1),	40,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gamma",	CF(do_math1),	41,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Polygamma Functions */

{"digamma",	CF(do_math1),	42,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"trigamma",	CF(do_math1),	43,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/* {"tetragamma",	CF(do_math1),	44,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pentagamma",	CF(do_math1),	45,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/
{"gammaCody",	CF(do_math1),	46,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/* see "psigamma" below !*/

/* Mathematical Functions of Two Numeric (+ 1-2 int) Variables */

{"atan2",	CF(do_math2),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"lbeta",	CF(do_math2),	2,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"beta",	CF(do_math2),	3,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"lchoose",	CF(do_math2),	4,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"choose",	CF(do_math2),	5,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dchisq",	CF(do_math2),	6,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pchisq",	CF(do_math2),	7,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qchisq",	CF(do_math2),	8,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dexp",	CF(do_math2),	9,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pexp",	CF(do_math2),	10,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qexp",	CF(do_math2),	11,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dgeom",	CF(do_math2),	12,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pgeom",	CF(do_math2),	13,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qgeom",	CF(do_math2),	14,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dpois",	CF(do_math2),	15,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"ppois",	CF(do_math2),	16,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qpois",	CF(do_math2),	17,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dt",		CF(do_math2),	18,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pt",		CF(do_math2),	19,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qt",		CF(do_math2),	20,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dsignrank",	CF(do_math2),	21,	11,	2+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"psignrank",	CF(do_math2),	22,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qsignrank",	CF(do_math2),	23,	11,	2+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"besselJ",	CF(do_math2),	24,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"besselY",	CF(do_math2),	25,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"psigamma",	CF(do_math2),	26,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},


/* Mathematical Functions of a Complex Argument */

{"Re",		CF(do_cmathfuns),	1,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Im",		CF(do_cmathfuns),	2,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Mod",		CF(do_cmathfuns),	3,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Arg",		CF(do_cmathfuns),	4,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Conj",	CF(do_cmathfuns),	5,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/* {"abs",	CF(do_cmathfuns),	6,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/


/* Mathematical Functions of Three Numeric (+ 1-2 int) Variables */

{"dbeta",	CF(do_math3),	1,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pbeta",	CF(do_math3),	2,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qbeta",	CF(do_math3),	3,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dbinom",	CF(do_math3),	4,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pbinom",	CF(do_math3),	5,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qbinom",	CF(do_math3),	6,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dcauchy",	CF(do_math3),	7,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pcauchy",	CF(do_math3),	8,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qcauchy",	CF(do_math3),	9,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"df",		CF(do_math3),	10,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pf",		CF(do_math3),	11,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qf",		CF(do_math3),	12,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dgamma",	CF(do_math3),	13,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pgamma",	CF(do_math3),	14,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qgamma",	CF(do_math3),	15,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dlnorm",	CF(do_math3),	16,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"plnorm",	CF(do_math3),	17,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qlnorm",	CF(do_math3),	18,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dlogis",	CF(do_math3),	19,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"plogis",	CF(do_math3),	20,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qlogis",	CF(do_math3),	21,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnbinom",	CF(do_math3),	22,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnbinom",	CF(do_math3),	23,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnbinom",	CF(do_math3),	24,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnorm",	CF(do_math3),	25,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnorm",	CF(do_math3),	26,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnorm",	CF(do_math3),	27,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dunif",	CF(do_math3),	28,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"punif",	CF(do_math3),	29,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qunif",	CF(do_math3),	30,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dweibull",	CF(do_math3),	31,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pweibull",	CF(do_math3),	32,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qweibull",	CF(do_math3),	33,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnchisq",	CF(do_math3),	34,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnchisq",	CF(do_math3),	35,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnchisq",	CF(do_math3),	36,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnt",		CF(do_math3),	37,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnt",		CF(do_math3),	38,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnt",		CF(do_math3),	39,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dwilcox",	CF(do_math3),	40,	11,	3+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pwilcox",	CF(do_math3),	41,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qwilcox",	CF(do_math3),	42,	11,	3+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"besselI",	CF(do_math3),	43,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"besselK",	CF(do_math3),	44,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Mathematical Functions of Four Numeric (+ 1-2 int) Variables */

{"dhyper",	CF(do_math4),	1,	11,	4+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"phyper",	CF(do_math4),	2,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qhyper",	CF(do_math4),	3,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnbeta",	CF(do_math4),	4,	11,	4+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnbeta",	CF(do_math4),	5,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnbeta",	CF(do_math4),	6,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dnf",		CF(do_math4),	7,	11,	4+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pnf",		CF(do_math4),	8,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qnf",		CF(do_math4),	9,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"dtukey",	CF(do_math4),	10,	11,	4+1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"ptukey",	CF(do_math4),	11,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qtukey",	CF(do_math4),	12,	11,	4+2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Random Numbers */

{"rchisq",	CF(do_random1),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rexp",	CF(do_random1),	1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rgeom",	CF(do_random1),	2,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rpois",	CF(do_random1),	3,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rt",		CF(do_random1),	4,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rsignrank",	CF(do_random1),	5,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"rbeta",	CF(do_random2),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rbinom",	CF(do_random2),	1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rcauchy",	CF(do_random2),	2,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rf",		CF(do_random2),	3,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rgamma",	CF(do_random2),	4,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rlnorm",	CF(do_random2),	5,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rlogis",	CF(do_random2),	6,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rnbinom",	CF(do_random2),	7,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rnchisq",	CF(do_random2),	12,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rnorm",	CF(do_random2),	8,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"runif",	CF(do_random2),	9,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rweibull",	CF(do_random2),	10,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rwilcox",	CF(do_random2),	11,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"rhyper",	CF(do_random3),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"rmultinom",	CF(do_rmultinom),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sample",	CF(do_sample),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"RNGkind",	CF(do_RNGkind),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"set.seed",	CF(do_setseed),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Data Summaries */

{"sum",		CF(do_summary),	0,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
  /*MM{"mean",	CF(do_summary),	1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/
{"min",		CF(do_summary),	2,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"max",		CF(do_summary),	3,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"prod",	CF(do_summary),	4,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"range",	CF(do_range),	0,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cov",		CF(do_cov),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cor",		CF(do_cov),	1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"cumsum",	CF(do_cum),	1,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cumprod",	CF(do_cum),	2,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cummax",	CF(do_cum),	3,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cummin",	CF(do_cum),	4,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Type coercion */

{"as.character",CF(do_ascharacter),0,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"as.vector",	CF(do_asvector),0,	10,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"paste",	CF(do_paste),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"format",	CF(do_format),	0,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"format.info",	CF(do_formatinfo),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"cat",		CF(do_cat),	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"call",	CF(do_call),	0,	0,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"do.call",	CF(do_docall),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"as.call",	CF(do_ascall),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"type.convert",CF(do_typecvt),	1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"as.environment",CF(do_as_environment),0,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},


/* String Manipulation */

{"nchar",	CF(do_nchar),	1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"substr",	CF(do_substr),	1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"substrgets",	CF(do_substrgets),1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"strsplit",	CF(do_strsplit),1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"abbreviate",	CF(do_abbrev),	1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"make.names",	CF(do_makenames),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"grep",	CF(do_grep),	1,	11,	7,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sub",		CF(do_gsub),	0,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gsub",	CF(do_gsub),	1,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"regexpr",	CF(do_regexpr),	1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"grep.perl",	CF(do_pgrep),	1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sub.perl",    CF(do_pgsub),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gsub.perl",	CF(do_pgsub),	1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"regexpr.perl",CF(do_pregexpr),1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"agrep",	CF(do_agrep),	1,	11,	8,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"tolower",	CF(do_tolower),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"toupper",	CF(do_tolower),	1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"chartr",	CF(do_chartr),	1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sprintf",	CF(do_sprintf),	1,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"make.unique",	CF(do_makeunique),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"charToRaw",	CF(do_charToRaw),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rawToChar",	CF(do_rawToChar),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rawShift",	CF(do_rawShift),1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"intToBits",	CF(do_intToBits),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rawToBits",	CF(do_rawToBits),1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"packBits",	CF(do_packBits),1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"encodeString",CF(do_encodeString),1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"iconv",	CF(do_iconv),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"strtrim",	CF(do_strtrim),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Type Checking (typically implemented in ./coerce.c ) */

{"is.null",	CF(do_is),	NILSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.logical",	CF(do_is),	LGLSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.integer",	CF(do_is),	INTSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.real",	CF(do_is),	REALSXP,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.double",	CF(do_is),	REALSXP,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.complex",	CF(do_is),	CPLXSXP,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.character",CF(do_is),	STRSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.symbol",	CF(do_is),	SYMSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.environment",CF(do_is),	ENVSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.list",	CF(do_is),	VECSXP,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.pairlist",	CF(do_is),	LISTSXP,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.expression",CF(do_is),	EXPRSXP,1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.object",	CF(do_is),	50,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.numeric",	CF(do_is),	100,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.matrix",	CF(do_is),	101,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.array",	CF(do_is),	102,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.atomic",	CF(do_is),	200,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.recursive",CF(do_is),	201,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.call",	CF(do_is),	300,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.language",	CF(do_is),	301,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.function",	CF(do_is),	302,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.single",	CF(do_is),	999,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"is.vector",	CF(do_isvector),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.na",	CF(do_isna),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.nan",	CF(do_isnan),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.finite",	CF(do_isfinite),0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.infinite",	CF(do_isinfinite),0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},


/* Miscellaneous */

{"proc.time",	CF(do_proctime),0,	1,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gc.time",	CF(do_gctime),	0,	1,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Version",	CF(do_version),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"machine",	CF(do_machine),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/*{"Machine",	CF(do_Machine),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/
{"commandArgs", CF(do_commandArgs), 0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"int.unzip",	CF(do_int_unzip),0,	11,    -1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#ifdef Win32
{"system",	CF(do_system),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#else
{"system",	CF(do_system),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#endif
#ifdef Win32
{"unlink",	CF(do_unlink),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"help.start",	CF(do_helpstart),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"show.help.item",CF(do_helpitem),0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"flush.console",CF(do_flushconsole),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"win.version", CF(do_winver),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"shell.exec",	CF(do_shellexec),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"winDialog",	CF(do_windialog),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"winDialogString", CF(do_windialogstring),0,11,2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"winMenuNames", CF(do_winmenunames),0, 11,     0,	-1,	-1,     {PP_FUNCALL, PREC_FN,   0}},
{"winMenuItems", CF(do_wingetmenuitems),0,11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"winMenuAdd",	CF(do_winmenuadd),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"winMenuDel",	CF(do_winmenudel),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"memory.size",	CF(do_memsize),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"DLL.version",	CF(do_dllversion),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"bringToTop",	CF(do_bringtotop),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"select.list",	CF(do_selectlist),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"readClipboard",CF(do_readClipboard),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"writeClipboard",CF(do_writeClipboard),0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"chooseFiles", CF(do_chooseFiles), 0,  	11, 	5,	-1,	-1,  	{PP_FUNCALL, PREC_FN,   0}},
{"getIdentification", CF(do_getIdentification),0,11,0,	-1,	-1,	{PP_FUNCALL, PREC_FN, 	0}},
{"getWindowHandle", CF(do_getWindowHandle),0,11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN, 	0}},
{"getWindowTitle",CF(do_getWindowTitle),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN, 	0}},
{"setWindowTitle",CF(do_setTitle),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#endif
#if defined(__APPLE_CC__) && defined(HAVE_AQUA)
{"wsbrowser",	CF(do_wsbrowser),	0,	11,	8,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pkgbrowser",	CF(do_browsepkgs),	0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"data.manager",	CF(do_datamanger),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"package.manager",	CF(do_packagemanger),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"flush.console",CF(do_flushconsole),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"hsbrowser",	CF(do_hsbrowser),	0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"select.list",	CF(do_selectlist),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"aqua.custom.print", CF(do_aqua_custom_print), 0, 11, 2,	-1,	-1,   {PP_FUNCALL, PREC_FN,   0}},
#endif
{"parse",	CF(do_parse),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"save",	CF(do_save),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"saveToConn",	CF(do_saveToConn),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"load",	CF(do_load),	0,	111,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"loadFromConn",CF(do_loadFromConn),0,	111,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"serializeToConn",	CF(do_serializeToConn),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"unserializeFromConn",	CF(do_unserializeFromConn),	0,	111,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"deparse",	CF(do_deparse),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dput",	CF(do_dput),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dump",	CF(do_dump),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"substitute",	CF(do_substitute),	0,	0,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"quote",	CF(do_quote),	0,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"quit",	CF(do_quit),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"interactive",	CF(do_interactive),	0,	0,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"readline",	CF(do_readln),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"menu",	CF(do_menu),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"print.default",CF(do_printdefault),0,	111,	7,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"prmatrix", 	CF(do_prmatrix), 	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"invisible",	CF(do_invisible),	0,	101,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gc",		CF(do_gc),		0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gcinfo",	CF(do_gcinfo),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gctorture",	CF(do_gctorture),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"memory.profile",CF(do_memoryprofile), 0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rep",		CF(do_rep),		0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"list",	CF(do_makelist),	1,	1,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"split",	CF(do_split),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"symbol.C",	CF(do_symbol),	0,	1,	1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{"symbol.For",	CF(do_symbol),	1,	1,	1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{"is.loaded",	CF(do_isloaded),	0,	1,	-1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{".C",		CF(do_dotCode),	0,	1,	-1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{".Fortran",	CF(do_dotCode),	1,	1,	-1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{".External",   CF(do_External),    0,      1,      -1,	-1,	-1,     {PP_FOREIGN, PREC_FN,	0}},
{".Call",       CF(do_dotcall),     0,      1,      -1,	-1,	-1,     {PP_FOREIGN, PREC_FN,	0}},
{".External.graphics", CF(do_Externalgr), 0, 1,	-1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{".Call.graphics", CF(do_dotcallgr), 0,	1,	-1,	-1,	-1,	{PP_FOREIGN, PREC_FN,	0}},
{"recordGraphics", CF(do_recordGraphics), 0, 11,     3,	-1,	-1,      {PP_FOREIGN, PREC_FN,	0}},
{"dyn.load",	CF(do_dynload),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dyn.unload",	CF(do_dynunload),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"ls",		CF(do_ls),		1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"typeof",	CF(do_typeof),	1,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"eval",	CF(do_eval),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"eval.with.vis",CF(do_eval),	1,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"expression",	CF(do_expression),	1,	0,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.parent",	CF(do_sys),		1,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.call",	CF(do_sys),		2,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.frame",	CF(do_sys),		3,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.nframe",	CF(do_sys),		4,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.calls",	CF(do_sys),		5,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.frames",	CF(do_sys),		6,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.on.exit",	CF(do_sys),		7,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.parents",	CF(do_sys),		8,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sys.function",CF(do_sys),		9,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"parent.frame",CF(do_parentframe),	0,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sort",	CF(do_sort),	1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"is.unsorted",	CF(do_isunsorted),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"psort",	CF(do_psort),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"qsort",	CF(do_qsort),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"radixsort",	CF(do_radixsort),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"order",	CF(do_order),	0,	11,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rank",	CF(do_rank),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"missing",	CF(do_missing),	1,	0,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"nargs",	CF(do_nargs),	1,	0,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"scan",	CF(do_scan),	0,	11,	17,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"count.fields",CF(do_countfields),	0,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"readTableHead",CF(do_readtablehead),0,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"t.default",	CF(do_transpose),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"aperm",	CF(do_aperm),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"builtins",	CF(do_builtins),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"edit",	CF(do_edit),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dataentry",	CF(do_dataentry),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"args",	CF(do_args),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"formals",	CF(do_formals),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"body",	CF(do_body),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"bodyCode",	CF(do_bodyCode),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"globalenv",	CF(do_globalenv),	0,	1,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"environment",	CF(do_envir),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"environment<-",CF(do_envirgets),	0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_LEFT,	1}},
{"env2list",	CF(do_env2list),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"reg.finalizer",CF(do_regFinaliz),	0,	1,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"options",	CF(do_options),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sink",	CF(do_sink),	0,	111,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"sink.number",	CF(do_sinknumber),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"lib.fixup",	CF(do_libfixup),	0,	111,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"pos.to.env",	CF(do_pos2env),	0,	1,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"eapply",	CF(do_eapply),	0,	10,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"lapply",	CF(do_lapply),	0,	10,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"apply",	CF(do_apply),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"colSums",	CF(do_colsum),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"colMeans",	CF(do_colsum),	1,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rowSums",	CF(do_colsum),	2,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rowMeans",	CF(do_colsum),	3,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Rprof",	CF(do_Rprof),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"object.size",	CF(do_objectsize),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"mem.limits",	CF(do_memlimits),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"merge",	CF(do_merge),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"capabilities",CF(do_capabilities),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"new.env",	CF(do_newenv),	0,	11,     2,	-1,	-1,      {PP_FUNCALL, PREC_FN,	0}},
{"parent.env",  CF(do_parentenv),   0,	11,     1,	-1,	-1,      {PP_FUNCALL, PREC_FN,	0}},
{"parent.env<-",CF(do_parentenvgets), 0,	11,     2,	-1,	-1,      {PP_FUNCALL, PREC_LEFT,	1}},
#if 0
{"visibleflag", CF(do_visibleflag),	0,	1,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#endif
{"l10n_info", 	CF(do_l10n_info),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Functions To Interact with the Operating System */

{"file.show",	CF(do_fileshow),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.edit",	CF(do_fileedit),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.create",	CF(do_filecreate),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.remove",	CF(do_fileremove),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.rename",	CF(do_filerename),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.append",	CF(do_fileappend),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"codeFiles.append",CF(do_fileappend),1,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.symlink",CF(do_filesymlink),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"list.files",	CF(do_listfiles),	0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.exists", CF(do_fileexists),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.choose", CF(do_filechoose),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.info",	CF(do_fileinfo),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"file.access",	CF(do_fileaccess),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dir.create",	CF(do_dircreate),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"tempfile",	CF(do_tempfile),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"tempdir",	CF(do_tempdir),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"R.home",	CF(do_Rhome),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"date",	CF(do_date),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/*{"Platform",	CF(do_Platform),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},*/
{"index.search",CF(do_indexsearch), 0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getenv",	CF(do_getenv),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"putenv",	CF(do_putenv),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getwd",	CF(do_getwd),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"setwd",	CF(do_setwd),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"basename",	CF(do_basename),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dirname",	CF(do_dirname),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Sys.info",	CF(do_sysinfo),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Sys.sleep",	CF(do_syssleep),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getlocale",	CF(do_getlocale),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"setlocale",	CF(do_setlocale),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"localeconv",	CF(do_localeconv),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"path.expand",	CF(do_pathexpand),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getpid",	CF(do_sysgetpid),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"normalizePath",CF(do_normalizepath),0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Complex Valued Functions */
{"fft",		CF(do_fft),		0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"mvfft",	CF(do_mvfft),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"nextn",	CF(do_nextn),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"polyroot",	CF(do_polyroot),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Device Drivers */

#ifdef Unix
{"X11",		CF(do_X11),		0,	111,	11,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
#endif

/* Graphics */

{"dev.control",	CF(do_devcontrol),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dev.copy",	CF(do_devcopy),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dev.cur",	CF(do_devcur),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/*
{"device",	CF(do_device),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
*/
{"dev.next",	CF(do_devnext),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dev.off",	CF(do_devoff),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dev.prev",	CF(do_devprev),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dev.set",	CF(do_devset),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rgb",		CF(do_rgb),		0,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rgb256",	CF(do_rgb),		1,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rgb2hsv",	CF(do_RGB2hsv),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"hsv",		CF(do_hsv),		0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"hcl",		CF(do_hcl),		0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"gray",	CF(do_gray),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"colors",	CF(do_colors),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"col2rgb",	CF(do_col2RGB),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"palette",	CF(do_palette),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"plot.new",	CF(do_plot_new),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"plot.window",	CF(do_plot_window),	0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"axis",	CF(do_axis),	0,	111,   12,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"plot.xy",	CF(do_plot_xy),	0,	111,	7,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"text",	CF(do_text),	0,	111,	7,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"mtext",	CF(do_mtext),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"title",	CF(do_title),	0,	111,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"abline",	CF(do_abline),	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"box",		CF(do_box),		0,	111,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"rect",	CF(do_rect),	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"polygon",	CF(do_polygon),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"par",		CF(do_par),		0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"readonly.pars",CF(do_readonlypars),0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"segments",	CF(do_segments),	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"arrows",	CF(do_arrows),	0,	111,	9,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"layout",	CF(do_layout),	0,	111,	10,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"locator",	CF(do_locator),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"identify",	CF(do_identify),	0,	11,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"strheight",	CF(do_strheight),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"strwidth",	CF(do_strwidth),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"contour",	CF(do_contour),	0,	11,	12,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"contourLines",CF(do_contourLines),0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"image",	CF(do_image),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dend",	CF(do_dend),	0,	111,	6,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"dend.window",	CF(do_dendwindow),	0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"replay",	CF(do_replay),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"erase",	CF(do_erase),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/*{"dotplot",	CF(do_dotplot),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}}, */
{"persp",	CF(do_persp),	0,	111,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"filledcontour",CF(do_filledcontour),0,	111,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
/* {"getDL",	CF(do_getDL),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getGPar",	CF(do_getGPar),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}}, */
{"playDL",	CF(do_playDL),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"setGPar",	CF(do_setGPar),	0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getSnapshot",	CF(do_getSnapshot),	0,	111,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"playSnapshot",CF(do_playSnapshot),0,	111,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"symbols",	CF(do_symbols),	0,	111,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getGraphicsEvent",CF(do_getGraphicsEvent),0,  11, 5,	-1,	-1,      {PP_FUNCALL, PREC_FN,   0}},

/* Objects */
{"inherits",	CF(do_inherits),	0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"UseMethod",	CF(do_usemethod),	0,	 0,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"NextMethod",	CF(do_nextmethod),	0,	10,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"standardGeneric",CF(do_standardGeneric),0, 1,	-1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* Modelling Functionality */

{"nlm",		CF(do_nlm),		0,	11,	11,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"fmin",	CF(do_fmin),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"zeroin",	CF(do_zeroin),	0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"optim",	CF(do_optim),	0,	11,	7,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"optimhess",	CF(do_optimhess),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"terms.formula",CF(do_termsform),	0,	11,	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"update.formula",CF(do_updateform),0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"model.frame",	CF(do_modelframe),	0,	11,	8,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"model.matrix",CF(do_modelmatrix),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"D",		CF(do_D),		0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"deriv.default",CF(do_deriv),	0,	11,	5,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* History manipulation */
{"loadhistory", CF(do_loadhistory),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"savehistory", CF(do_savehistory),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

/* date-time manipulations */
{"Sys.time",	CF(do_systime),	0,	11,	0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"as.POSIXct",	CF(do_asPOSIXct),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"as.POSIXlt",	CF(do_asPOSIXlt),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"format.POSIXlt",CF(do_formatPOSIXlt),0,	11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"strptime",	CF(do_strptime),	0,	11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"Date2POSIXlt",CF(do_D2POSIXlt),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"POSIXlt2Date",CF(do_POSIXlt2D),	0,	11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

#ifdef BYTECODE
{"mkCode",     CF(do_mkcode),       0,      11,     2,	-1,	-1,      {PP_FUNCALL, PREC_FN, 0}},
{"bcClose",    CF(do_bcclose),      0,      11,     3,	-1,	-1,      {PP_FUNCALL, PREC_FN, 0}},
{"is.builtin.internal",    CF(do_is_builtin_internal),      0,      11,     1,	-1,	-1,      {PP_FUNCALL, PREC_FN, 0}},
{"disassemble", CF(do_disassemble), 0,      11,     1,	-1,	-1,	{PP_FUNCALL, PREC_FN, 0}},
{"bcVersion", CF(do_bcversion),     0,      11,     0,	-1,	-1,	{PP_FUNCALL, PREC_FN, 0}},
{"load.from.file", CF(do_loadfile), 0,      11,     1,	-1,	-1,	{PP_FUNCALL, PREC_FN, 0}},
{"save.to.file", CF(do_savefile),   0,      11,     3,	-1,	-1,	{PP_FUNCALL, PREC_FN, 0}},
{"putconst", CF(do_putconst),       0,      11,     2,	-1,	-1,     {PP_FUNCALL, PREC_FN, 0}},
#endif

/* Connections */
{"stdin", 	CF(do_stdin),	0,      11,     0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"stdout", 	CF(do_stdout),	0,      11,     0,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"stderr", 	CF(do_stderr),	0,      11,     0,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"readLines", 	CF(do_readLines),0,     11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"writeLines", 	CF(do_writelines),0,    11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"readBin", 	CF(do_readbin),	0,      11,     6,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"writeBin", 	CF(do_writebin),0,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"readChar", 	CF(do_readchar),0,      11,     2,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"writeChar", 	CF(do_writechar),0,	11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"open", 	CF(do_open),	0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"isOpen", 	CF(do_isopen),	0,      11,     2,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"isIncomplete",CF(do_isincomplete),0,  11,     1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"isSeekable", 	CF(do_isseekable),0,	11,     1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"close", 	CF(do_close),	0,      11,     2,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"flush", 	CF(do_flush),	0,      11,     1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"file", 	CF(do_url),	1,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"url", 	CF(do_url),	0,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"pipe", 	CF(do_pipe),	0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"fifo", 	CF(do_fifo),	0,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"gzfile", 	CF(do_gzfile),	0,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"unz", 	CF(do_unz),	0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"bzfile", 	CF(do_bzfile),	0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"seek", 	CF(do_seek),	0,      11,     4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"truncate", 	CF(do_truncate),0,      11,     1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"pushBack", 	CF(do_pushback),0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"clearPushBackLength",CF(do_clearpushback),0,11,1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"pushBackLength",CF(do_pushbacklength),0, 11,  1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"textConnection",CF(do_textconnection),0, 11,  4,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"socketConnection",CF(do_sockconn),0,	11,     6,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"sockSelect",CF(do_sockselect),0,	11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"getAllConnections",CF(do_getallconnections),0,11,0,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"summary.connection",CF(do_sumconnection),0,11,1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"download", 	CF(do_download),0,      11,     5,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"nsl", 	CF(do_nsl),	0,      11,     1,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},
{"gzcon", 	CF(do_gzcon),	0,      11,     3,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},


{"readDCF", 	CF(do_readDCF),	0,      11,     2,	-1,	-1,     {PP_FUNCALL, PREC_FN,	0}},

{"getNumRtoCConverters", CF(do_getNumRtoCConverters),			0, 11, 0, -1, -1, {PP_FUNCALL, PREC_FN,0}},
{"getRtoCConverterDescriptions", CF(do_getRtoCConverterDescriptions),	0, 11, 0, -1, -1, {PP_FUNCALL, PREC_FN, 0}},
{"getRtoCConverterStatus", CF(do_getRtoCConverterStatus),		0, 11, 0, -1, -1, {PP_FUNCALL, PREC_FN, 0}},
{"setToCConverterActiveStatus", CF(do_setToCConverterActiveStatus),	0, 11, 2, -1, -1, {PP_FUNCALL, PREC_FN, 0}},
{"removeToCConverterActiveStatus", CF(do_setToCConverterActiveStatus),	1, 11, 1, -1, -1, {PP_FUNCALL, PREC_FN, 0}},

{"lockEnvironment", CF(do_lockEnv),	0, 11,  2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"environmentIsLocked",	CF(do_envIsLocked),0,11,1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"lockBinding", CF(do_lockBnd),		0, 11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"unlockBinding", CF(do_lockBnd),	1, 11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"bindingIsLocked", CF(do_bndIsLocked),	0, 11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"makeActiveBinding", CF(do_mkActiveBnd),0,11,	3,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"bindingIsActive", CF(do_bndIsActive),	0, 11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"mkUnbound",	CF(do_mkUnbound),	0, 11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"isNamespaceEnv",CF(do_isNSEnv),	0, 11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"registerNamespace",CF(do_regNS),	0, 11,	2,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"unregisterNamespace",CF(do_unregNS),	0, 11,  1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getRegisteredNamespace",CF(do_getRegNS),0,11,	1,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"getNamespaceRegistry",CF(do_getNSRegistry),0,11,0,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},
{"importIntoEnv",CF(do_importIntoEnv), 0, 11, 	4,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{"write.table",	CF(do_writetable),0,	111,	11,	-1,	-1,	{PP_FUNCALL, PREC_FN,	0}},

{NULL,		0, NULL,	0,	0,	0,	0,	0,	{0,	     PREC_FN,	0}},
#undef CF
};

int R_FunTab_NumEntries = sizeof(R_FunTab)/sizeof(FUNTAB) - 1;


SEXP do_primitive(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP name;
    int i;
    checkArity(op, args);
    name = CAR(args);
    if (!isString(name) || length(name) < 1 ||
	STRING_ELT(name, 0) == R_NilValue)
	errorcall(call, _("string argument required"));
    for (i = 0; R_FunTab[i].name; i++)
	if (strcmp(CHAR(STRING_ELT(name, 0)), R_FunTab[i].name) == 0) {
	    if ((R_FunTab[i].eval % 100 )/10)
		return mkPRIMSXP(i, R_FunTab[i].eval % 10);
	    else
		return mkPRIMSXP(i, R_FunTab[i].eval % 10);
	}
    errorcall(call, _("no such primitive function"));
    return(R_NilValue);		/* -Wall */
}

int StrToInternal(char *s)
{
    int i;
    for (i = 0; R_FunTab[i].name; i++)
	if (strcmp(s, R_FunTab[i].name) == 0) return i;
    return 0;
}

static void installFunTab(int i)
{
    if ((R_FunTab[i].eval % 100 )/10)
	SET_INTERNAL(install(R_FunTab[i].name),
		     mkPRIMSXP(i, R_FunTab[i].eval % 10));
    else
	SET_SYMVALUE(install(R_FunTab[i].name),
		     mkPRIMSXP(i, R_FunTab[i].eval % 10));
}

static void SymbolShortcuts()
{
    R_Bracket2Symbol = install("[[");
    R_BracketSymbol = install("[");
    R_BraceSymbol = install("{");
    R_TmpvalSymbol = install("*tmp*");
    R_ClassSymbol = install("class");
    R_DimNamesSymbol = install("dimnames");
    R_DimSymbol = install("dim");
    R_DollarSymbol = install("$");
    R_DotsSymbol = install("...");
    R_DropSymbol = install("drop");
    R_LevelsSymbol = install("levels");
    R_ModeSymbol = install("mode");
    R_NamesSymbol = install("names");
    R_NaRmSymbol = install("na.rm");
    R_RowNamesSymbol = install("row.names");
    R_SeedsSymbol = install(".Random.seed");
    R_LastvalueSymbol = install(".Last.value");
    R_TspSymbol = install("tsp");
    R_CommentSymbol = install("comment");
    R_SourceSymbol = install("source");
    R_DotEnvSymbol = install(".Environment");
    R_RecursiveSymbol = install("recursive");
    R_UseNamesSymbol = install("use.names");
}

extern SEXP framenames; /* from model.c */

/* initialize the symbol table */
void InitNames()
{
    int i;
    /* R_UnboundValue */
    R_UnboundValue = allocSExp(SYMSXP);
    SET_SYMVALUE(R_UnboundValue, R_UnboundValue);
    SET_PRINTNAME(R_UnboundValue, R_NilValue);
    SET_ATTRIB(R_UnboundValue, R_NilValue);
    /* R_MissingArg */
    R_MissingArg = allocSExp(SYMSXP);
    SET_SYMVALUE(R_MissingArg, R_MissingArg);
    SET_PRINTNAME(R_MissingArg, mkChar(""));
    SET_ATTRIB(R_MissingArg, R_NilValue);
    /* R_RestartToken */
    R_RestartToken = allocSExp(SYMSXP);
    SET_SYMVALUE(R_RestartToken, R_RestartToken);
    SET_PRINTNAME(R_RestartToken, mkChar(""));
    SET_ATTRIB(R_RestartToken, R_NilValue);
    /* Parser Structures */
    R_CommentSxp = R_NilValue;
    R_ParseText = R_NilValue;
    /* String constants (CHARSXP values */
    /* Note: changed from mkChar so mkChar can see if it is getting
       "NA" and then return NA_STRING rather than alloc a new CHAR */
    /* NA_STRING */
    NA_STRING = allocString(strlen("NA"));
    strcpy(CHAR(NA_STRING), "NA");
    R_print.na_string = NA_STRING;
    /* R_BlankString */
    R_BlankString = mkChar("");
    /* Initialize the symbol Table */
    if (!(R_SymbolTable = (SEXP *) malloc(HSIZE * sizeof(SEXP))))
	R_Suicide("couldn't allocate memory for symbol table");
    for (i = 0; i < HSIZE; i++)
	R_SymbolTable[i] = R_NilValue;
    /* Set up a set of globals so that a symbol table search can be
       avoided when matching something like dim or dimnames. */
    SymbolShortcuts();
    /*  Builtin Functions */
    for (i = 0; R_FunTab[i].name; i++)
	installFunTab(i);
    /*  Unbound values which are to be preserved through GCs */
    R_PreciousList = R_NilValue;
    framenames = R_NilValue;
#ifdef BYTECODE
    R_initialize_bcode();
#endif
}


/*  install - probe the symbol table */
/*  If "name" is not found, it is installed in the symbol table.
    The symbol corresponding to the string "name" is returned. */

SEXP install(char const *name)
{
    char buf[MAXIDSIZE+1];
    SEXP sym;
    int i, hashcode;

    if (*name == '\0')
	error(_("attempt to use zero-length variable name"));
    if (strlen(name) > MAXIDSIZE)
	error(_("symbol print-name too long"));
    strcpy(buf, name);
    hashcode = R_Newhashpjw(buf);
    i = hashcode % HSIZE;
    /* Check to see if the symbol is already present;  if it is, return it. */
    for (sym = R_SymbolTable[i]; sym != R_NilValue; sym = CDR(sym))
	if (strcmp(buf, CHAR(PRINTNAME(CAR(sym)))) == 0)
	    return (CAR(sym));
    /* Create a new symbol node and link it into the table. */
    sym = mkSYMSXP(mkChar(buf), R_UnboundValue);
    SET_HASHVALUE(PRINTNAME(sym), hashcode);
    SET_HASHASH(PRINTNAME(sym), 1);
    R_SymbolTable[i] = CONS(sym, R_SymbolTable[i]);
    return (sym);
}


/*  do_internal - This is the code for .Internal(). */

SEXP do_internal(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP s, fun;
    int save = R_PPStackTop, flag;
    checkArity(op, args);
    s = CAR(args);
    if (!isPairList(s))
	errorcall(call, _("invalid .Internal() argument"));
    fun = CAR(s);
    if (!isSymbol(fun))
	errorcall(call, _("invalid internal function"));
    if (INTERNAL(fun) == R_NilValue)
	errorcall(call, _("no internal function \"%s\""),
		  CHAR(PRINTNAME(fun)));
    args = CDR(s);
    if (TYPEOF(INTERNAL(fun)) == BUILTINSXP)
	args = evalList(args, env);
    PROTECT(args);
    flag = PRIMPRINT(INTERNAL(fun));
    R_Visible = 1 - flag;
    args = PRIMFUN(INTERNAL(fun)) (s, INTERNAL(fun), args, env);
    if (flag) R_Visible = 0;
    UNPROTECT(1);
    if (save != R_PPStackTop) {
	REprintf("stack imbalance in internal %s, %d then %d",
	       PRIMNAME(INTERNAL(fun)), save, R_PPStackTop);
    }
    return (args);
}
#undef __R_Names__
