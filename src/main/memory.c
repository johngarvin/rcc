/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1998--2004  The R Development Core Team.
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

/*
 *	This code implements a non-moving generational collector
 *      with two or three generations.
 *
 *	Memory allocated by R_alloc is maintained in a stack.  Code
 *	that R_allocs memory must use vmaxget and vmaxset to obtain
 *	and reset the stack pointer.
 */

/* <UTF8> char here is handled as a whole */

#define USE_RINTERNALS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(Win32) && defined(LEA_MALLOC)
#include <stddef.h>
extern void *Rm_malloc(size_t n);
extern void *Rm_calloc(size_t n_elements, size_t element_size);
extern void Rm_free(void * p);
extern void *Rm_realloc(void * p, size_t n);
#define calloc Rm_calloc
#define malloc Rm_malloc
#define realloc Rm_realloc
#define free Rm_free
#endif

#include <Defn.h>
#include <Graphics.h> /* display lists */
#include <Rdevices.h> /* GetDevice */

Rboolean global_dump_stats; /* = (getenv("R_DUMP_STATS") != NULL); */
                            /* initialized in InitMemory()         */
Rboolean global_stack_debug; /* = (getenv("R_STACK_DEBUG") != NULL) */
                             /* initialized in InitMemory()         */
Rboolean global_stack_info = FALSE;

const int global_default_alloc_stack_space_size = 4096;
int global_alloc_stack_space_size;  /* R_ALLOC_STACK_SPACE_SIZE    */
                                    /* initialized in InitMemory() */

/* when true, use heap allocation all the time as a fallback */
static Rboolean fallback_alloc = FALSE;


/* for debugging: print pointers of an S-expression and all its
   subfields. Only prints allocated expressions, not constants such as
   R_NilValue. To avoid infinite loops, does not print components of
   environments. */
void printAllPointers(SEXP e)
{
  int i;
  const int n_exclude = 6;

  /* these don't count */
  SEXP exclude[] = {R_NilValue, NA_STRING, R_UnboundValue, R_RestartToken, R_MissingArg, R_CommentSxp};
  for (i = 0; i < n_exclude; i++) {
    if (e == exclude[i]) {
      return;
    }
  }
  if (TYPEOF(e) == BUILTINSXP ||
      TYPEOF(e) == SPECIALSXP)
  {
    return;
  }
  
  fprintf(stderr, "0x%x ", e);
  switch(TYPEOF(e)) {
  case NILSXP:
  case CHARSXP:
  case LGLSXP:
  case INTSXP:
  case REALSXP:
  case CPLXSXP:
  case WEAKREFSXP:
  case RAWSXP:
  case RCC_FUNSXP:
  case DOTSXP:
  case SYMSXP:
  case BCODESXP:
    return;
  case STRSXP:
  case EXPRSXP:
  case VECSXP:
    for (i = 0; i < LENGTH(e); i++) {
      printAllPointers(STRING_ELT(e, i));
    }
    break;
  case ENVSXP:
    /*
    printAllPointers(FRAME(e));
    printAllPointers(ENCLOS(e));
    printAllPointers(HASHTAB(e));
    */
    break;
  case CLOSXP:
    printAllPointers(FORMALS(e));
    printAllPointers(BODY(e));
    printAllPointers(CLOENV(e));
    break;
  case RCC_CLOSXP:
    printAllPointers(RCC_CLOSXP_FORMALS(e));
    printAllPointers(RCC_CLOSXP_CLOENV(e));
    break;
  case PROMSXP:
    printAllPointers(PRCODE(e));
    printAllPointers(PRENV(e));
    printAllPointers(PRVALUE(e));
    break;
  case LISTSXP:
  case LANGSXP:
    printAllPointers(TAG(e));
    printAllPointers(CAR(e));
    printAllPointers(CDR(e));
    break;
  case EXTPTRSXP:
    printAllPointers(EXTPTR_PROT(e));
    printAllPointers(EXTPTR_TAG(e));
    break;
  default:
    error(_("printAllPointers: unrecognized type"));
  }
}

/* malloc uses size_t.  We are assuming here that size_t is at least
   as large as unsigned long.  Changed from int at 1.6.0 to (i) allow
   2-4Gb objects on 32-bit system and (ii) objects limited only by
   length on a 64-bit system.
*/

static int gc_reporting = 0;
static int gc_count = 0;

#define GC_TORTURE

#ifdef GC_TORTURE
# define FORCE_GC !gc_inhibit_torture
#else
# define FORCE_GC 0
#endif

extern SEXP framenames;

#define GC_PROT(X) {int __t = gc_inhibit_torture; \
	gc_inhibit_torture = 1 ; X ; gc_inhibit_torture = __t;}

static void R_gc_internal(R_size_t size_needed);
static void mem_err_heap(R_size_t size);

static SEXPREC UnmarkedNodeTemplate;
#define NODE_IS_MARKED(s) (MARK(s)==1)
#define MARK_NODE(s) (MARK(s)=1)
#define UNMARK_NODE(s) (MARK(s)=0)


/* Tuning Constants. Most of these could be made settable from R,
   within some reasonable constraints at least.  Since there are quite
   a lot of constants it would probably make sense to put together
   several "packages" representing different space/speed tradeoffs
   (e.g. very aggressive freeing and small increments to conserve
   memory; much less frequent releasing and larger increments to
   increase speed). */

/* There are three levels of collections.  Level 0 collects only the
   youngest generation, level 1 collects the two youngest generations,
   and level 2 collects all generations.  Higher level collections
   occur at least after specified numbers of lower level ones.  After
   LEVEL_0_FREQ level zero collections a level 1 collection is done;
   after every LEVEL_1_FREQ level 1 collections a level 2 collection
   occurs.  Thus, roughly, every LEVEL_0_FREQ-th collection is a level
   1 collection and every (LEVEL_0_FREQ * LEVEL_1_FREQ)-th collection
   is a level 2 collection.  */
#define LEVEL_0_FREQ 20
#define LEVEL_1_FREQ 5
static int collect_counts_max[] = { LEVEL_0_FREQ, LEVEL_1_FREQ };

/* When a level N collection fails to produce at least MinFreeFrac *
   R_NSize free nodes and MinFreeFrac * R_VSize free vector space, the
   next collection will be a level N + 1 collection.

   This constant is also used in heap size adjustment as a minimal
   fraction of the minimal heap size levels that should be available
   for allocation. */
static double R_MinFreeFrac = 0.2;

/* When pages are released, a number of free nodes equal to
   R_MaxKeepFrac times the number of allocated nodes for each class is
   retained.  Pages not needed to meet this requirement are released.
   An attempt to release pages is made every R_PageReleaseFreq level 1
   or level 2 collections. */
static double R_MaxKeepFrac = 0.5;
static int R_PageReleaseFreq = 1;

/* The heap size constants R_NSize and R_VSize are used for triggering
   collections.  The initial values set by defaults or command line
   arguments are used as minimal values.  After full collections these
   levels are adjusted up or down, though not below the minimal values
   or above the maximum values, towards maintain heap occupancy within
   a specified range.  When the number of nodes in use reaches
   R_NGrowFrac * R_NSize, the value of R_NSize is incremented by
   R_NGrowIncrMin + R_NGrowIncrFrac * R_NSize.  When the number of
   nodes in use falls below R_NShrinkFrac, R_NSize is decremented by
   R_NShrinkIncrMin * R_NShrinkFrac * R_NSize.  Analogous adjustments
   are made to R_VSize.

   This mechanism for adjusting the heap size constants is very
   primitive but hopefully adequate for now.  Some modeling and
   experimentation would be useful.  We want the heap sizes to get set
   at levels adequate for the current computations.  The present
   mechanism uses only the size of the current live heap to provide
   information about the current needs; since the current live heap
   size can be very volatile, the adjustment mechanism only makes
   gradual adjustments.  A more sophisticated strategy would use more
   of the live heap history. */
static double R_NGrowFrac = 0.70;
static double R_NShrinkFrac = 0.30;

static double R_VGrowFrac = 0.70;
static double R_VShrinkFrac = 0.30;

#ifdef SMALL_MEMORY
/* On machines with only 32M of memory (or on a classic Mac OS port)
   it might be a good idea to use settings like these that are more
   aggressive at keeping memory usage down. */
static double R_NGrowIncrFrac = 0.0, R_NShrinkIncrFrac = 0.2;
static int R_NGrowIncrMin = 50000, R_NShrinkIncrMin = 0;
static double R_VGrowIncrFrac = 0.0, R_VShrinkIncrFrac = 0.2;
static int R_VGrowIncrMin = 100000, R_VShrinkIncrMin = 0;
#else
static double R_NGrowIncrFrac = 0.05, R_NShrinkIncrFrac = 0.2;
static int R_NGrowIncrMin = 40000, R_NShrinkIncrMin = 0;
static double R_VGrowIncrFrac = 0.05, R_VShrinkIncrFrac = 0.2;
static int R_VGrowIncrMin = 80000, R_VShrinkIncrMin = 0;
#endif

/* Maximal Heap Limits.  These variables contain upper limits on the
   heap sizes.  They could be made adjustable from the R level,
   perhaps by a handler for a recoverable error.

   Access to these values is provided with reader and writer
   functions; the writer function insures that the maximal values are
   never set below the current ones. */
static R_size_t R_MaxVSize = R_SIZE_T_MAX;
static R_size_t R_MaxNSize = R_SIZE_T_MAX;
static int vsfac = 1; /* current units for vsize: changes at initialization */

R_size_t R_GetMaxVSize(void) 
{
    if (R_MaxVSize == R_SIZE_T_MAX) return R_SIZE_T_MAX;
    return R_MaxVSize*vsfac;
}

void R_SetMaxVSize(R_size_t size)
{
    if (size == R_SIZE_T_MAX) return;
    if (size / vsfac >= R_VSize) R_MaxVSize = (size+1)/sizeof(VECREC);
}

R_size_t R_GetMaxNSize(void) 
{ 
    return R_MaxNSize;
}

void R_SetMaxNSize(R_size_t size)
{
    if (size >= R_NSize) R_MaxNSize = size;
}

void R_SetPPSize(unsigned long size)
{
    R_PPStackSize = size;
}

/* Miscellaneous Globals. */

static SEXP R_VStack = NULL;		/* R_alloc stack pointer */
static R_size_t R_LargeVallocSize = 0;
static R_size_t R_SmallVallocSize = 0;
static R_size_t orig_R_NSize;
static R_size_t orig_R_VSize;

static R_size_t R_N_maxused=0;
static R_size_t R_V_maxused=0;

/* Node Classes.  Non-vector nodes are of class zero. Small vector
   nodes are in classes 1, ..., NUM_SMALL_NODE_CLASSES, and large
   vector nodes are in class LARGE_NODE_CLASS.  For vector nodes the
   node header is followed in memory by the vector data, offset from
   the header by SEXPREC_ALIGN. */

#define NUM_NODE_CLASSES 8

/* sxpinfo allocates 3 bits for the node class, so at most 8 are allowed */
#if NUM_NODE_CLASSES > 8
# error NUM_NODE_CLASSES must be at most 8
#endif

#define LARGE_NODE_CLASS (NUM_NODE_CLASSES - 1)
#define NUM_SMALL_NODE_CLASSES (NUM_NODE_CLASSES - 1)

/* the number of VECREC's in nodes of the small node classes */
static int NodeClassSize[NUM_SMALL_NODE_CLASSES] = { 0, 1, 2, 4, 6, 8, 16 };

#define NODE_CLASS(s) ((s)->sxpinfo.gccls)
#define SET_NODE_CLASS(s,v) (((s)->sxpinfo.gccls) = (v))


/* Node Generations. */

#define NUM_OLD_GENERATIONS 2

/* sxpinfo allocates one bit for the old generation count, so only 1
   or 2 is allowed */
#if NUM_OLD_GENERATIONS > 2 || NUM_OLD_GENERATIONS < 1
# error number of old generations must be 1 or 2
#endif

#define NODE_GENERATION(s) ((s)->sxpinfo.gcgen)
#define SET_NODE_GENERATION(s,g) ((s)->sxpinfo.gcgen=(g))

#define NODE_GEN_IS_YOUNGER(s,g) \
  (! NODE_IS_MARKED(s) || NODE_GENERATION(s) < (g))
#define NODE_IS_OLDER(x, y) \
  (NODE_IS_MARKED(x) && \
   (! NODE_IS_MARKED(y) || NODE_GENERATION(x) > NODE_GENERATION(y)))

static int num_old_gens_to_collect = 0;
static int gen_gc_counts[NUM_OLD_GENERATIONS + 1];
static int collect_counts[NUM_OLD_GENERATIONS];


/* Node Pages.  Non-vector nodes and small vector nodes are allocated
   from fixed size pages.  The pages for each node class are kept in a
   linked list. */

typedef union PAGE_HEADER {
  union PAGE_HEADER *next;
  double align;
} PAGE_HEADER;

#define BASE_PAGE_SIZE 2000
#define R_PAGE_SIZE \
  (((BASE_PAGE_SIZE - sizeof(PAGE_HEADER)) / sizeof(SEXPREC)) \
   * sizeof(SEXPREC) \
   + sizeof(PAGE_HEADER))
#define NODE_SIZE(c) \
  ((c) == 0 ? sizeof(SEXPREC) : \
   sizeof(SEXPREC_ALIGN) + NodeClassSize[c] * sizeof(VECREC))

#define PAGE_DATA(p) ((void *) (p + 1))
#define VHEAP_FREE() (R_VSize - R_LargeVallocSize - R_SmallVallocSize)


/* The Heap Structure.  Nodes for each class/generation combination
   are arranged in circular doubly-linked lists.  The double linking
   allows nodes to be removed in constant time; this is used by the
   collector to move reachable nodes out of free space and into the
   appropriate generation.  The circularity eliminates the need for
   end checks.  In addition, each link is anchored at an artificial
   node, the Peg SEXPREC's in the structure below, which simplifies
   pointer maintenance.  The circular doubly-linked arrangement is
   taken from Baker's in-place incremental collector design; see
   ftp://ftp.netcom.com/pub/hb/hbaker/NoMotionGC.html or the Jones and
   Lins GC book.  The linked lists are implemented by adding two
   pointer fields to the SEXPREC structure, which increases its size
   from 5 to 7 words. Other approaches are possible but don't seem
   worth pursuing for R.

   There are two options for dealing with old-to-new pointers.  The
   first option is to make sure they never occur by transferring all
   referenced younger objects to the generation of the referrer when a
   reference to a newer object is assigned to an older one.  This is
   enabled by defining EXPEL_OLD_TO_NEW.  The second alternative is to
   keep track of all nodes that may contain references to newer nodes
   and to "age" the nodes they refer to at the beginning of each
   collection.  This is the default.  The first option is simpler in
   some ways, but will create more floating garbage and add a bit to
   the execution time, though the difference is probably marginal on
   both counts.*/
/*#define EXPEL_OLD_TO_NEW*/
static struct {
    SEXP Old[NUM_OLD_GENERATIONS], New, Free;
    SEXPREC OldPeg[NUM_OLD_GENERATIONS], NewPeg;
#ifndef EXPEL_OLD_TO_NEW
    SEXP OldToNew[NUM_OLD_GENERATIONS];
    SEXPREC OldToNewPeg[NUM_OLD_GENERATIONS];
#endif
    int OldCount[NUM_OLD_GENERATIONS], AllocCount, PageCount;
    PAGE_HEADER *pages;
} R_GenHeap[NUM_NODE_CLASSES];

static R_size_t R_NodesInUse = 0;

#define NEXT_NODE(s) (s)->gengc_next_node
#define PREV_NODE(s) (s)->gengc_prev_node
#define SET_NEXT_NODE(s,t) (NEXT_NODE(s) = (t))
#define SET_PREV_NODE(s,t) (PREV_NODE(s) = (t))


/* Node List Manipulation */

/* unsnap node s from its list */

static void UNSNAP_NODE(SEXP s) {
  SEXP un__n__ = (s);
  SEXP next = NEXT_NODE(un__n__);
  SEXP prev = PREV_NODE(un__n__);
  if (!prev) error(_("UNSNAP_NODE: non-heap object in heap"));
  SET_NEXT_NODE(prev, next);
  SET_PREV_NODE(next, prev);
}

/*
#define UNSNAP_NODE(s) do { \
  SEXP un__n__ = (s); \
  SEXP next = NEXT_NODE(un__n__); \
  SEXP prev = PREV_NODE(un__n__); \
  if (!prev) error(_("UNSNAP_NODE: non-heap object in heap")); \
  SET_NEXT_NODE(prev, next); \
  SET_PREV_NODE(next, prev); \
} while(0)
*/

/* snap in node s before node t */

static void SNAP_NODE(SEXP s, SEXP t) {
  SEXP sn__n__ = (s);
  SEXP next = (t);
  SEXP prev = PREV_NODE(next);
  /*
    temporary debugging hack for andre1 --garvin

  if ((unsigned long)s > 0x700000000000) error(_("SNAP_NODE: non-heap object in heap"));
  if ((unsigned long)t > 0x700000000000) error(_("SNAP_NODE: non-heap object in heap"));
  */
  SET_NEXT_NODE(sn__n__, next);
  SET_PREV_NODE(next, sn__n__);
  SET_NEXT_NODE(prev, sn__n__);
  SET_PREV_NODE(sn__n__, prev);
}

/*
#define SNAP_NODE(s,t) do { \
  SEXP sn__n__ = (s); \
  SEXP next = (t); \
  SEXP prev = PREV_NODE(next);	\
  SET_NEXT_NODE(sn__n__, next); \
  SET_PREV_NODE(next, sn__n__); \
  SET_NEXT_NODE(prev, sn__n__); \
  SET_PREV_NODE(sn__n__, prev); \
} while (0)
*/


/* move all nodes on from_peg to to_peg */
#define BULK_MOVE(from_peg,to_peg) do { \
  SEXP __from__ = (from_peg); \
  SEXP __to__ = (to_peg); \
  SEXP first_old = NEXT_NODE(__from__); \
  SEXP last_old = PREV_NODE(__from__); \
  SEXP first_new = NEXT_NODE(__to__); \
  SET_PREV_NODE(first_old, __to__); \
  SET_NEXT_NODE(__to__, first_old); \
  SET_PREV_NODE(first_new, last_old); \
  SET_NEXT_NODE(last_old, first_new); \
  SET_NEXT_NODE(__from__, __from__); \
  SET_PREV_NODE(__from__, __from__); \
} while (0);


/* Processing Node Children */

/* This macro calls dc__action__ for each child of __n__, passing
   dc__extra__ as a second argument for each call. */
#define DO_CHILDREN(__n__,dc__action__,dc__extra__) do { \
  if (ATTRIB(__n__) != R_NilValue) \
    dc__action__(ATTRIB(__n__), dc__extra__); \
  switch (TYPEOF(__n__)) { \
  case NILSXP: \
  case BUILTINSXP: \
  case SPECIALSXP: \
  case CHARSXP: \
  case LGLSXP: \
  case INTSXP: \
  case REALSXP: \
  case CPLXSXP: \
  case WEAKREFSXP: \
  case RAWSXP: \
  case RCC_FUNSXP: \
    break; \
  case STRSXP: \
  case EXPRSXP: \
  case VECSXP: \
    { \
      int i; \
      for (i = 0; i < LENGTH(__n__); i++) \
        dc__action__(STRING_ELT(__n__, i), dc__extra__); \
    } \
    break; \
  case ENVSXP: \
    dc__action__(FRAME(__n__), dc__extra__); \
    dc__action__(ENCLOS(__n__), dc__extra__); \
    dc__action__(HASHTAB(__n__), dc__extra__); \
    break; \
  case CLOSXP: \
  case RCC_CLOSXP: \
  case PROMSXP: \
  case LISTSXP: \
  case LANGSXP: \
  case DOTSXP: \
  case SYMSXP: \
  case BCODESXP: \
    dc__action__(TAG(__n__), dc__extra__); \
    dc__action__(CAR(__n__), dc__extra__); \
    dc__action__(CDR(__n__), dc__extra__); \
    break; \
  case EXTPTRSXP: \
    dc__action__(EXTPTR_PROT(__n__), dc__extra__); \
    dc__action__(EXTPTR_TAG(__n__), dc__extra__); \
    break; \
  default: \
    abort(); \
  } \
} while(0)


/* Forwarding Nodes.  These macros mark nodes or children of nodes and
   place them on the forwarding list.  The forwarding list is assumed
   to be in a local variable of the caller named named
   forwarded_nodes. */

/* added clause not to forward if GC pointer is null; prevents stack
   allocated objects from being added to heap. We _do_ want stack
   allocated objects to be added to forwarded_nodes so their children
   can be processed. --garvin */

/*
#define FORWARD_NODE(s) do {				\
	SEXP fn__n__ = (s);				\
	if (fn__n__ && ! NODE_IS_MARKED(fn__n__)) { \
	    MARK_NODE(fn__n__);						\
	    if (PREV_NODE(fn__n__) != NULL) {				\
		UNSNAP_NODE(fn__n__);					\
	    }								\
	    SET_NEXT_NODE(fn__n__, forwarded_nodes);			\
	    forwarded_nodes = fn__n__;					\
	}						\
    } while (0)
*/

#define FORWARD_NODE(s) FORWARD_NODE_f(s, &forwarded_nodes)

void FORWARD_NODE_f(SEXP s, SEXP * forwarded_nodes) {
    SEXP fn__n__ = (s); 
    if (fn__n__ && ! NODE_IS_MARKED(fn__n__)) {
	MARK_NODE(fn__n__);
	if (PREV_NODE(fn__n__) != NULL) {
	    UNSNAP_NODE(fn__n__);
	}
	SET_NEXT_NODE(fn__n__, *forwarded_nodes);
	*forwarded_nodes = fn__n__;
    }
}

#define FC_FORWARD_NODE_f(__n__,__dummy__) FORWARD_NODE_f(__n__,forwarded_nodes)
#define FC_FORWARD_NODE(__n__,__dummy__) FORWARD_NODE(__n__)
#define FORWARD_CHILDREN_f(__n__) DO_CHILDREN(__n__,FC_FORWARD_NODE_f, 0)
#define FORWARD_CHILDREN(__n__) DO_CHILDREN(__n__,FC_FORWARD_NODE, 0)


/* Node Allocation. */

#define CLASS_GET_FREE_NODE(c,s) do { \
  SEXP __n__ = R_GenHeap[c].Free; \
  if (__n__ == R_GenHeap[c].New) { \
    GetNewPage(c); \
    __n__ = R_GenHeap[c].Free; \
  } \
  R_GenHeap[c].Free = NEXT_NODE(__n__); \
  R_NodesInUse++; \
  (s) = __n__; \
} while (0)

#define NO_FREE_NODES() (R_NodesInUse >= R_NSize)
#define GET_FREE_NODE(s) CLASS_GET_FREE_NODE(0,s)


/* Debugging Routines. */

#ifdef DEBUG_GC
static void CheckNodeGeneration(SEXP x, int g)
{
    if (NODE_GENERATION(x) < g) {
	REprintf("untraced old-to-new reference\n");
    }
}

static void DEBUG_CHECK_NODE_COUNTS(char *where)
{
    int i, OldCount, NewCount, OldToNewCount, gen;
    SEXP s;

    REprintf("Node counts %s:\n", where);
    for (i = 0; i < NUM_NODE_CLASSES; i++) {
	for (s = NEXT_NODE(R_GenHeap[i].New), NewCount = 0;
	     s != R_GenHeap[i].New;
	     s = NEXT_NODE(s)) {
	    NewCount++;
	    if (i != NODE_CLASS(s))
		REprintf("Inconsistent class assignment for node!\n");
	}
	for (gen = 0, OldCount = 0, OldToNewCount = 0;
	     gen < NUM_OLD_GENERATIONS;
	     gen++) {
	    for (s = NEXT_NODE(R_GenHeap[i].Old[gen]);
		 s != R_GenHeap[i].Old[gen];
		 s = NEXT_NODE(s)) {
		OldCount++;
		if (i != NODE_CLASS(s))
		    REprintf("Inconsistent class assignment for node!\n");
		if (gen != NODE_GENERATION(s))
		    REprintf("Inconsistent node generation\n");
		DO_CHILDREN(s, CheckNodeGeneration, gen);
	    }
	    for (s = NEXT_NODE(R_GenHeap[i].OldToNew[gen]);
		 s != R_GenHeap[i].OldToNew[gen];
		 s = NEXT_NODE(s)) {
		OldToNewCount++;
		if (i != NODE_CLASS(s))
		    REprintf("Inconsistent class assignment for node!\n");
		if (gen != NODE_GENERATION(s))
		    REprintf("Inconsistent node generation\n");
	    }
	}
	REprintf("Class: %d, New = %d, Old = %d, OldToNew = %d, Total = %d\n",
		 i,
		 NewCount, OldCount, OldToNewCount,
		 NewCount + OldCount + OldToNewCount);
    }
}

static void DEBUG_GC_SUMMARY(int full_gc)
{
    int i, gen, OldCount;
    REprintf("\n%s, VSize = %lu", full_gc ? "Full" : "Minor",
	     R_SmallVallocSize + R_LargeVallocSize);
    for (i = 1; i < NUM_NODE_CLASSES; i++) {
	for (gen = 0, OldCount = 0; gen < NUM_OLD_GENERATIONS; gen++)
	    OldCount += R_GenHeap[i].OldCount[gen];
	REprintf(", class %d: %d", i, OldCount);
    }
}
#else
#define DEBUG_CHECK_NODE_COUNTS(s)
#define DEBUG_GC_SUMMARY(x)
#endif /* DEBUG_GC */

#ifdef DEBUG_ADJUST_HEAP
static void DEBUG_ADJUST_HEAP_PRINT(double node_occup, double vect_occup)
{
    int i;
    R_size_t alloc;
    REprintf("Node occupancy: %.0f%%\nVector occupancy: %.0f%%\n",
	     100.0 * node_occup, 100.0 * vect_occup);
    alloc = R_LargeVallocSize +
	sizeof(SEXPREC_ALIGN) * R_GenHeap[LARGE_NODE_CLASS].AllocCount;
    for (i = 0; i < NUM_SMALL_NODE_CLASSES; i++)
	alloc += R_PAGE_SIZE * R_GenHeap[i].PageCount;
    REprintf("Total allocation: %lu\n", alloc);
    REprintf("Ncells %lu\nVcells %lu\n", R_NSize, R_VSize);
}
#else
#define DEBUG_ADJUST_HEAP_PRINT(node_occup, vect_occup)
#endif /* DEBUG_ADJUST_HEAP */

#ifdef DEBUG_RELEASE_MEM
static void DEBUG_RELEASE_PRINT(int rel_pages, int maxrel_pages, int i)
{
    if (maxrel_pages > 0) {
	int gen, n;
	REprintf("Class: %d, pages = %d, maxrel = %d, released = %d\n", i,
		 R_GenHeap[i].PageCount, maxrel_pages, rel_pages);
	for (gen = 0, n = 0; gen < NUM_OLD_GENERATIONS; gen++)
	    n += R_GenHeap[i].OldCount[gen];
	REprintf("Allocated = %d, in use = %d\n", R_GenHeap[i].AllocCount, n);
    }
}
#else
#define DEBUG_RELEASE_PRINT(rel_pages, maxrel_pages, i)
#endif /* DEBUG_RELEASE_MEM */


/* Page Allocation and Release. */

static void GetNewPage(int node_class)
{
    SEXP s, base;
    char *data;
    PAGE_HEADER *page;
    int node_size, page_count, i;

    node_size = NODE_SIZE(node_class);
    page_count = (R_PAGE_SIZE - sizeof(PAGE_HEADER)) / node_size;

    page = malloc(R_PAGE_SIZE);
    if (page == NULL)
	mem_err_heap((R_size_t) NodeClassSize[node_class]);
    page->next = R_GenHeap[node_class].pages;
    R_GenHeap[node_class].pages = page;
    R_GenHeap[node_class].PageCount++;

    data = PAGE_DATA(page);
    base = R_GenHeap[node_class].New;
    for (i = 0; i < page_count; i++, data += node_size) {
	s = (SEXP) data;
	R_GenHeap[node_class].AllocCount++;
	SNAP_NODE(s, base);
	s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	SET_NODE_CLASS(s, node_class);
	base = s;
	R_GenHeap[node_class].Free = s;
    }
}

static void ReleasePage(PAGE_HEADER *page, int node_class)
{
    SEXP s;
    char *data;
    int node_size, page_count, i;

    node_size = NODE_SIZE(node_class);
    page_count = (R_PAGE_SIZE - sizeof(PAGE_HEADER)) / node_size;
    data = PAGE_DATA(page);

    for (i = 0; i < page_count; i++, data += node_size) {
	s = (SEXP) data;
	UNSNAP_NODE(s);
	R_GenHeap[node_class].AllocCount--;
    }
    R_GenHeap[node_class].PageCount--;

    /* added to debug memory corruption errors */
    if (global_stack_debug) {
	memset(page, 0xfa, R_PAGE_SIZE);
    }
    free(page);
}

static void TryToReleasePages(void)
{
    SEXP s;
    int i;
    static int release_count = 0;

    if (release_count == 0) {
	release_count = R_PageReleaseFreq;
	for (i = 0; i < NUM_SMALL_NODE_CLASSES; i++) {
	    int pages_free = 0;
	    PAGE_HEADER *page, *last, *next;
	    int node_size = NODE_SIZE(i);
	    int page_count = (R_PAGE_SIZE - sizeof(PAGE_HEADER)) / node_size;
	    int maxrel, maxrel_pages, rel_pages, gen;

	    maxrel = R_GenHeap[i].AllocCount;
	    for (gen = 0; gen < NUM_OLD_GENERATIONS; gen++)
		maxrel -= (1.0 + R_MaxKeepFrac) * R_GenHeap[i].OldCount[gen];
	    maxrel_pages = maxrel > 0 ? maxrel / page_count : 0;

	    /* all nodes in New space should be both free and unmarked */
	    for (page = R_GenHeap[i].pages, rel_pages = 0, last = NULL;
		 rel_pages < maxrel_pages && page != NULL;) {
		int j, in_use;
		char *data = PAGE_DATA(page);

		next = page->next;
		for (in_use = 0, j = 0; j < page_count;
		     j++, data += node_size) {
		    s = (SEXP) data;
		    if (NODE_IS_MARKED(s)) {
			in_use = 1;
			break;
		    }
		}
		if (! in_use) {
		    ReleasePage(page, i);
		    if (last == NULL)
			R_GenHeap[i].pages = next;
		    else
			last->next = next;
		    pages_free++;
		    rel_pages++;
		}
		else last = page;
		page = next;
	    }
	    DEBUG_RELEASE_PRINT(rel_pages, maxrel_pages, i);
	    R_GenHeap[i].Free = NEXT_NODE(R_GenHeap[i].New);
	}
    }
    else release_count--;
}

static void ReleaseLargeFreeVectors(void)
{
    SEXP s = NEXT_NODE(R_GenHeap[LARGE_NODE_CLASS].New);
    while (s != R_GenHeap[LARGE_NODE_CLASS].New) {
	SEXP next = NEXT_NODE(s);
	if (CHAR(s) != NULL) {
	    R_size_t size, size_b;
	    switch (TYPEOF(s)) {	/* get size in bytes */
	    case CHARSXP:
		size_b = LENGTH(s) + 1;
		break;
	    case RAWSXP:
		size_b = LENGTH(s);
		break;
	    case LGLSXP:
	    case INTSXP:
		size_b = LENGTH(s) * sizeof(int);
		break;
	    case REALSXP:
		size_b = LENGTH(s) * sizeof(double);
		break;
	    case CPLXSXP:
		size_b = LENGTH(s) * sizeof(Rcomplex);
		break;
	    case STRSXP:
	    case EXPRSXP:
	    case VECSXP:
		size_b = LENGTH(s) * sizeof(SEXP);
		break;
	    default:
		abort();
	    }
	    size = BYTE2VEC(size_b);
	    UNSNAP_NODE(s);
	    R_LargeVallocSize -= size;
	    R_GenHeap[LARGE_NODE_CLASS].AllocCount--;
	    if (global_stack_debug) {
		memset(s, 0xfa, size_b);
	    }
	    free(s);
	}
	s = next;
    }
}


/* Heap Size Adjustment. */

static void AdjustHeapSize(R_size_t size_needed)
{
    R_size_t R_MinNFree = orig_R_NSize * R_MinFreeFrac;
    R_size_t R_MinVFree = orig_R_VSize * R_MinFreeFrac;
    R_size_t NNeeded = R_NodesInUse + R_MinNFree;
    R_size_t VNeeded = R_SmallVallocSize + R_LargeVallocSize
	+ size_needed + R_MinVFree;
    double node_occup = ((double) NNeeded) / R_NSize;
    double vect_occup =	((double) VNeeded) / R_VSize;

    if (node_occup > R_NGrowFrac) {
	R_size_t change = R_NGrowIncrMin + R_NGrowIncrFrac * R_NSize;
	if (R_MaxNSize >= R_NSize + change)
	    R_NSize += change;
    }
    else if (node_occup < R_NShrinkFrac) {
	R_NSize -= (R_NShrinkIncrMin + R_NShrinkIncrFrac * R_NSize);
	if (R_NSize < NNeeded)
	    R_NSize = (NNeeded < R_MaxNSize) ? NNeeded: R_MaxNSize;
	if (R_NSize < orig_R_NSize)
	    R_NSize = orig_R_NSize;
    }

    if (vect_occup > 1.0 && VNeeded < R_MaxVSize)
	R_VSize = VNeeded;
    if (vect_occup > R_VGrowFrac) {
	R_size_t change = R_VGrowIncrMin + R_VGrowIncrFrac * R_NSize;
	if (R_MaxVSize - R_VSize >= change)
	    R_VSize += change;
    }
    else if (vect_occup < R_VShrinkFrac) {
	R_VSize -= R_VShrinkIncrMin + R_VShrinkIncrFrac * R_VSize;
	if (R_VSize < VNeeded)
	    R_VSize = VNeeded;
	if (R_VSize < orig_R_VSize)
	    R_VSize = orig_R_VSize;
    }

    DEBUG_ADJUST_HEAP_PRINT(node_occup, vect_occup);
}


/* Managing Old-to-New References. */

#define AGE_NODE(s,g) do { \
  SEXP an__n__ = (s); \
  int an__g__ = (g); \
  if (an__n__ && NODE_GEN_IS_YOUNGER(an__n__, an__g__)) { \
    if (NODE_IS_MARKED(an__n__) && PREV_NODE(an__n__) != NULL)	\
      R_GenHeap[NODE_CLASS(an__n__)].OldCount[NODE_GENERATION(an__n__)]--; \
    else \
      MARK_NODE(an__n__); \
    SET_NODE_GENERATION(an__n__, an__g__); \
    if (PREV_NODE(an__n__) != NULL) \
	UNSNAP_NODE(an__n__);		     \
    SET_NEXT_NODE(an__n__, forwarded_nodes); \
    forwarded_nodes = an__n__; \
  } \
} while (0)

static void AgeNodeAndChildren(SEXP s, int gen)
{
    SEXP forwarded_nodes = NULL;
    AGE_NODE(s, gen);
    while (forwarded_nodes != NULL) {
	s = forwarded_nodes;
	forwarded_nodes = NEXT_NODE(forwarded_nodes);
	if (NODE_GENERATION(s) != gen)
	    REprintf("****snapping into wrong generation\n");
	if (PREV_NODE(s) != NULL) {
	    SNAP_NODE(s, R_GenHeap[NODE_CLASS(s)].Old[gen]);
	    R_GenHeap[NODE_CLASS(s)].OldCount[gen]++;
	}
	DO_CHILDREN(s, AGE_NODE, gen);
    }
}

static void old_to_new(SEXP x, SEXP y)
{
    /*    if (!PREV_NODE(x) || !PREV_NODE(y)) error(_("old_to_new: non-heap object in heap")); */
    if (!PREV_NODE(x) || !PREV_NODE(y)) return;
#ifdef EXPEL_OLD_TO_NEW
    AgeNodeAndChildren(y, NODE_GENERATION(x));
#else
    if (PREV_NODE(x) != NULL) {
	UNSNAP_NODE(x);
	SNAP_NODE(x, R_GenHeap[NODE_CLASS(x)].OldToNew[NODE_GENERATION(x)]);
    }
#endif
}

#define CHECK_OLD_TO_NEW(x,y) do { \
  if (NODE_IS_OLDER(x, y)) old_to_new(x,y);  } while (0)


/* Node Sorting.  SortNodes attempts to improve locality of reference
   by rearranging the free list to place nodes on the same place page
   together and order nodes within pages.  This involves a sweep of the
   heap, so it should not be done too often, but doing it at least
   occasionally does seem essential.  Sorting on each full colllection is
   probably sufficient.
*/

#define SORT_NODES
#ifdef SORT_NODES
static void SortNodes(void)
{
    SEXP s;
    int i;

    for (i = 0; i < NUM_SMALL_NODE_CLASSES; i++) {
	PAGE_HEADER *page;
	int node_size = NODE_SIZE(i);
	int page_count = (R_PAGE_SIZE - sizeof(PAGE_HEADER)) / node_size;

	SET_NEXT_NODE(R_GenHeap[i].New, R_GenHeap[i].New);
	SET_PREV_NODE(R_GenHeap[i].New, R_GenHeap[i].New);
	for (page = R_GenHeap[i].pages; page != NULL; page = page->next) {
	    int j;
	    char *data = PAGE_DATA(page);

	    for (j = 0; j < page_count; j++, data += node_size) {
		s = (SEXP) data;
		if (! NODE_IS_MARKED(s))
		    SNAP_NODE(s, R_GenHeap[i].New);
	    }
	}
	R_GenHeap[i].Free = NEXT_NODE(R_GenHeap[i].New);
    }
}
#endif


/* Finalization and Weak References */

/* The design of this mechanism is very close to the one described in
   "Stretching the storage manager: weak pointers and stable names in
   Haskell" by Peyton Jones, Marlow, and Elliott (at
   www.research.microsoft.com/Users/simonpj/papers/weak.ps.gz). --LT */

static SEXP R_weak_refs = NULL;

#define READY_TO_FINALIZE_MASK 1

#define SET_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp |= READY_TO_FINALIZE_MASK)
#define CLEAR_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp &= ~READY_TO_FINALIZE_MASK)
#define IS_READY_TO_FINALIZE(s) ((s)->sxpinfo.gp & READY_TO_FINALIZE_MASK)

#define FINALIZE_ON_EXIT_MASK 2

#define SET_FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp |= FINALIZE_ON_EXIT_MASK)
#define CLEAR_FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp &= ~FINALIZE_ON_EXIT_MASK)
#define FINALIZE_ON_EXIT(s) ((s)->sxpinfo.gp & FINALIZE_ON_EXIT_MASK)

#define WEAKREF_SIZE 4
#define WEAKREF_KEY(w) VECTOR_ELT(w, 0)
#define SET_WEAKREF_KEY(w, k) SET_VECTOR_ELT(w, 0, k)
#define WEAKREF_VALUE(w) VECTOR_ELT(w, 1)
#define SET_WEAKREF_VALUE(w, v) SET_VECTOR_ELT(w, 1, v)
#define WEAKREF_FINALIZER(w) VECTOR_ELT(w, 2)
#define SET_WEAKREF_FINALIZER(w, f) SET_VECTOR_ELT(w, 2, f)
#define WEAKREF_NEXT(w) VECTOR_ELT(w, 3)
#define SET_WEAKREF_NEXT(w, n) SET_VECTOR_ELT(w, 3, n)

static SEXP MakeCFinalizer(R_CFinalizer_t cfun);

static SEXP NewWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit)
{
    SEXP w;

    switch (TYPEOF(key)) {
    case NILSXP:
    case ENVSXP:
    case EXTPTRSXP:
	break;
    default: error(_("can only weakly reference/finalize reference objects"));
    }
	
    PROTECT(key);
    PROTECT(val = NAMED(val) ? duplicate(val) : val);
    PROTECT(fin);
    w = allocVector(VECSXP, WEAKREF_SIZE);
    SET_TYPEOF(w, WEAKREFSXP);
    if (key != R_NilValue) {
	/* If the key is R_NilValue we don't register the weak reference.
	   This is used in loading saved images. */
        SET_WEAKREF_KEY(w, key);
	SET_WEAKREF_VALUE(w, val);
	SET_WEAKREF_FINALIZER(w, fin);
	SET_WEAKREF_NEXT(w, R_weak_refs);
	CLEAR_READY_TO_FINALIZE(w);
	if (onexit)
	    SET_FINALIZE_ON_EXIT(w);
	else
	    CLEAR_FINALIZE_ON_EXIT(w);
	R_weak_refs = w;
    }
    UNPROTECT(3);
    return w;
}

SEXP R_MakeWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit)
{
    switch (TYPEOF(fin)) {
    case NILSXP:
    case RCC_CLOSXP:
    case CLOSXP:
    case BUILTINSXP:
    case SPECIALSXP:
	break;
    default: error(_("finalizer must be a function or NULL"));
    }
    return NewWeakRef(key, val, fin, onexit);
}

SEXP R_MakeWeakRefC(SEXP key, SEXP val, R_CFinalizer_t fin, Rboolean onexit)
{
    SEXP w;
    PROTECT(key);
    PROTECT(val);
    w = NewWeakRef(key, val, MakeCFinalizer(fin), onexit);
    UNPROTECT(2);
    return w;
}

static void CheckFinalizers(void)
{
    SEXP s;
    for (s = R_weak_refs; s != R_NilValue; s = WEAKREF_NEXT(s))
	if (! NODE_IS_MARKED(WEAKREF_KEY(s)) && ! IS_READY_TO_FINALIZE(s))
	    SET_READY_TO_FINALIZE(s);
}

/* C finalizers are stored in a CHARSXP.  It would be nice if we could
   use EXTPTRSXP's but these only hold a void *, and function pointers
   are not guaranteed to be compatible with a void *.  There should be
   a cleaner way of doing this, but this will do for now. --LT */
static Rboolean isCFinalizer(SEXP fun)
{
    return TYPEOF(fun) == CHARSXP;
    /*return TYPEOF(fun) == EXTPTRSXP;*/
}

static SEXP MakeCFinalizer(R_CFinalizer_t cfun)
{
    SEXP s = allocString(sizeof(R_CFinalizer_t));
    *((R_CFinalizer_t *) CHAR(s)) = cfun;
    return s;
    /*return R_MakeExternalPtr((void *) cfun, R_NilValue, R_NilValue);*/
}

static R_CFinalizer_t GetCFinalizer(SEXP fun)
{
    return *((R_CFinalizer_t *) CHAR(fun));
    /*return (R_CFinalizer_t) R_ExternalPtrAddr(fun);*/
}

SEXP R_WeakRefKey(SEXP w)
{
    if (TYPEOF(w) != WEAKREFSXP)
	error(_("not a weak reference"));
    return WEAKREF_KEY(w);
}

SEXP R_WeakRefValue(SEXP w)
{
    SEXP v;
    if (TYPEOF(w) != WEAKREFSXP)
	error(_("not a weak reference"));
    v = WEAKREF_VALUE(w);
    if (v != R_NilValue && NAMED(v) != 2)
	SET_NAMED(v, 2);
    return v;
}

void R_RunWeakRefFinalizer(SEXP w)
{
    SEXP key, fun, e;
    if (TYPEOF(w) != WEAKREFSXP)
	error(_("not a weak reference"));
    key = WEAKREF_KEY(w);
    fun = WEAKREF_FINALIZER(w);
    SET_WEAKREF_KEY(w, R_NilValue);
    SET_WEAKREF_VALUE(w, R_NilValue);
    SET_WEAKREF_FINALIZER(w, R_NilValue);
    if (! IS_READY_TO_FINALIZE(w))
	SET_READY_TO_FINALIZE(w); /* insures removal from list on next gc */
    PROTECT(key);
    PROTECT(fun);
    if (isCFinalizer(fun)) {
	/* Must be a C finalizer. */
	R_CFinalizer_t cfun = GetCFinalizer(fun);
	cfun(key);
    }
    else if (fun != R_NilValue) {
	/* An R finalizer. */
	PROTECT(e = LCONS(fun, LCONS(key, R_NilValue)));
	eval(e, R_GlobalEnv);
	UNPROTECT(1);
    }
    UNPROTECT(2);
}

static Rboolean RunFinalizers(void)
{
    volatile SEXP s, last;
    volatile Rboolean finalizer_run = FALSE;

    for (s = R_weak_refs, last = R_NilValue; s != R_NilValue;) {
	SEXP next = WEAKREF_NEXT(s);
	if (IS_READY_TO_FINALIZE(s)) {
	    RCNTXT thiscontext;
	    RCNTXT * volatile saveToplevelContext;
	    volatile int savestack;
	    volatile SEXP topExp;

	    finalizer_run = TRUE;

	    /* A top level context is established for the finalizer to
	       insure that any errors that might occur do not spill
	       into the call that triggered the collection. */
	    begincontext(&thiscontext, CTXT_TOPLEVEL, R_NilValue, R_GlobalEnv,
			 R_NilValue, R_NilValue, R_NilValue);
	    saveToplevelContext = R_ToplevelContext;
	    PROTECT(topExp = R_CurrentExpr);
	    savestack = R_PPStackTop;
	    if (! SETJMP(thiscontext.cjmpbuf)) {
		R_GlobalContext = R_ToplevelContext = &thiscontext;

		/* The entry in the weak reference list is removed
		   before running the finalizer.  This insures that a
		   finalizer is run only once, even if running it
		   raises an error. */
		if (last == R_NilValue)
		    R_weak_refs = next;
		else
		    SET_WEAKREF_NEXT(last, next);
		R_RunWeakRefFinalizer(s);
	    }
	    endcontext(&thiscontext);
	    R_ToplevelContext = saveToplevelContext;
	    R_PPStackTop = savestack;
	    R_CurrentExpr = topExp;
	    UNPROTECT(1);
	}
	else last = s;
	s = next;
    }
    return finalizer_run;
}

void R_RunExitFinalizers(void)
{
    SEXP s;

    for (s = R_weak_refs; s != R_NilValue; s = WEAKREF_NEXT(s))
	if (FINALIZE_ON_EXIT(s))
	    SET_READY_TO_FINALIZE(s);
    RunFinalizers();
}

void R_RegisterFinalizerEx(SEXP s, SEXP fun, Rboolean onexit)
{
    R_MakeWeakRef(s, R_NilValue, fun, onexit);
}

void R_RegisterFinalizer(SEXP s, SEXP fun)
{
    R_RegisterFinalizerEx(s, fun, FALSE);
}

void R_RegisterCFinalizerEx(SEXP s, R_CFinalizer_t fun, Rboolean onexit)
{
    R_MakeWeakRefC(s, R_NilValue, fun, onexit);
}

void R_RegisterCFinalizer(SEXP s, R_CFinalizer_t fun)
{
    R_RegisterCFinalizerEx(s, fun, FALSE);
}

/* R interface function */

SEXP do_regFinaliz(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    checkArity(op, args);

    if (TYPEOF(CAR(args)) != ENVSXP && TYPEOF(CAR(args)) != EXTPTRSXP)
	errorcall(call, _("first argument must be environment or external pointer"));
    if (TYPEOF(CADR(args)) != CLOSXP && TYPEOF(CADR(args)) != RCC_CLOSXP)
	errorcall(call, _("second argument must be a function"));
    
    R_RegisterFinalizer(CAR(args), CADR(args));
    return R_NilValue;
}


/* The Generational Collector. */

/* Added if clause: if it's a stack-allocated object, we don't want to
   process the node, but we do want to process its children because
   they may be heap allocated. --garvin */


#define PROCESS_NODES() do {	      \
    while (forwarded_nodes != NULL) { \
	s = forwarded_nodes; \
	forwarded_nodes = NEXT_NODE(forwarded_nodes); \
	if (PREV_NODE(s) != NULL) { \
	    SNAP_NODE(s, R_GenHeap[NODE_CLASS(s)].Old[NODE_GENERATION(s)]); \
	    R_GenHeap[NODE_CLASS(s)].OldCount[NODE_GENERATION(s)]++;	\
	} else {							\
	    SET_NEXT_NODE(s, NULL);					\
	}								\
	FORWARD_CHILDREN(s);			\
    } \
} while (0)

void PROCESS_NODES_f(SEXP * forwarded_nodes) {
    while(*forwarded_nodes != NULL) {
	SEXP s = *forwarded_nodes;
	*forwarded_nodes = NEXT_NODE(*forwarded_nodes);
	if (PREV_NODE(s) != NULL) {
	    SNAP_NODE(s, R_GenHeap[NODE_CLASS(s)].Old[NODE_GENERATION(s)]);
	    R_GenHeap[NODE_CLASS(s)].OldCount[NODE_GENERATION(s)]++;
	} else {
	    SET_NEXT_NODE(s, NULL); 
	} 
	FORWARD_CHILDREN_f(s);
    }
}
	    

/*
#define PROCESS_NODES() do { \
    while (forwarded_nodes != NULL) { \
	s = forwarded_nodes; \
	forwarded_nodes = NEXT_NODE(forwarded_nodes); \
	SNAP_NODE(s, R_GenHeap[NODE_CLASS(s)].Old[NODE_GENERATION(s)]); \
	R_GenHeap[NODE_CLASS(s)].OldCount[NODE_GENERATION(s)]++;	\
	FORWARD_CHILDREN(s); \
    } \
} while (0)
*/

/* removed static for debugging --garvin */

/*static*/ void RunGenCollect(R_size_t size_needed)
{
    int i, gen, gens_collected;
    DevDesc *dd;
    RCNTXT *ctxt;
    SEXP s;
    SEXP forwarded_nodes;

    /* determine number of generations to collect */
    while (num_old_gens_to_collect < NUM_OLD_GENERATIONS) {
	if (collect_counts[num_old_gens_to_collect]-- <= 0) {
	    collect_counts[num_old_gens_to_collect] =
		collect_counts_max[num_old_gens_to_collect];
	    num_old_gens_to_collect++;
	}
	else break;
    }

 again:
    gens_collected = num_old_gens_to_collect;

#ifndef EXPEL_OLD_TO_NEW
    /* eliminate old-to-new references in generations to collect by
       transferring referenced nodes to referring generation */
    for (gen = 0; gen < num_old_gens_to_collect; gen++) {
	for (i = 0; i < NUM_NODE_CLASSES; i++) {
	    s = NEXT_NODE(R_GenHeap[i].OldToNew[gen]);
	    while (s != R_GenHeap[i].OldToNew[gen]) {
		SEXP next = NEXT_NODE(s);
		DO_CHILDREN(s, AgeNodeAndChildren, gen);
		UNSNAP_NODE(s);
		if (NODE_GENERATION(s) != gen)
		    REprintf("****snapping into wrong generation\n");
		SNAP_NODE(s, R_GenHeap[i].Old[gen]);
		s = next;
	    }
	}
    }
#endif

    DEBUG_CHECK_NODE_COUNTS("at start");

    /* unmark all marked nodes in old generations to be collected and
       move to New space */
    for (gen = 0; gen < num_old_gens_to_collect; gen++) {
	for (i = 0; i < NUM_NODE_CLASSES; i++) {
	    R_GenHeap[i].OldCount[gen] = 0;
	    s = NEXT_NODE(R_GenHeap[i].Old[gen]);
	    while (s != R_GenHeap[i].Old[gen]) {
		SEXP next = NEXT_NODE(s);
		if (gen < NUM_OLD_GENERATIONS - 1)
		    SET_NODE_GENERATION(s, gen + 1);
		UNMARK_NODE(s);
		s = next;
	    }
	    if (NEXT_NODE(R_GenHeap[i].Old[gen]) != R_GenHeap[i].Old[gen])
		BULK_MOVE(R_GenHeap[i].Old[gen], R_GenHeap[i].New);
	}
    }

    forwarded_nodes = NULL;

#ifndef EXPEL_OLD_TO_NEW
    /* scan nodes in uncollected old generations with old-to-new pointers */
    for (gen = num_old_gens_to_collect; gen < NUM_OLD_GENERATIONS; gen++)
	for (i = 0; i < NUM_NODE_CLASSES; i++)
	    for (s = NEXT_NODE(R_GenHeap[i].OldToNew[gen]);
		 s != R_GenHeap[i].OldToNew[gen];
		 s = NEXT_NODE(s))
		FORWARD_CHILDREN(s);
#endif

    /* forward all roots */
    FORWARD_NODE(R_NilValue);	           /* Builtin constants */
    FORWARD_NODE(NA_STRING);
    FORWARD_NODE(R_BlankString);
    FORWARD_NODE(R_UnboundValue);
    FORWARD_NODE(R_RestartToken);
    FORWARD_NODE(R_MissingArg);
    FORWARD_NODE(R_CommentSxp);

    FORWARD_NODE(R_GlobalEnv);	           /* Global environment */
    FORWARD_NODE(R_Warnings);	           /* Warnings, if any */

#ifdef NEW_CONDITION_HANDLING
    FORWARD_NODE(R_HandlerStack);          /* Condition handler stack */
    FORWARD_NODE(R_RestartStack);          /* Available restarts stack */
#endif

    for (i = 0; i < HSIZE; i++)	           /* Symbol table */
	FORWARD_NODE(R_SymbolTable[i]);

    if (R_CurrentExpr != NULL)	           /* Current expression */
	FORWARD_NODE(R_CurrentExpr);

    for (i = 0; i < R_MaxDevices; i++) {   /* Device display lists */
	dd = GetDevice(i);
	if (dd) {
	    if (dd->newDevStruct) {
		FORWARD_NODE(((GEDevDesc*) dd)->dev->displayList);
		FORWARD_NODE(((GEDevDesc*) dd)->dev->savedSnapshot);
	    }
	    else
		FORWARD_NODE(dd->displayList);
	}
    }

    for (ctxt = R_GlobalContext ; ctxt != NULL ; ctxt = ctxt->nextcontext) {
	FORWARD_NODE(ctxt->conexit);       /* on.exit expressions */
	FORWARD_NODE(ctxt->promargs);	   /* promises supplied to closure */
	FORWARD_NODE(ctxt->callfun);       /* the closure called */
        FORWARD_NODE(ctxt->sysparent);     /* calling environment */
	FORWARD_NODE(ctxt->call);          /* the call */
	FORWARD_NODE(ctxt->cloenv);        /* the closure environment */
#ifdef NEW_CONDITION_HANDLING
	FORWARD_NODE(ctxt->handlerstack);  /* the condition handler stack */
	FORWARD_NODE(ctxt->restartstack);  /* the available restarts stack */
#endif
    }

    FORWARD_NODE(framenames); 		   /* used for interprocedure
					      communication in model.c */

    FORWARD_NODE(R_PreciousList);

    for (i = 0; i < R_PPStackTop; i++)	   /* Protected pointers */
	FORWARD_NODE(R_PPStack[i]);

    FORWARD_NODE(R_VStack);		   /* R_alloc stack */

#ifdef BYTECODE
    {
	SEXP *sp;
	for (sp = R_BCNodeStackBase; sp < R_BCNodeStackTop; sp++)
	    FORWARD_NODE(*sp);
    }
#endif

    /* main processing loop */
    PROCESS_NODES_f(&forwarded_nodes);

    /* identify weakly reachable nodes */
    {
	Rboolean recheck_weak_refs;
	do {
	    recheck_weak_refs = FALSE;
	    for (s = R_weak_refs; s != R_NilValue; s = WEAKREF_NEXT(s)) {
		if (NODE_IS_MARKED(WEAKREF_KEY(s))) {
		    if (! NODE_IS_MARKED(WEAKREF_VALUE(s))) {
			recheck_weak_refs = TRUE;
			FORWARD_NODE(WEAKREF_VALUE(s));
		    }
		    if (! NODE_IS_MARKED(WEAKREF_FINALIZER(s))) {
			recheck_weak_refs = TRUE;
			FORWARD_NODE(WEAKREF_FINALIZER(s));
		    }
		}
	    }
	    PROCESS_NODES_f(&forwarded_nodes);
	} while (recheck_weak_refs);
    }

    /* mark nodes ready for finalizing */
    CheckFinalizers();
    
    /* process the weak reference chain */
    for (s = R_weak_refs; s != R_NilValue; s = WEAKREF_NEXT(s)) {
	FORWARD_NODE(s);
	FORWARD_NODE(WEAKREF_KEY(s));
	FORWARD_NODE(WEAKREF_VALUE(s));
	FORWARD_NODE(WEAKREF_FINALIZER(s));
    }
    PROCESS_NODES_f(&forwarded_nodes);

    DEBUG_CHECK_NODE_COUNTS("after processing forwarded list");

    /* release large vector allocations */
    ReleaseLargeFreeVectors();

    DEBUG_CHECK_NODE_COUNTS("after releasing large allocated nodes");

    /* reset Free pointers */
    for (i = 0; i < NUM_NODE_CLASSES; i++)
	R_GenHeap[i].Free = NEXT_NODE(R_GenHeap[i].New);

    /* update heap statistics */
    R_Collected = R_NSize;
    R_SmallVallocSize = 0;
    for (gen = 0; gen < NUM_OLD_GENERATIONS; gen++) {
	for (i = 1; i < NUM_SMALL_NODE_CLASSES; i++)
	    R_SmallVallocSize += R_GenHeap[i].OldCount[gen] * NodeClassSize[i];
	for (i = 0; i < NUM_NODE_CLASSES; i++)
	    R_Collected -= R_GenHeap[i].OldCount[gen];
    }
    R_NodesInUse = R_NSize - R_Collected;

    if (num_old_gens_to_collect < NUM_OLD_GENERATIONS) {
	if (R_Collected < R_MinFreeFrac * R_NSize ||
	    VHEAP_FREE() < size_needed + R_MinFreeFrac * R_VSize) {
	    num_old_gens_to_collect++;
	    if (R_Collected <= 0 || VHEAP_FREE() < size_needed)
		goto again;
	}
	else num_old_gens_to_collect = 0;
    }
    else num_old_gens_to_collect = 0;

    gen_gc_counts[gens_collected]++;

    if (gens_collected == NUM_OLD_GENERATIONS) {
	/**** do some adjustment for intermediate collections? */
	AdjustHeapSize(size_needed);
	TryToReleasePages();
	DEBUG_CHECK_NODE_COUNTS("after heap adjustment");
    }
    else if (gens_collected > 0) {
	TryToReleasePages();
	DEBUG_CHECK_NODE_COUNTS("after heap adjustment");
    }
#ifdef SORT_NODES
    if (gens_collected == NUM_OLD_GENERATIONS)
	SortNodes();
#endif

    if (gc_reporting) {
	REprintf("Garbage collection %d = %d", gc_count, gen_gc_counts[0]);
	for (i = 0; i < NUM_OLD_GENERATIONS; i++)
	    REprintf("+%d", gen_gc_counts[i + 1]);
	REprintf(" (level %d) ... ", gens_collected);
	DEBUG_GC_SUMMARY(gens_collected == NUM_OLD_GENERATIONS);
    }
}


SEXP do_gctorture(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    int i;
    SEXP old = allocVector(LGLSXP, 1);

    checkArity(op, args);
    i = asLogical(CAR(args));
    LOGICAL(old)[0] = !gc_inhibit_torture;
    if (i != NA_LOGICAL)
	gc_inhibit_torture = !i;
    return old;
}

SEXP do_gcinfo(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    int i;
    SEXP old = allocVector(LGLSXP, 1);

    checkArity(op, args);
    i = asLogical(CAR(args));
    LOGICAL(old)[0] = gc_reporting;
    if (i != NA_LOGICAL)
	gc_reporting = i;
    return old;
}


SEXP do_gc(SEXP call, SEXP op, SEXP args, SEXP rho)
{
    SEXP value;
    int ogc, reset_max;
    R_size_t onsize = R_NSize;

    checkArity(op, args);
    ogc = gc_reporting;
    gc_reporting = asLogical(CAR(args));
    reset_max=asLogical(CADR(args));
    num_old_gens_to_collect = NUM_OLD_GENERATIONS;
    R_gc();
    gc_reporting = ogc;
    /*- now return the [used , gc trigger size] for cells and heap */
    PROTECT(value = allocVector(INTSXP, 14));
    INTEGER(value)[0] = onsize - R_Collected;
    INTEGER(value)[1] = R_VSize - VHEAP_FREE();
    /* carefully here: we can't report large sizes in R's integer */
    INTEGER(value)[4] = (R_NSize < INT_MAX) ? R_NSize : NA_INTEGER;
    INTEGER(value)[5] = (R_VSize < INT_MAX) ? R_VSize : NA_INTEGER;
    /* next four are in 0.1Mb, rounded up */
    INTEGER(value)[2] = 10. * (onsize - R_Collected)/Mega * sizeof(SEXPREC) + 0.999;
    INTEGER(value)[3] = 10. * (R_VSize - VHEAP_FREE())/Mega * vsfac + 0.999;
    INTEGER(value)[6] = 10. * R_NSize/Mega * sizeof(SEXPREC) + 0.999;
    INTEGER(value)[7] = 10. * R_VSize/Mega * vsfac + 0.999;
    INTEGER(value)[8] = (R_MaxNSize < R_SIZE_T_MAX) ? 
	(10. * R_MaxNSize/Mega * sizeof(SEXPREC) + 0.999) : NA_INTEGER;
    INTEGER(value)[9] = (R_MaxVSize < R_SIZE_T_MAX) ? 
	(10. * R_MaxVSize/Mega * vsfac + 0.999) : NA_INTEGER;
    if (reset_max){
	    R_N_maxused = INTEGER(value)[0];
	    R_V_maxused = INTEGER(value)[1];	    
    }
    INTEGER(value)[10] = (R_N_maxused < INT_MAX) ? R_N_maxused : NA_INTEGER;
    INTEGER(value)[11] = (R_V_maxused < INT_MAX) ? R_V_maxused : NA_INTEGER;
    INTEGER(value)[12] = 10. * R_N_maxused/Mega*sizeof(SEXPREC)+0.999;
    INTEGER(value)[13] = 10. * R_V_maxused/Mega*vsfac +0.999;
    UNPROTECT(1);
    return value;
}


static void mem_err_heap(R_size_t size)
{
    errorcall(R_NilValue, _("vector memory exhausted (limit reached?)"));
}


static void mem_err_cons()
{
    errorcall(R_NilValue, _("cons memory exhausted (limit reached?)"));
}

/* InitMemory : Initialise the memory to be used in R. */
/* This includes: stack space, node space and vector space */

#define PP_REDZONE_SIZE 1000L
static R_size_t R_StandardPPStackSize, R_RealPPStackSize;

void InitMemory()
{
    int i;
    int gen;

    gc_reporting = R_Verbose;
    R_StandardPPStackSize = R_PPStackSize;
    R_RealPPStackSize = R_PPStackSize + PP_REDZONE_SIZE;
    if (!(R_PPStack = (SEXP *) malloc(R_RealPPStackSize * sizeof(SEXP))))
	R_Suicide("couldn't allocate memory for pointer stack");
    R_PPStackTop = 0;

    vsfac = sizeof(VECREC);
    R_VSize = (((R_VSize + 1)/ vsfac));

    UNMARK_NODE(&UnmarkedNodeTemplate);

    for (i = 0; i < NUM_NODE_CLASSES; i++) {
      for (gen = 0; gen < NUM_OLD_GENERATIONS; gen++) {
        R_GenHeap[i].Old[gen] = &R_GenHeap[i].OldPeg[gen];
	SET_PREV_NODE(R_GenHeap[i].Old[gen], R_GenHeap[i].Old[gen]);
	SET_NEXT_NODE(R_GenHeap[i].Old[gen], R_GenHeap[i].Old[gen]);

#ifndef EXPEL_OLD_TO_NEW
	R_GenHeap[i].OldToNew[gen] = &R_GenHeap[i].OldToNewPeg[gen];
	SET_PREV_NODE(R_GenHeap[i].OldToNew[gen], R_GenHeap[i].OldToNew[gen]);
	SET_NEXT_NODE(R_GenHeap[i].OldToNew[gen], R_GenHeap[i].OldToNew[gen]);
#endif

	R_GenHeap[i].OldCount[gen] = 0;
      }
      R_GenHeap[i].New = &R_GenHeap[i].NewPeg;
      SET_PREV_NODE(R_GenHeap[i].New, R_GenHeap[i].New);
      SET_NEXT_NODE(R_GenHeap[i].New, R_GenHeap[i].New);
    }

    for (i = 0; i < NUM_NODE_CLASSES; i++)
        R_GenHeap[i].Free = NEXT_NODE(R_GenHeap[i].New);

    SET_NODE_CLASS(&UnmarkedNodeTemplate, 0);
    orig_R_NSize = R_NSize;
    orig_R_VSize = R_VSize;

    /* R_NilValue */
    /* THIS MUST BE THE FIRST CONS CELL ALLOCATED */
    /* OR ARMAGEDDON HAPPENS. */
    /* Field assignments for R_NilValue must not go through write barrier
       since the write barrier prevents assignments to R_NilValue's fields.
       because of checks for nil */
    GET_FREE_NODE(R_NilValue);
    R_NilValue->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(R_NilValue) = NILSXP;
    CAR(R_NilValue) = R_NilValue;
    CDR(R_NilValue) = R_NilValue;
    TAG(R_NilValue) = R_NilValue;
    ATTRIB(R_NilValue) = R_NilValue;

#ifdef BYTECODE
    R_BCNodeStackBase = (SEXP *) malloc(R_BCNODESTACKSIZE * sizeof(SEXP));
    if (R_BCNodeStackBase == NULL)
	R_Suicide("couldn't allocate node stack");
# ifdef BC_INT_STACK
    R_BCIntStackBase =
      (IStackval *) malloc(R_BCINTSTACKSIZE * sizeof(IStackval));
    if (R_BCIntStackBase == NULL)
	R_Suicide("couldn't allocate integer stack");
# endif
    R_BCNodeStackTop = R_BCNodeStackBase;
    R_BCNodeStackEnd = R_BCNodeStackBase + R_BCNODESTACKSIZE;
# ifdef BC_INT_STACK
    R_BCIntStackTop = R_BCIntStackBase;
    R_BCIntStackEnd = R_BCIntStackBase + R_BCINTSTACKSIZE;
# endif
#endif
    R_weak_refs = R_NilValue;

#ifdef NEW_CONDITION_HANDLING
    R_HandlerStack = R_RestartStack = R_NilValue;
#endif

    pushAllocStack(NULL, -1, &allocVectorHeap, &allocNodeHeap);
    /* default allocator uses the regular old heap */
    fallback_alloc = FALSE;
    global_dump_stats = (getenv("R_DUMP_STATS") != NULL);
    global_stack_debug = (getenv("R_STACK_DEBUG") != NULL);
    global_alloc_stack_space_size = getenv("R_ALLOC_STACK_SPACE_SIZE") ? atoi(getenv("R_ALLOC_STACK_SPACE_SIZE")) : global_default_alloc_stack_space_size;
}

/* Since memory allocated from the heap is non-moving, R_alloc just
   allocates off the heap as CHARSXP's and maintains the stack of
   allocations through the ATTRIB pointer.  The stack pointer R_VStack
   is traced by the collector. */
char *vmaxget(void)
{
    return (char *) R_VStack;
}

void vmaxset(char *ovmax)
{
    R_VStack = (SEXP) ovmax;
}

char *R_alloc(long nelem, int eltsize)
{
    R_size_t size = nelem * eltsize;
    double dsize = nelem * eltsize;
    if (dsize > 0) { /* precaution against integer overflow */
	SEXP s;
#if SIZE_LONG > 4
	if(dsize < R_LEN_T_MAX)
	    s = allocString(size); /**** avoid extra null byte?? */
	else if(dsize < sizeof(double) * (R_LEN_T_MAX - 1))
	    s = allocVector(REALSXP, (int)(0.99+dsize/sizeof(double)));
	else
	    error(_("cannot allocate memory block of size %.0f"), dsize);
#else
	if(dsize > R_LEN_T_MAX)
	    error(_("cannot allocate memory block of size %.0f"), dsize);
	s = allocString(size); /**** avoid extra null byte?? */
#endif
	ATTRIB(s) = R_VStack;
	R_VStack = s;
	return CHAR(s);
    }
    else return NULL;
}

/* S COMPATIBILITY */

char *S_alloc(long nelem, int eltsize)
{
    R_size_t /*i,*/ size  = nelem * eltsize;
    char *p = R_alloc(nelem, eltsize);

    memset(p, 0, size);
    return p;
}


char *S_realloc(char *p, long new, long old, int size)
{
    int i, nold;
    char *q;
    /* shrinking is a no-op */
    if(new <= old) return p;
    q = R_alloc(new, size);
    nold = old * size;
    for(i = 0; i < nold; i++)
	q[i] = p[i];
    for(i = nold; i < new*size; i++)
	q[i] = 0;
    return q;
}

static AllocStack * allocStackTop = NULL;
static AllocStack * allocStackCurrent = NULL;
static int max_alloc_stack_id = 0;

void * getAllocStackTop()
{
  return (void *)allocStackTop;
}

void * getAllocStackCurrent()
{
  return (void *)allocStackCurrent;
}

/* pushes new allocator on the stack. Sets both allocStackTop and
 * allocStackCurrent to the new allocator.
 */
void pushAllocStack(SEXP space,
		    R_len_t size,
		    AllocVectorFunction alloc_vector_function,
		    AllocNodeFunction alloc_node_function)
{
    AllocStack * elem = (AllocStack *)malloc(sizeof(AllocStack));
    /*     fprintf(stderr, "A\n"); */
    elem->valid = TRUE;
    elem->id = max_alloc_stack_id++;
    elem->size = elem->original_size = size;
    elem->space = elem->stack = space;
    elem->allocateVector = alloc_vector_function;
    elem->allocateNode = alloc_node_function;
    elem->up = allocStackTop;
    elem->down = NULL;
    if (elem->up != NULL) elem->up->down = elem;
    allocStackTop = allocStackCurrent = elem;
}

/* pops top of allocator stack. allocStackCurrent is updated if it is
   the top, unchanged otherwise. */
void popAllocStack()
{
    /*     fprintf(stderr, "B\n"); */
    if (allocStackTop == NULL) {
	errorcall(R_NilValue,  _("cannot pop empty allocation stack"));
    } else {
	AllocStack * temp = allocStackTop;
	if (global_stack_debug) {
	    // if debugging, fill deallocated space with garbage
	    memset(temp->stack, 0xfa, temp->original_size);
	}
	allocStackTop = temp->up;
	allocStackTop->down = NULL;
	if (allocStackCurrent == temp) {
	    allocStackCurrent = allocStackTop;
	}
	if (global_stack_debug) {
	    temp->valid = FALSE;
	} else {
	    free(temp);
	}
    }
}

/* Moves allocStackCurrent up to point to its parent. Useful for
   specifying that memory should be allocated in the parent
   allocator. */
void upAllocStack()
{
    if (allocStackCurrent == NULL) {
	errorcall(R_NilValue, _("cannot up empty allocation stack"));
    } else if (allocStackCurrent->up == NULL) {
	errorcall(R_NilValue, _("cannot up allocator; current allocator has no parent"));
    } else {
	allocStackCurrent = allocStackCurrent->up;
    }
}

void downAllocStack()
{
    if (allocStackCurrent == NULL) {
	errorcall(R_NilValue, _("cannot down empty allocation stack"));
    } else if (allocStackCurrent->down == NULL) {
	errorcall(R_NilValue, _("cannot down allocator; current allocator has no child"));
    } else {
	allocStackCurrent = allocStackCurrent->down;
    }
}

#if 0
/* Makes allocStackCurrent point to the top of the stack again. Useful
   for undoing an upAllocStack call. */
void restoreAllocStack()
{
  allocStackCurrent = allocStackTop;
}
#endif

Rboolean getFallbackAlloc()
{
    return fallback_alloc;
}

void setFallbackAlloc(Rboolean x)
{
    fallback_alloc = x;
}

SEXP allocSExp(SEXPTYPE t)
{
    SEXP s;
    /*
    if (FORCE_GC || NO_FREE_NODES()) {
	R_gc_internal(0);
	if (NO_FREE_NODES())
	    mem_err_cons();
    }
    GET_FREE_NODE(s);
    */
    SEXP protect_on_gc[1] = {NULL};
    if (global_dump_stats) fprintf(stderr, "Alloc: SEXP type %u ", t);
    s = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    SET_TYPEOF(s, t);
    CAR(s) = R_NilValue;
    CDR(s) = R_NilValue;
    SET_TAG(s, R_NilValue);
    SET_ATTRIB(s, R_NilValue);
    return s;
}

static SEXP old_allocSExpNonCons(SEXPTYPE t)
{
    SEXP s;
    if (FORCE_GC || NO_FREE_NODES()) {
	R_gc_internal(0);
	if (NO_FREE_NODES())
	    mem_err_cons();
    }
    GET_FREE_NODE(s);
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = t;
    TAG(s) = R_NilValue;
    ATTRIB(s) = R_NilValue;
    return s;
}

static SEXP allocSExpNonConsHeap()
{
    SEXP s;

    if (FORCE_GC || NO_FREE_NODES()) {
	R_gc_internal(0);
	if (NO_FREE_NODES())
	    mem_err_cons();
    }
    GET_FREE_NODE(s);
    return s;
}

static void allocSExpNonConsInPlace(SEXPTYPE t, SEXP s)
{
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = t;
    TAG(s) = R_NilValue;
    ATTRIB(s) = R_NilValue;
}

static SEXP allocSExpNonCons(SEXPTYPE t)
{
    SEXP s;
    SEXP protect_on_gc[1] = {NULL};
    if (global_dump_stats) fprintf(stderr, "Alloc: SEXP non-cons type %u ", t);
    s = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);
    allocSExpNonConsInPlace(t, s);
    return s;
}

/* cons is defined directly do avoid the need to protect its arguments
   unless a GC will actually occur. */
SEXP old_cons(SEXP car, SEXP cdr)
{
    SEXP s;
    if (FORCE_GC || NO_FREE_NODES()) {
      PROTECT(car);
      PROTECT(cdr);
      R_gc_internal(0);
      UNPROTECT(2);
      if (NO_FREE_NODES())
	mem_err_cons();
    }
    GET_FREE_NODE(s);
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = LISTSXP;
    CAR(s) = car;
    CDR(s) = cdr;
    TAG(s) = R_NilValue;
    ATTRIB(s) = R_NilValue;
    return s;
}

SEXP cons(SEXP car, SEXP cdr)
{
    SEXP s;
    SEXP protect_on_gc[3] = {car, cdr, NULL};

    if (global_dump_stats) fprintf(stderr, "Alloc: cons "); 
    s = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);
    consInPlace(car, cdr, s);
    return s;
}

SEXP consHeap(SEXP car, SEXP cdr)
{
    SEXP s;
    
    if (global_dump_stats) fprintf(stderr, "Alloc: consHeap %u bytes ", sizeof(SEXPREC));
    if (FORCE_GC || NO_FREE_NODES()) {
	PROTECT(car);
	PROTECT(cdr);
	R_gc_internal(0);
	UNPROTECT(2);
	if (NO_FREE_NODES())
	    mem_err_cons();
    }
    GET_FREE_NODE(s);
    CDR(s) = cdr;     /* so that we can connect cells */
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);
    return s;
}

void consInPlace(SEXP car, SEXP cdr, SEXP s)
{
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = LISTSXP;
    CAR(s) = car;
    CDR(s) = cdr;
    TAG(s) = R_NilValue;
    ATTRIB(s) = R_NilValue;
}

/* protect_on_gc is a NULL-terminated list of SEXPs to PROTECT if a GC
   will be triggered */
SEXP allocNodeHeap(AllocStack * allocator, SEXP * protect_on_gc)
{
    SEXP s;

    if (global_dump_stats) fprintf(stderr, "node heap %u bytes ", sizeof(SEXPREC));

    if (FORCE_GC || NO_FREE_NODES()) {
	int n;
	for (n = 0; *protect_on_gc != NULL; protect_on_gc++, n++) {
	    PROTECT(*protect_on_gc);
	}
	R_gc_internal(0);
	UNPROTECT(n);
	if (NO_FREE_NODES())
	    mem_err_cons();
    }
    GET_FREE_NODE(s);
    return s;
}

/*----------------------------------------------------------------------

  NewEnvironment

  Create an environment by extending "rho" with a frame obtained by
  pairing the variable names given by the tags on "namelist" with
  the values given by the elements of "valuelist".

  NewEnvironment is defined directly to avoid the need to protect its
  arguments unless a GC will actually occur.  This definition allows
  the namelist argument to be shorter than the valuelist; in this
  case the remaining values must be named already.  (This is useful
  in cases where the entire valuelist is already named--namelist can
  then be R_NilValue.)

  The valuelist is destructively modified and used as the
  environment's frame.
*/
SEXP NewEnvironment(SEXP namelist, SEXP valuelist, SEXP rho)
{
    SEXP v, n, newrho;

    /*
    if (FORCE_GC || NO_FREE_NODES()) {
      PROTECT(namelist);
      PROTECT(valuelist);
      PROTECT(rho);
      R_gc_internal(0);
      UNPROTECT(3);
      if (NO_FREE_NODES())
	mem_err_cons();
    }
    GET_FREE_NODE(newrho);
    */
    SEXP protect_on_gc[4] = {namelist, valuelist, rho, NULL};
    if (global_dump_stats) fprintf(stderr, "Alloc: environment ");
    newrho = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", newrho);
    newrho->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(newrho) = ENVSXP;
    FRAME(newrho) = valuelist;
    ENCLOS(newrho) = rho;
    HASHTAB(newrho) = R_NilValue;
    ATTRIB(newrho) = R_NilValue;

    v = valuelist;
    n = namelist;
    while (v != R_NilValue && n != R_NilValue) {
	SET_TAG(v, TAG(n));
	v = CDR(v);
	n = CDR(n);
    }
    return (newrho);
}

/* mkPROMISE is defined directly do avoid the need to protect its arguments
   unless a GC will actually occur. */
SEXP mkPROMISE(SEXP expr, SEXP rho)
{
    SEXP s;
    /*
    if (FORCE_GC || NO_FREE_NODES()) {
      PROTECT(expr);
      PROTECT(rho);
      R_gc_internal(0);
      UNPROTECT(2);
      if (NO_FREE_NODES())
	mem_err_cons();
    }
    GET_FREE_NODE(s);
    */
    SEXP protect_on_gc[3] = {expr, rho, NULL};
    if (global_dump_stats) fprintf(stderr, "Alloc: promise ");
    s = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);    
    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
    TYPEOF(s) = PROMSXP;
    PRCODE(s) = expr;
    PRENV(s) = rho;
    PRVALUE(s) = R_UnboundValue;
    PRSEEN(s) = 0;
    ATTRIB(s) = R_NilValue;
    return s;
}

/* "allocString" allocate a string on the (vector) heap. */
/* All vector objects  must be a multiple of sizeof(ALIGN) */
/* bytes so that alignment is preserved for all objects */

SEXP allocString(int length)
{
  return allocVector(CHARSXP, length);
}


/* Allocate a vector object on the heap */

SEXP allocVector(SEXPTYPE type, R_len_t length)
{
    SEXP s;
    R_size_t size;

    if (type == NILSXP) {
	return R_NilValue;
    }
    if (global_dump_stats) fprintf(stderr, "Alloc: vector type %u length %u ", type, length);
    s = allocStackCurrent->allocateVector(allocStackCurrent, type, length);
    if (global_dump_stats) fprintf(stderr, "0x%x\n", s);
    allocVectorInPlace(type, length, s);
    return s;
}

SEXP old_allocVector(SEXPTYPE type, R_len_t length)
{
    SEXP s;     /* For the generational collector it would be safer to
		   work in terms of a VECSEXP here, but that would
		   require several casts below... */
    R_len_t i;
    R_size_t size = 0, alloc_size, old_R_VSize;
    int node_class;

    if (length < 0 )
	errorcall(R_GlobalContext->call,
		  _("negative length vectors are not allowed"));
    /* number of vector cells to allocate */
    switch (type) {
    case NILSXP:
	return R_NilValue;
    case RAWSXP:
	size = BYTE2VEC(length);
	break;
    case CHARSXP:
	size = BYTE2VEC(length + 1);
	break;
    case LGLSXP:
    case INTSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(int))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = INT2VEC(length);
	}
	break;
    case REALSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(double))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = FLOAT2VEC(length);
	}
	break;
    case CPLXSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(Rcomplex))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = COMPLEX2VEC(length);
	}
	break;
    case STRSXP:
    case EXPRSXP:
    case VECSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(SEXP))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = PTR2VEC(length);
	}
	break;
    case LANGSXP:
	if(length == 0) return R_NilValue;
	s = allocList(length);
	TYPEOF(s) = LANGSXP;
	return s;
    case LISTSXP:
	return allocList(length);
    default:
	error(_("invalid type/length (%d/%d) in vector allocation"), 
	      type, length);
    }

    if (size <= NodeClassSize[1]) {
      node_class = 1;
      alloc_size = NodeClassSize[1];
    }
    else {
      node_class = LARGE_NODE_CLASS;
      alloc_size = size;
      for (i = 2; i < NUM_SMALL_NODE_CLASSES; i++) {
	if (size <= NodeClassSize[i]) {
	  node_class = i;
	  alloc_size = NodeClassSize[i];
	  break;
	}
      }
    }

    /* save current R_VSize to roll back adjustment if malloc fails */
    old_R_VSize = R_VSize;

    /* we need to do the gc here so allocSExp doesn't! */
    if (FORCE_GC || NO_FREE_NODES() || VHEAP_FREE() < alloc_size) {
	R_gc_internal(alloc_size);
	if (NO_FREE_NODES())
	    mem_err_cons();
	if (VHEAP_FREE() < alloc_size)
	    mem_err_heap(size);
    }

    if (size > 0) {
	if (node_class < NUM_SMALL_NODE_CLASSES) {
	    CLASS_GET_FREE_NODE(node_class, s);
	    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	    SET_NODE_CLASS(s, node_class);
	    R_SmallVallocSize += alloc_size;
	}
	else {
	    Rboolean success = FALSE;
	    s = NULL; /* initialize to suppress warning */
	    if (size < (R_SIZE_T_MAX / sizeof(VECREC)) - sizeof(SEXPREC_ALIGN)) {
		s = malloc(sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));
		if (s == NULL) {
		    /* If we are near the address space limit, we
		       might be short of address space.  So return
		       all unused objects to malloc and try again. */
		    R_gc_internal(alloc_size);
		    s = malloc(sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));
		}
		if (s != NULL) success = TRUE;
	    }
	    if (! success) {
		/* reset the vector heap limit */
		R_VSize = old_R_VSize;
		errorcall(R_NilValue, _("cannot allocate vector of size %lu Kb"),
			  (size * sizeof(VECREC))/1024);
	    }
	    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	    SET_NODE_CLASS(s, LARGE_NODE_CLASS);
	    R_LargeVallocSize += size;
	    R_GenHeap[LARGE_NODE_CLASS].AllocCount++;
	    R_NodesInUse++;
	    SNAP_NODE(s, R_GenHeap[LARGE_NODE_CLASS].New);
	}
	ATTRIB(s) = R_NilValue;
	TYPEOF(s) = type;
    }
    else {
	GC_PROT(s = allocSExpNonCons(type));
    }
    LENGTH(s) = length;
    SET_NAMED(s, 0);

    /* The following prevents disaster in the case */
    /* that an uninitialised string vector is marked */
    /* Direct assignment is OK since the node was just allocated and */
    /* so is at least as new as R_NilValue and R_BlankString */
    if (type == EXPRSXP || type == VECSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_NilValue;
    }
    else if(type == STRSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_BlankString;
    }
    else if (type == CHARSXP)
	CHAR(s)[length] = 0;
    return s;
}

SEXP allocVectorHeap(AllocStack * allocator, SEXPTYPE type, R_len_t length)
{
    SEXP s;     /* For the generational collector it would be safer to
		   work in terms of a VECSEXP here, but that would
		   require several casts below... */
    R_len_t i;
    R_size_t size = 0, alloc_size, old_R_VSize;
    int node_class;

    if (length < 0 )
	errorcall(R_GlobalContext->call,
		  _("negative length vectors are not allowed"));
    /* number of vector cells to allocate */
    switch (type) {
    case NILSXP:
	return R_NilValue;
    case RAWSXP:
	size = BYTE2VEC(length);
	break;
    case CHARSXP:
	size = BYTE2VEC(length + 1);
	break;
    case LGLSXP:
    case INTSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(int))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = INT2VEC(length);
	}
	break;
    case REALSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(double))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = FLOAT2VEC(length);
	}
	break;
    case CPLXSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(Rcomplex))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = COMPLEX2VEC(length);
	}
	break;
    case STRSXP:
    case EXPRSXP:
    case VECSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(SEXP))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = PTR2VEC(length);
	}
	break;
    case LANGSXP:
	if(length == 0) return R_NilValue;
	s = allocListHeap(length);       /* alloc */
	TYPEOF(s) = LANGSXP;
	return s;
    case LISTSXP:
	return allocListHeap(length);    /* alloc */
    default:
	error(_("invalid type/length (%d/%d) in vector allocation"), 
	      type, length);
    }

    if (size <= NodeClassSize[1]) {
      node_class = 1;
      alloc_size = NodeClassSize[1];
    }
    else {
      node_class = LARGE_NODE_CLASS;
      alloc_size = size;
      for (i = 2; i < NUM_SMALL_NODE_CLASSES; i++) {
	if (size <= NodeClassSize[i]) {
	  node_class = i;
	  alloc_size = NodeClassSize[i];
	  break;
	}
      }
    }

    /* save current R_VSize to roll back adjustment if malloc fails */
    old_R_VSize = R_VSize;

    /* we need to do the gc here so allocSExp doesn't! */
    if (FORCE_GC || NO_FREE_NODES() || VHEAP_FREE() < alloc_size) {
	R_gc_internal(alloc_size);
	if (NO_FREE_NODES())
	    mem_err_cons();
	if (VHEAP_FREE() < alloc_size)
	    mem_err_heap(size);
    }

    if (size > 0) {
	if (node_class < NUM_SMALL_NODE_CLASSES) {

	    if (global_dump_stats) fprintf(stderr, "vector heap %u bytes ", NODE_SIZE(node_class));
	    CLASS_GET_FREE_NODE(node_class, s);      /* alloc */
	    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	    SET_NODE_CLASS(s, node_class);
	    R_SmallVallocSize += alloc_size;
	}
	else {
	    Rboolean success = FALSE;

	    if (global_dump_stats) fprintf(stderr, "vector heap %u bytes ", sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));
	    
	    s = NULL; /* initialize to suppress warning */
	    if (size < (R_SIZE_T_MAX / sizeof(VECREC)) - sizeof(SEXPREC_ALIGN)) {
		s = malloc(sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));   /* alloc */
		if (s == NULL) {
		    /* If we are near the address space limit, we
		       might be short of address space.  So return
		       all unused objects to malloc and try again. */
		    R_gc_internal(alloc_size);
		    s = malloc(sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));  /* alloc */
		}
		if (s != NULL) success = TRUE;
	    }
	    if (! success) {
		/* reset the vector heap limit */
		R_VSize = old_R_VSize;
		errorcall(R_NilValue, _("cannot allocate vector of size %lu Kb"),
			  (size * sizeof(VECREC))/1024);
	    }
	    s->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	    SET_NODE_CLASS(s, LARGE_NODE_CLASS);
	    R_LargeVallocSize += size;
	    R_GenHeap[LARGE_NODE_CLASS].AllocCount++;
	    R_NodesInUse++;
	    SNAP_NODE(s, R_GenHeap[LARGE_NODE_CLASS].New);
	}
    }
    else {
	GC_PROT(s = allocSExpNonConsHeap(type));
	allocSExpNonConsInPlace(type, s);
    }

    LENGTH(s) = length;
    SET_NAMED(s, 0);

    /* The following prevents disaster in the case */
    /* that an uninitialised string vector is marked */
    /* Direct assignment is OK since the node was just allocated and */
    /* so is at least as new as R_NilValue and R_BlankString */
    if (type == EXPRSXP || type == VECSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_NilValue;
    }
    else if(type == STRSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_BlankString;
    }
    else if (type == CHARSXP)
	CHAR(s)[length] = 0;
    return s;
}

const int max_vector_stack_length = 2;

SEXP allocVectorStack(AllocStack * allocator, SEXPTYPE type, R_len_t length)
{
    R_size_t size;
    
    if (!allocator->valid) {
	error("allocVectorStack: invalid allocation stack node");
    }
    if (fallback_alloc) return allocVectorHeap(NULL, type, length);
    if (length > max_vector_stack_length) return allocVectorHeap(NULL, type, length);

    size = allocVectorGetSize(type, length);
    if (allocator->size == -1) { /* -1 means heap allocator */
	error("allocVectorStack called on heap allocator");
    }
    if (size > allocator->size) {
	if (global_dump_stats) {
	    fprintf(stderr, "parent ");
	}
	return allocator->up->allocateVector(allocator->up, type, length); /* parent */
    } else {
	SEXP space = allocator->space;
	if (global_dump_stats) fprintf(stderr, "vector stack %u bytes from pool %u ", size, allocator->id);
	space->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	SET_NEXT_NODE(space, NULL);
	SET_PREV_NODE(space, NULL);
	allocator->space = (SEXP)(((R_size_t)allocator->space) + size);
	allocator->size -= size;
	if (global_stack_info) {
	    fprintf(stderr, "allocVectorStack allocated %u bytes; %u bytes available in this allocator\n", size, allocator->size);
	}
	return space;
    }
}

#if 0
int get_stack_depth(AllocStack * p) {
    int depth = 0;
    while (p) {
	depth++;
	p = p->up;
    }
    return depth;
}
#endif


SEXP allocNodeStack(AllocStack * allocator, SEXP * protect_on_gc)
{
    const R_size_t node_size = sizeof(SEXPREC);
    if (!allocator->valid) {
	error("allocNodeStack: invalid allocation stack node");
    }
    if (fallback_alloc) return allocNodeHeap(allocator, protect_on_gc);
    if (allocator == NULL) {
	error("allocNodeStack called on null allocator");
    }
    if (node_size > allocator->size) {
	if (global_dump_stats) {
	    fprintf(stderr, "parent ");
	}
	SEXP protect_on_gc[1] = {NULL};
	return (*allocator->up->allocateNode)(allocator->up, protect_on_gc); /* parent */
    } else {
	SEXP space = allocator->space;
	if (global_dump_stats) fprintf(stderr, "node stack %u bytes from pool %u ", node_size, allocator->id);
	space->sxpinfo = UnmarkedNodeTemplate.sxpinfo;
	SET_NEXT_NODE(space, NULL);
	SET_PREV_NODE(space, NULL);
	allocator->space++;  /* space is an SEXP, so incrementing ptr will add size of node */
	allocator->size -= node_size;
	if (global_stack_info) {
	  fprintf(stderr,"allocNodeStack allocated %u bytes; %u bytes available in this allocator\n", node_size, allocator->size);
	}
	return space;
    }
}
    

/* return size in bytes of space to be allocated. Does not allocate anything.  */
R_size_t allocVectorGetSize(SEXPTYPE type, R_len_t length)
{
    R_len_t i;
    R_size_t size = 0; /* size in VECRECs */
    int node_class;

    if (length < 0 )
	errorcall(R_GlobalContext->call,
		  _("negative length vectors are not allowed"));
    /* number of vector cells to allocate */
    switch (type) {
    case NILSXP:
	return 0;
    case RAWSXP:
	size = BYTE2VEC(length);
	break;
    case CHARSXP:
	size = BYTE2VEC(length + 1);
	break;
    case LGLSXP:
    case INTSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(int))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = INT2VEC(length);
	}
	break;
    case REALSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(double))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = FLOAT2VEC(length);
	}
	break;
    case CPLXSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(Rcomplex))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = COMPLEX2VEC(length);
	}
	break;
    case STRSXP:
    case EXPRSXP:
    case VECSXP:
	if (length <= 0)
	    size = 0;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(SEXP))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    size = PTR2VEC(length);
	}
	break;
    case LANGSXP:
    case LISTSXP:
	return length * sizeof(SEXPREC);
    default:
	error(_("invalid type/length (%d/%d) in vector allocation"), 
	      type, length);
    }

    if (size <= NodeClassSize[1]) {
      node_class = 1;
    }
    else {
      node_class = LARGE_NODE_CLASS;
      for (i = 2; i < NUM_SMALL_NODE_CLASSES; i++) {
	if (size <= NodeClassSize[i]) {
	  node_class = i;
	  break;
	}
      }
    }

    if (size > 0) {
	if (node_class < NUM_SMALL_NODE_CLASSES) {
	    return NODE_SIZE(node_class);
	}
	else {
	    if (size < (R_SIZE_T_MAX / sizeof(VECREC)) - sizeof(SEXPREC_ALIGN)) {
		return(sizeof(SEXPREC_ALIGN) + size * sizeof(VECREC));
	    }
	    else {
		errorcall(R_NilValue, _("cannot allocate vector of size %lu Kb"),
			  (size * sizeof(VECREC))/1024);
	    }
	}
    }
    else {
	return sizeof(SEXPREC);
    }
}

void allocVectorInPlace(SEXPTYPE type, R_len_t length, SEXP s)
{
    R_len_t i;
    Rboolean positive_size = FALSE;

    if (length < 0 )
	errorcall(R_GlobalContext->call,
		  _("negative length vectors are not allowed"));
    /* number of vector cells to allocate */
    switch (type) {
    case NILSXP:
	return;
    case RAWSXP:
	positive_size = (BYTE2VEC(length) > 0);
	break;
    case CHARSXP:
	positive_size = (BYTE2VEC(length + 1) > 0);
	break;
    case LGLSXP:
    case INTSXP:
	if (length <= 0)
	    positive_size = FALSE;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(int))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    positive_size = TRUE;
	}	
	break;
    case REALSXP:
	if (length <= 0)
	    positive_size = FALSE;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(double))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    positive_size = TRUE;
	}
	break;
    case CPLXSXP:
	if (length <= 0)
	    positive_size = FALSE;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(Rcomplex))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    positive_size = TRUE;
	}
	break;
    case STRSXP:
    case EXPRSXP:
    case VECSXP:
	if (length <= 0)
	    positive_size = FALSE;
	else {
	    if (length > R_SIZE_T_MAX / sizeof(SEXP))
		errorcall(R_GlobalContext->call,
			  _("cannot allocate vector of length %d"), length);
	    positive_size = TRUE;
	}
	break;
    case LANGSXP:
	allocListInPlace(s);
	TYPEOF(s) = LANGSXP;
	return;
    case LISTSXP:
	allocListInPlace(s);
	return;
    default:
	error(_("invalid type/length (%d/%d) in vector allocation"), 
	      type, length);
    }
    
    if (positive_size) {
	ATTRIB(s) = R_NilValue;
	TYPEOF(s) = type;
    } else {
	allocSExpNonConsInPlace(type, s);
    }
    LENGTH(s) = length;
    SET_NAMED(s, 0);


    /* The following prevents disaster in the case */
    /* that an uninitialised string vector is marked */
    /* Direct assignment is OK since the node was just allocated and */
    /* so is at least as new as R_NilValue and R_BlankString */
    if (type == EXPRSXP || type == VECSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_NilValue;
    }
    else if(type == STRSXP) {
	SEXP *data = STRING_PTR(s);
	for (i = 0; i < length; i++)
	    data[i] = R_BlankString;
    }
    else if (type == CHARSXP)
	CHAR(s)[length] = 0;
}
  
SEXP allocList(int n)
{
    /* TODO: two passes through the list. Too much of an efficiency hit? */
    int i;
    SEXP result = R_NilValue;

    if (global_dump_stats) fprintf(stderr, "Alloc: list length %u ", n);
    for (i = 0; i < n; i++) {
	/* protect CDR (result), but CAR is always R_NilValue and doesn't need protection */
	SEXP prev_result = result;
	SEXP protect_on_gc[2] = {result, NULL};
	result = allocStackCurrent->allocateNode(allocStackCurrent, protect_on_gc);
	CDR(result) = prev_result;
    }
    if (global_dump_stats) fprintf(stderr, "0x%x\n", result);
    allocListInPlace(result);
    return result;
}

SEXP allocListHeap(int n)
{
    int i;    
    SEXP result, s;

    if (global_dump_stats) fprintf(stderr, "Alloc: allocListHeap %u cells %u bytes ", n, sizeof(SEXPREC));
    result = R_NilValue;
    for (i = 0; i < n; i++) {
      if (FORCE_GC || NO_FREE_NODES()) {
	PROTECT(result);
	R_gc_internal(0);
	UNPROTECT(1);
	if (NO_FREE_NODES())
	  mem_err_cons();
      }
      GET_FREE_NODE(s);
      CDR(s) = result;     /* so that we can connect cells */
      result = s;
    }
    if (global_dump_stats) fprintf(stderr, "0x%x\n", result);
    return result;
}

void allocListInPlace(SEXP s)
{
    while (s != R_NilValue) {
	consInPlace(R_NilValue, CDR(s), s);
	s = CDR(s);
    }
}

/* "gc" a mark-sweep or in-place generational garbage collector */

void R_gc(void)
{
    R_gc_internal(0);
}

#ifdef _R_HAVE_TIMING_
double R_getClockIncrement(void);
void R_getProcTime(double *data);

static double gctimes[5], gcstarttimes[5];
static Rboolean gctime_enabled = FALSE;

SEXP do_gctime(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans;
    if (args == R_NilValue)
	gctime_enabled = TRUE;
    else
	gctime_enabled = asLogical(CAR(args));
    ans = allocVector(REALSXP, 5);
    REAL(ans)[0] = gctimes[0];
    REAL(ans)[1] = gctimes[1];
    REAL(ans)[2] = gctimes[2];
    REAL(ans)[3] = gctimes[3];
    REAL(ans)[4] = gctimes[4];
    return ans;
}
#else /* not _R_HAVE_TIMING_ */
SEXP do_gctime(SEXP call, SEXP op, SEXP args, SEXP env)
{
    error(_("gc.time() is not implemented on this system"));
    return R_NilValue;		/* -Wall */
}
#endif /* not _R_HAVE_TIMING_ */

static void gc_start_timing(void)
{
#ifdef _R_HAVE_TIMING_
    if (gctime_enabled)
	R_getProcTime(gcstarttimes);
#endif /* _R_HAVE_TIMING_ */
}

static void gc_end_timing(void)
{
#ifdef _R_HAVE_TIMING_
    if (gctime_enabled) {
	double times[5], delta;
	R_getProcTime(times);
	delta = R_getClockIncrement();

	/* add delta to compensate for timer resolution */
	gctimes[0] += times[0] - gcstarttimes[0] + delta;
	gctimes[1] += times[1] - gcstarttimes[1] + delta;
	gctimes[2] += times[2] - gcstarttimes[2] + delta;
	gctimes[3] += times[3] - gcstarttimes[3];
	gctimes[4] += times[4] - gcstarttimes[4];
    }
#endif /* _R_HAVE_TIMING_ */
}

#define MAX(a,b) (a) < (b) ? (b) : (a)

static void R_gc_internal(R_size_t size_needed)
{
    R_size_t vcells;
    double vfrac;
    Rboolean first = TRUE;

 again:

    gc_count++;

    R_N_maxused = MAX(R_N_maxused, R_NodesInUse);
    R_V_maxused = MAX(R_V_maxused, R_VSize - VHEAP_FREE());

    BEGIN_SUSPEND_INTERRUPTS {
      gc_start_timing();
      RunGenCollect(size_needed);
      gc_end_timing();
    } END_SUSPEND_INTERRUPTS;

    if (gc_reporting) {
	REprintf("\n%d cons cells free (%d%%)\n",
		 R_Collected, (100 * R_Collected / R_NSize));
	vcells = VHEAP_FREE();
	vfrac = (100.0 * vcells) / R_VSize;
	/* arrange for percentage to be rounded down, or we get
	   `100% free' ! */
	REprintf("%.1f Mbytes of heap free (%d%%)\n",
		 vcells * sizeof(VECREC) / Mega, (int)vfrac);
    }

    if (first) {
	first = FALSE;
	/* Run any eligible finalizers.  The return result of
	   RunFinalizers is TRUE if any finalizers are actually run.
	   There is a small chance that running finalizers here may
	   chew up enough memory to make another immediate collection
	   necessary.  If so, we jump back to the beginning and run
	   the collection, but on this second pass we do not run
	   finalizers. */
	if (RunFinalizers() &&
	    (NO_FREE_NODES() || size_needed > VHEAP_FREE()))
	    goto again;
    }
}

SEXP do_memlimits(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans;
    int nsize, vsize;
    R_size_t tmp;
    
    checkArity(op, args);
    nsize = asInteger(CAR(args));
    vsize = asInteger(CADR(args));
    if(nsize != NA_INTEGER) R_SetMaxNSize((R_size_t) nsize);
    if(vsize != NA_INTEGER) R_SetMaxVSize((R_size_t) vsize);
    PROTECT(ans = allocVector(INTSXP, 2));
    tmp = R_GetMaxNSize();
    INTEGER(ans)[0] = (tmp < INT_MAX) ? tmp : NA_INTEGER;
    tmp = R_GetMaxVSize();
    INTEGER(ans)[1] = (tmp < INT_MAX) ? tmp : NA_INTEGER;
    UNPROTECT(1);
    return ans;
}

SEXP do_memoryprofile(SEXP call, SEXP op, SEXP args, SEXP env)
{
    SEXP ans, nms;
    int i;

    PROTECT(ans = allocVector(INTSXP, 25));
    PROTECT(nms = allocVector(STRSXP, 25));
    for (i = 0; i < 25; i++) {
        INTEGER(ans)[i] = 0;
        SET_STRING_ELT(nms, i, R_BlankString);
    }
    SET_STRING_ELT(nms, NILSXP, mkChar("NILSXP"));
    SET_STRING_ELT(nms, SYMSXP, mkChar("SYMSXP"));
    SET_STRING_ELT(nms, LISTSXP, mkChar("LISTSXP"));
    SET_STRING_ELT(nms, CLOSXP, mkChar("CLOSXP"));
    SET_STRING_ELT(nms, RCC_CLOSXP, mkChar("RCC_CLOSXP"));
    SET_STRING_ELT(nms, ENVSXP, mkChar("ENVSXP"));
    SET_STRING_ELT(nms, PROMSXP, mkChar("PROMSXP"));
    SET_STRING_ELT(nms, LANGSXP, mkChar("LANGSXP"));
    SET_STRING_ELT(nms, SPECIALSXP, mkChar("SPECIALSXP"));
    SET_STRING_ELT(nms, BUILTINSXP, mkChar("BUILTINSXP"));
    SET_STRING_ELT(nms, CHARSXP, mkChar("CHARSXP"));
    SET_STRING_ELT(nms, RAWSXP, mkChar("RAWSXP"));
    SET_STRING_ELT(nms, LGLSXP, mkChar("LGLSXP"));
    SET_STRING_ELT(nms, INTSXP, mkChar("INTSXP"));
    SET_STRING_ELT(nms, REALSXP, mkChar("REALSXP"));
    SET_STRING_ELT(nms, CPLXSXP, mkChar("CPLXSXP"));
    SET_STRING_ELT(nms, STRSXP, mkChar("STRSXP"));
    SET_STRING_ELT(nms, DOTSXP, mkChar("DOTSXP"));
    SET_STRING_ELT(nms, ANYSXP, mkChar("ANYSXP"));
    SET_STRING_ELT(nms, VECSXP, mkChar("VECSXP"));
    SET_STRING_ELT(nms, EXPRSXP, mkChar("EXPRSXP"));
#ifdef BYTECODE
    SET_STRING_ELT(nms, BCODESXP, mkChar("BCODESXP"));
#endif
    SET_STRING_ELT(nms, EXTPTRSXP, mkChar("EXTPTRSXP"));
    SET_STRING_ELT(nms, WEAKREFSXP, mkChar("WEAKREFSXP"));
    setAttrib(ans, R_NamesSymbol, nms);

    BEGIN_SUSPEND_INTERRUPTS {
      int gen;

      /* run a full GC to make sure that all stuff in use in in Old space */
      num_old_gens_to_collect = NUM_OLD_GENERATIONS;
      R_gc();
      for (gen = 0; gen < NUM_OLD_GENERATIONS; gen++) {
	for (i = 0; i < NUM_NODE_CLASSES; i++) {
	  SEXP s;
	  for (s = NEXT_NODE(R_GenHeap[i].Old[gen]);
	       s != R_GenHeap[i].Old[gen];
	       s = NEXT_NODE(s))
	    INTEGER(ans)[TYPEOF(s)]++;
	}
      }
    } END_SUSPEND_INTERRUPTS;
    UNPROTECT(2);
    return ans;
}

/* "protect" push a single argument onto R_PPStack */

/* In handling a stack overflow we have to be careful not to use
   PROTECT. error("protect(): stack overflow") would call deparse1,
   which uses PROTECT and segfaults.*/

/* However, the traceback creation in the normal error handler also
   does a PROTECT, as does the jumping code, at least if there are
   cleanup expressions to handle on the way out.  So for the moment
   we'll allocate a slightly larger PP stack and only enable the added
   red zone during handling of a stack overflow error.  LT */

static void reset_pp_stack(void *data)
{
    R_size_t *poldpps = data;
    R_PPStackSize =  *poldpps;
}


void protect_error() 
{
  RCNTXT cntxt;
  R_size_t oldpps = R_PPStackSize;

  begincontext(&cntxt, CTXT_CCODE, R_NilValue, R_NilValue, R_NilValue,
	       R_NilValue, R_NilValue);
  cntxt.cend = &reset_pp_stack;
  cntxt.cenddata = &oldpps;

  if (R_PPStackSize < R_RealPPStackSize)
    R_PPStackSize = R_RealPPStackSize;
  errorcall(R_NilValue, _("protect(): protection stack overflow"));

  endcontext(&cntxt); /* not reached */
}


SEXP protect(SEXP s)
{
  if (R_PPStackTop >= R_PPStackSize) { 
    protect_error();
  }
  R_PPStack[R_PPStackTop++] = s;
  return s;
}


/* "unprotect" pop argument list from top of R_PPStack */

void unprotect(int l)
{
    if (R_PPStackTop >=  l)
	R_PPStackTop -= l;
    else
	error(_("unprotect(): stack imbalance"));
}

/* "unprotect_ptr" remove pointer from somewhere in R_PPStack */

void unprotect_ptr(SEXP s)
{
    int i = R_PPStackTop;

    /* go look for  s  in  R_PPStack */
    /* (should be among the top few items) */
    do {
    	if (i == 0)
	    error(_("unprotect_ptr: pointer not found"));
    } while ( R_PPStack[--i] != s );

    /* OK, got it, and  i  is indexing its location */
    /* Now drop stack above it */

    do {
    	R_PPStack[i] = R_PPStack[i + 1];
    } while ( i++ < R_PPStackTop );

    R_PPStackTop--;
}

void R_ProtectWithIndex(SEXP s, PROTECT_INDEX *pi)
{
    protect(s);
    *pi = R_PPStackTop - 1;
}

void R_Reprotect(SEXP s, PROTECT_INDEX i)
{
    R_PPStack[i] = s;
}


/* "initStack" initialize environment stack */
void initStack(void)
{
    R_PPStackTop = 0;
}


/* S-like wrappers for calloc, realloc and free that check for error
   conditions */

void *R_chk_calloc(size_t nelem, size_t elsize)
{
    void *p;
#ifndef HAVE_WORKING_CALLOC
    if(nelem == 0)
	return(NULL);
#endif
    p = calloc(nelem, elsize);
    if(!p) error(_("Calloc could not allocate (%d of %d) memory"), 
		 nelem, elsize);
    return(p);
}
void *R_chk_realloc(void *ptr, size_t size)
{
    void *p;
    /* Protect against broken realloc */
    if(ptr) p = realloc(ptr, size); else p = malloc(size);
    if(!p) error(_("Realloc could not re-allocate (size %d) memory"), size);
    return(p);
}
void R_chk_free(void *ptr)
{
    /* S-PLUS warns here, but there seems no reason to do so */
    /* if(!ptr) warning("attempt to free NULL pointer by Free"); */
    if(ptr) free(ptr); /* ANSI C says free has no effect on NULL, but
			  better to be safe here */
}

/* This code keeps a list of objects which are not assigned to variables
   but which are required to persist across garbage collections.  The
   objects are registered with R_PreserveObject and deregistered with
   R_ReleaseObject. */

void R_PreserveObject(SEXP object)
{
    R_PreciousList = CONS(object, R_PreciousList);
}

static SEXP RecursiveRelease(SEXP object, SEXP list)
{
    if (!isNull(list)) {
        if (object == CAR(list))
            return CDR(list);
        else
            CDR(list) = RecursiveRelease(object, CDR(list));
    }
    return list;
}

void R_ReleaseObject(SEXP object)
{
    R_PreciousList =  RecursiveRelease(object, R_PreciousList);
}


/* External Pointer Objects */
SEXP R_MakeExternalPtr(void *p, SEXP tag, SEXP prot)
{
    SEXP s = allocSExp(EXTPTRSXP);
    EXTPTR_PTR(s) = p;
    EXTPTR_PROT(s) = prot;
    EXTPTR_TAG(s) = tag;
    return s;
}

void *R_ExternalPtrAddr(SEXP s)
{
    return EXTPTR_PTR(s);
}

SEXP R_ExternalPtrTag(SEXP s)
{
    return EXTPTR_TAG(s);
}

SEXP R_ExternalPtrProtected(SEXP s)
{
    return EXTPTR_PROT(s);
}

void R_ClearExternalPtr(SEXP s)
{
    EXTPTR_PTR(s) = NULL;
}

void R_SetExternalPtrAddr(SEXP s, void *p)
{
    EXTPTR_PTR(s) = p;
}

void R_SetExternalPtrTag(SEXP s, SEXP tag)
{
    CHECK_OLD_TO_NEW(s, tag);
    EXTPTR_TAG(s) = tag;
}

void R_SetExternalPtrProtected(SEXP s, SEXP p)
{
    CHECK_OLD_TO_NEW(s, p);
    EXTPTR_PROT(s) = p;
}


/* The following functions are replacements for the accessor macros.
   They are used by code that does not have direct access to the
   internal representation of objects.  The assignment functions
   implement the write barrier. */

/* General Cons Cell Attributes */
SEXP (ATTRIB)(SEXP x) { return ATTRIB(x); }
int (OBJECT)(SEXP x) { return OBJECT(x); }
int (MARK)(SEXP x) { return MARK(x); }
int (TYPEOF)(SEXP x) { return TYPEOF(x); }
int (NAMED)(SEXP x) { return NAMED(x); }

void (SET_ATTRIB)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); ATTRIB(x) = v; }
void (SET_OBJECT)(SEXP x, int v) { SET_OBJECT(x, v); }
void (SET_TYPEOF)(SEXP x, int v) { SET_TYPEOF(x, v); }
void (SET_NAMED)(SEXP x, int v) { SET_NAMED(x, v); }

/* Vector Accessors */
int (LENGTH)(SEXP x) { return LENGTH(x); }
int (TRUELENGTH)(SEXP x) { return TRUELENGTH(x); }
char *(R_CHAR)(SEXP x) { return CHAR(x); }
SEXP (STRING_ELT)(SEXP x, int i) { return STRING_ELT(x, i); }
SEXP (VECTOR_ELT)(SEXP x, int i) { return VECTOR_ELT(x, i); }
int (LEVELS)(SEXP x) { return LEVELS(x); }

int *(LOGICAL)(SEXP x) { return LOGICAL(x); }
int *(INTEGER)(SEXP x) { return INTEGER(x); }
Rbyte *(RAW)(SEXP x) { return RAW(x); }
double *(REAL)(SEXP x) { return REAL(x); }
Rcomplex *(COMPLEX)(SEXP x) { return COMPLEX(x); }
SEXP *(STRING_PTR)(SEXP x) { return STRING_PTR(x); }
SEXP *(VECTOR_PTR)(SEXP x)
{
  error(_("not safe to return vector pointer"));
  return NULL;
}

void (SETLENGTH)(SEXP x, int v) { SETLENGTH(x, v); }
void (SET_TRUELENGTH)(SEXP x, int v) { SET_TRUELENGTH(x, v); }
void (SET_STRING_ELT)(SEXP x, int i, SEXP v) { CHECK_OLD_TO_NEW(x, v); STRING_ELT(x, i) = v; }
SEXP (SET_VECTOR_ELT)(SEXP x, int i, SEXP v) { CHECK_OLD_TO_NEW(x, v); return VECTOR_ELT(x, i) =v; }
int (SETLEVELS)(SEXP x, int v) { return SETLEVELS(x, v); }

/* List Accessors */
SEXP (TAG)(SEXP e) { return TAG(e); }
SEXP (CAR)(SEXP e) { return CAR(e); }
SEXP (CDR)(SEXP e) {return CDR(e); }
SEXP (CAAR)(SEXP e) { return CAAR(e); }
SEXP (CDAR)(SEXP e) { return CDAR(e); }
SEXP (CADR)(SEXP e) { return CADR(e); }
SEXP (CDDR)(SEXP e) { return CDDR(e); }
SEXP (CADDR)(SEXP e) { return CADDR(e); }
SEXP (CADDDR)(SEXP e) { return CADDDR(e); }
SEXP (CAD4R)(SEXP e) { return CAD4R(e); }
int (MISSING)(SEXP x) { return MISSING(x); }

void (SET_TAG)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); TAG(x) = v; }

SEXP (SETCAR)(SEXP x, SEXP y)
{
  if (x == NULL || x == R_NilValue)
    error(_("bad value"));
  CHECK_OLD_TO_NEW(x, y);
  CAR(x) = y;
  return y;
}

SEXP (SETCDR)(SEXP x, SEXP y)
{
  if (x == NULL || x == R_NilValue)
    error(_("bad value"));
  CHECK_OLD_TO_NEW(x, y);
  CDR(x) = y;
  return y;
}

SEXP (SETCADR)(SEXP x, SEXP y)
{
  SEXP cell;
  if (x == NULL || x == R_NilValue ||
      CDR(x) == NULL || CDR(x) == R_NilValue)
    error(_("bad value"));
  cell = CDR(x);
  CHECK_OLD_TO_NEW(cell, y);
  CAR(cell) = y;
  return y;
}

SEXP (SETCADDR)(SEXP x, SEXP y)
{
  SEXP cell;
  if (x == NULL || x == R_NilValue ||
      CDR(x) == NULL || CDR(x) == R_NilValue ||
      CDDR(x) == NULL || CDDR(x) == R_NilValue)
    error(_("bad value"));
  cell = CDDR(x);
  CHECK_OLD_TO_NEW(cell, y);
  CAR(cell) = y;
  return y;
}

#define CDDDR(x) CDR(CDR(CDR(x)))

SEXP (SETCADDDR)(SEXP x, SEXP y)
{
  SEXP cell;
  if (x == NULL || x == R_NilValue ||
      CDR(x) == NULL || CDR(x) == R_NilValue ||
      CDDR(x) == NULL || CDDR(x) == R_NilValue ||
      CDDDR(x) == NULL || CDDDR(x) == R_NilValue)
    error(_("bad value"));
  cell = CDDDR(x);
  CHECK_OLD_TO_NEW(cell, y);
  CAR(cell) = y;
  return y;
}

#define CD4R(x) CDR(CDR(CDR(CDR(x))))

SEXP (SETCAD4R)(SEXP x, SEXP y)
{
  SEXP cell;
  if (x == NULL || x == R_NilValue ||
      CDR(x) == NULL || CDR(x) == R_NilValue ||
      CDDR(x) == NULL || CDDR(x) == R_NilValue ||
      CDDDR(x) == NULL || CDDDR(x) == R_NilValue ||
      CD4R(x) == NULL || CD4R(x) == R_NilValue)
    error(_("bad value"));
  cell = CD4R(x);
  CHECK_OLD_TO_NEW(cell, y);
  CAR(cell) = y;
  return y;
}

void (SET_MISSING)(SEXP x, int v) { SET_MISSING(x, v); }

/* Closure Accessors */
SEXP (FORMALS)(SEXP x) { return FORMALS(x); }
SEXP (BODY)(SEXP x) { return BODY(x); }
SEXP (CLOENV)(SEXP x) { return CLOENV(x); }
int (DEBUG)(SEXP x) { return DEBUG(x); }
int (TRACE)(SEXP x) { return TRACE(x); }

void (SET_FORMALS)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); FORMALS(x) = v; }
void (SET_BODY)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); BODY(x) = v; }
void (SET_CLOENV)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); CLOENV(x) = v; }
void (SET_DEBUG)(SEXP x, int v) { SET_DEBUG(x, v); }
void (SET_TRACE)(SEXP x, int v) { SET_TRACE(x, v); }

/* RCC Closure accessors */
SEXP (RCC_CLOSXP_SET_CLOENV)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); RCC_CLOSXP_CLOENV(x) = v; }
SEXP (RCC_CLOSXP_SET_FORMALS)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); RCC_CLOSXP_FORMALS(x) = v; }
SEXP (RCC_CLOSXP_SET_FUN)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); RCC_CLOSXP_FUN(x) = v; }

SEXP (RCC_FUNSXP_SET_BODY_EXPR)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); RCC_FUNSXP_BODY_EXPR(x) = v; }
SEXP (RCC_FUNSXP_SET_CFUN)(SEXP x, RCC_CCODE v) { RCC_FUNSXP_CFUN(x) = v; }

/* Primitive Accessors */
int (PRIMOFFSET)(SEXP x) { return PRIMOFFSET(x); }

void (SET_PRIMOFFSET)(SEXP x, int v) { SET_PRIMOFFSET(x, v); }

/* Symbol Accessors */
SEXP (PRINTNAME)(SEXP x) { return PRINTNAME(x); }
SEXP (SYMVALUE)(SEXP x) { return SYMVALUE(x); }
SEXP (INTERNAL)(SEXP x) { return INTERNAL(x); }
int (DDVAL)(SEXP x) { return DDVAL(x); }

void (SET_PRINTNAME)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRINTNAME(x) = v; }
void (SET_SYMVALUE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); SYMVALUE(x) = v; }
void (SET_INTERNAL)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); INTERNAL(x) = v; }
void (SET_DDVAL)(SEXP x, int v) { SET_DDVAL(x, v); }

/* Environment Accessors */
SEXP (FRAME)(SEXP x) { return FRAME(x); }
SEXP (ENCLOS)(SEXP x) { return ENCLOS(x); }
SEXP (HASHTAB)(SEXP x) { return HASHTAB(x); }
int (ENVFLAGS)(SEXP x) { return ENVFLAGS(x); }

void (SET_FRAME)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); FRAME(x) = v; }
void (SET_ENCLOS)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); ENCLOS(x) = v; }
void (SET_HASHTAB)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); HASHTAB(x) = v; }
void (SET_ENVFLAGS)(SEXP x, int v) { SET_ENVFLAGS(x, v); }

/* Promise Accessors */
SEXP (PRCODE)(SEXP x) { return PRCODE(x); }
SEXP (PRENV)(SEXP x) { return PRENV(x); }
SEXP (PRVALUE)(SEXP x) { return PRVALUE(x); }
int (PRSEEN)(SEXP x) { return PRSEEN(x); }

void (SET_PRENV)(SEXP x, SEXP v){ CHECK_OLD_TO_NEW(x, v); PRENV(x) = v; }
void (SET_PRVALUE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRVALUE(x) = v; }
void (SET_PRCODE)(SEXP x, SEXP v) { CHECK_OLD_TO_NEW(x, v); PRCODE(x) = v; }
void (SET_PRSEEN)(SEXP x, int v) { SET_PRSEEN(x, v); }

/* Hashing Accessors */
int (HASHASH)(SEXP x) { return HASHASH(x); }
int (HASHVALUE)(SEXP x) { return HASHVALUE(x); }

void (SET_HASHASH)(SEXP x, int v) { SET_HASHASH(x, v); }
void (SET_HASHVALUE)(SEXP x, int v) { SET_HASHVALUE(x, v); }
