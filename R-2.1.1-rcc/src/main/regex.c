/* Extended regular expression matching and search library.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Isamu Hasegawa <isamu@yamato.ibm.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* AIX requires this to be the first thing in the file. */
#if defined _AIX && !defined REGEX_MALLOC
  #pragma alloca
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
/* for declaration of mempcpy */
#define _GNU_SOURCE 1

#ifndef USE_SYSTEM_REGEX
#include <R_ext/Error.h>

#include <ctype.h>
#include <limits.h> /* needs to come before regex.h */
#include <stdlib.h> /* for abort, and malloc etc */
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
# include <assert.h>
#else
# undef assert
# define assert(e)
#endif

/* Make alloca work the best possible way.  */
/* glibc has already included alloca.h */
#ifndef _ALLOCA_H
#  ifdef __GNUC__
#   define alloca __builtin_alloca
#  else /* not __GNUC__ */
#   ifdef HAVE_ALLOCA_H
#    include <alloca.h>
#   endif /* HAVE_ALLOCA_H */
#  endif /* not __GNUC__ */
#endif

/* POSIX says that <sys/types.h> must be included (by the caller) before
   <regex.h>.  */
#include <sys/types.h>
#include "Rregex.h"

#if defined HAVE_LOCALE_H || defined _LIBC
# include <locale.h>
#endif
#if defined HAVE_WCHAR_H || defined _LIBC
# include <wchar.h>
#endif /* HAVE_WCHAR_H || _LIBC */
#if defined HAVE_WCTYPE_H || defined _LIBC
# include <wctype.h>
#endif /* HAVE_WCTYPE_H || _LIBC */

/* In case that the system doesn't have isblank().  */
#if !defined _LIBC && !defined HAVE_ISBLANK && !defined isblank
# define isblank(ch) ((ch) == ' ' || (ch) == '\t')
#endif

#ifdef _LIBC
# ifndef _RE_DEFINE_LOCALE_FUNCTIONS
#  define _RE_DEFINE_LOCALE_FUNCTIONS 1
#   include <locale/localeinfo.h>
#   include <locale/elem-hash.h>
#   include <locale/coll-lookup.h>
# endif
#endif

/* This is for other GNU distributions with internationalized messages.
#if HAVE_LIBINTL_H || defined _LIBC
# include <libintl.h>
# ifdef _LIBC
#  undef gettext
#  define gettext(msgid) \
  INTUSE(__dcgettext) (INTUSE(_libc_intl_domainname), msgid, LC_MESSAGES)
# endif
#else
# define gettext(msgid) (msgid)
#endif
*/
#ifdef ENABLE_NLS
# include <libintl.h>
#else
# define gettext(msgid) (msgid)
#endif

#ifndef gettext_noop
/* This define is so xgettext can find the internationalizable
   strings.  */
# define gettext_noop(String) String
#endif

#ifdef SUPPORT_MBCS
#if (defined MB_CUR_MAX && defined HAVE_LOCALE_H && defined HAVE_WCTYPE_H && defined HAVE_WCHAR_H && defined HAVE_WCRTOMB && defined HAVE_MBRTOWC && defined HAVE_WCSCOLL)
# define RE_ENABLE_I18N
#endif
#endif

#if __GNUC__ >= 3
# define BE(expr, val) __builtin_expect (expr, val)
#else
# define BE(expr, val) (expr)
/*# define inline */
#endif

/* Number of bits in a byte.  */
#define BYTE_BITS 8
/* Number of single byte character.  */
#define SBC_MAX 256

#define COLL_ELEM_LEN_MAX 8

/* The character which represents newline.  */
#define NEWLINE_CHAR '\n'
#define WIDE_NEWLINE_CHAR L'\n'

/* Rename to standard API for using out of glibc.  */
#ifndef _LIBC
# define __wctype wctype
# define __iswctype iswctype
# define __btowc btowc
#undef __mempcpy
# define __mempcpy mempcpy
# define __wcrtomb wcrtomb
# define attribute_hidden
#endif /* not _LIBC */

#ifdef __GNUC__
# define __attribute __attribute__
#else
# define __attribute(x)
#endif

/* extern const char __re_error_msgid[] attribute_hidden;
   extern const size_t __re_error_msgid_idx[] attribute_hidden; */

/* Number of bits in an unsinged int.  */
#define UINT_BITS (sizeof (unsigned int) * BYTE_BITS)
/* Number of unsigned int in an bit_set.  */
#define BITSET_UINTS ((SBC_MAX + UINT_BITS - 1) / UINT_BITS)
typedef unsigned int bitset[BITSET_UINTS];
typedef unsigned int *re_bitset_ptr_t;

#define bitset_set(set,i) (set[i / UINT_BITS] |= 1 << i % UINT_BITS)
#define bitset_clear(set,i) (set[i / UINT_BITS] &= ~(1 << i % UINT_BITS))
#define bitset_contain(set,i) (set[i / UINT_BITS] & (1 << i % UINT_BITS))
#define bitset_empty(set) memset (set, 0, sizeof (unsigned int) * BITSET_UINTS)
#define bitset_set_all(set) \
  memset (set, 255, sizeof (unsigned int) * BITSET_UINTS)
#define bitset_copy(dest,src) \
  memcpy (dest, src, sizeof (unsigned int) * BITSET_UINTS)
static R_INLINE void bitset_not (bitset set);
static R_INLINE void bitset_merge (bitset dest, const bitset src);
static R_INLINE void bitset_not_merge (bitset dest, const bitset src);
static R_INLINE void bitset_mask (bitset dest, const bitset src);

#define PREV_WORD_CONSTRAINT 0x0001
#define PREV_NOTWORD_CONSTRAINT 0x0002
#define NEXT_WORD_CONSTRAINT 0x0004
#define NEXT_NOTWORD_CONSTRAINT 0x0008
#define PREV_NEWLINE_CONSTRAINT 0x0010
#define NEXT_NEWLINE_CONSTRAINT 0x0020
#define PREV_BEGBUF_CONSTRAINT 0x0040
#define NEXT_ENDBUF_CONSTRAINT 0x0080
#define DUMMY_CONSTRAINT 0x0100

typedef enum
{
  INSIDE_WORD = PREV_WORD_CONSTRAINT | NEXT_WORD_CONSTRAINT,
  WORD_FIRST = PREV_NOTWORD_CONSTRAINT | NEXT_WORD_CONSTRAINT,
  WORD_LAST = PREV_WORD_CONSTRAINT | NEXT_NOTWORD_CONSTRAINT,
  LINE_FIRST = PREV_NEWLINE_CONSTRAINT,
  LINE_LAST = NEXT_NEWLINE_CONSTRAINT,
  BUF_FIRST = PREV_BEGBUF_CONSTRAINT,
  BUF_LAST = NEXT_ENDBUF_CONSTRAINT,
  WORD_DELIM = DUMMY_CONSTRAINT
} re_context_type;

typedef struct
{
  int alloc;
  int nelem;
  int *elems;
} re_node_set;

typedef enum
{
  NON_TYPE = 0,

  /* Node type, These are used by token, node, tree.  */
  CHARACTER = 1,
  END_OF_RE = 2,
  SIMPLE_BRACKET = 3,
  OP_BACK_REF = 4,
  OP_PERIOD = 5,
#ifdef RE_ENABLE_I18N
  COMPLEX_BRACKET = 6,
#endif /* RE_ENABLE_I18N */
  OP_UTF8_PERIOD = 7,

  /* We define EPSILON_BIT as a macro so that OP_OPEN_SUBEXP is used
     when the debugger shows values of this enum type.  */
#define EPSILON_BIT 8
  OP_OPEN_SUBEXP = EPSILON_BIT | 0,
  OP_CLOSE_SUBEXP = EPSILON_BIT | 1,
  OP_ALT = EPSILON_BIT | 2,
  OP_DUP_ASTERISK = EPSILON_BIT | 3,
  OP_DUP_PLUS = EPSILON_BIT | 4,
  OP_DUP_QUESTION = EPSILON_BIT | 5,
  ANCHOR = EPSILON_BIT | 6,

  /* Tree type, these are used only by tree. */
  CONCAT = 16,

  /* Token type, these are used only by token.  */
  OP_OPEN_BRACKET = 17,
  OP_CLOSE_BRACKET,
  OP_CHARSET_RANGE,
  OP_OPEN_DUP_NUM,
  OP_CLOSE_DUP_NUM,
  OP_NON_MATCH_LIST,
  OP_OPEN_COLL_ELEM,
  OP_CLOSE_COLL_ELEM,
  OP_OPEN_EQUIV_CLASS,
  OP_CLOSE_EQUIV_CLASS,
  OP_OPEN_CHAR_CLASS,
  OP_CLOSE_CHAR_CLASS,
  OP_WORD,
  OP_NOTWORD,
  OP_SPACE,
  OP_NOTSPACE,
  BACK_SLASH

} re_token_type_t;

#ifdef RE_ENABLE_I18N
typedef struct
{
  /* Multibyte characters.  */
  wchar_t *mbchars;

  /* Collating symbols.  */
# ifdef _LIBC
  int32_t *coll_syms;
# endif

  /* Equivalence classes. */
# ifdef _LIBC
  int32_t *equiv_classes;
# endif

  /* Range expressions. */
# ifdef _LIBC
  uint32_t *range_starts;
  uint32_t *range_ends;
# else /* not _LIBC */
  wchar_t *range_starts;
  wchar_t *range_ends;
# endif /* not _LIBC */

  /* Character classes. */
  wctype_t *char_classes;

  /* If this character set is the non-matching list.  */
  unsigned int non_match : 1;

  /* # of multibyte characters.  */
  int nmbchars;

  /* # of collating symbols.  */
  int ncoll_syms;

  /* # of equivalence classes. */
  int nequiv_classes;

  /* # of range expressions. */
  int nranges;

  /* # of character classes. */
  int nchar_classes;
} re_charset_t;
#endif /* RE_ENABLE_I18N */

typedef struct
{
  union
  {
    unsigned char c;		/* for CHARACTER */
    re_bitset_ptr_t sbcset;	/* for SIMPLE_BRACKET */
#ifdef RE_ENABLE_I18N
    re_charset_t *mbcset;	/* for COMPLEX_BRACKET */
#endif /* RE_ENABLE_I18N */
    int idx;			/* for BACK_REF */
    re_context_type ctx_type;	/* for ANCHOR */
  } opr;
#if __GNUC__ >= 2
  __extension__ re_token_type_t type : 8;
#else
  re_token_type_t type;
#endif
  unsigned int constraint : 10;	/* context constraint */
  unsigned int duplicated : 1;
#ifdef RE_ENABLE_I18N
  /* These 2 bits can be moved into the union if needed (e.g. if running out
     of bits; move opr.c to opr.c.c and move the flags to opr.c.flags).  */
  unsigned int mb_partial : 1;
#endif
  unsigned int word_char : 1;
} re_token_t;

#define IS_EPSILON_NODE(type) ((type) & EPSILON_BIT)
#define ACCEPT_MB_NODE(type) \
  ((type) >= OP_PERIOD && (type) <= OP_UTF8_PERIOD)

struct re_string_t
{
  /* Indicate the raw buffer which is the original string passed as an
     argument of regexec(), re_search(), etc..  */
  const unsigned char *raw_mbs;
  /* Store the multibyte string.  In case of "case insensitive mode" like
     REG_ICASE, upper cases of the string are stored, otherwise MBS points
     the same address that RAW_MBS points.  */
  unsigned char *mbs;
#ifdef RE_ENABLE_I18N
  /* Store the wide character string which is corresponding to MBS.  */
  wint_t *wcs;
  int *offsets;
  mbstate_t cur_state;
#else
  int *offsets;
#endif
  /* Index in RAW_MBS.  Each character mbs[i] corresponds to
     raw_mbs[raw_mbs_idx + i].  */
  int raw_mbs_idx;
  /* The length of the valid characters in the buffers.  */
  int valid_len;
  /* The corresponding number of bytes in raw_mbs array.  */
  int valid_raw_len;
  /* The length of the buffers MBS and WCS.  */
  int bufs_len;
  /* The index in MBS, which is updated by re_string_fetch_byte.  */
  int cur_idx;
  /* length of RAW_MBS array.  */
  int raw_len;
  /* This is RAW_LEN - RAW_MBS_IDX + VALID_LEN - VALID_RAW_LEN.  */
  int len;
  /* End of the buffer may be shorter than its length in the cases such
     as re_match_2, re_search_2.  Then, we use STOP for end of the buffer
     instead of LEN.  */
  int raw_stop;
  /* This is RAW_STOP - RAW_MBS_IDX adjusted through OFFSETS.  */
  int stop;

  /* The context of mbs[0].  We store the context independently, since
     the context of mbs[0] may be different from raw_mbs[0], which is
     the beginning of the input string.  */
  unsigned int tip_context;
  /* The translation passed as a part of an argument of re_compile_pattern.  */
  RE_TRANSLATE_TYPE trans;
  /* 1 if REG_ICASE.  */
  unsigned char icase;
  unsigned char is_utf8;
  unsigned char map_notascii;
  unsigned char mbs_allocated;
  unsigned char offsets_needed;
  int mb_cur_max;
};
typedef struct re_string_t re_string_t;


struct re_dfa_t;
typedef struct re_dfa_t re_dfa_t;
#ifndef RE_NO_INTERNAL_PROTOTYPES
static reg_errcode_t re_string_allocate (re_string_t *pstr, const char *str,
					 int len, int init_len,
					 RE_TRANSLATE_TYPE trans, int icase,
					 const re_dfa_t *dfa);
static reg_errcode_t re_string_construct (re_string_t *pstr, const char *str,
					  int len, RE_TRANSLATE_TYPE trans,
					  int icase, const re_dfa_t *dfa);
static reg_errcode_t re_string_reconstruct (re_string_t *pstr, int idx,
					    int eflags, int newline);
static reg_errcode_t re_string_realloc_buffers (re_string_t *pstr,
						int new_buf_len);
# ifdef RE_ENABLE_I18N
static void build_wcs_buffer (re_string_t *pstr);
static int build_wcs_upper_buffer (re_string_t *pstr);
# endif /* RE_ENABLE_I18N */
static void build_upper_buffer (re_string_t *pstr);
static void re_string_translate_buffer (re_string_t *pstr);
static void re_string_destruct (re_string_t *pstr);
# ifdef RE_ENABLE_I18N
static int re_string_elem_size_at (const re_string_t *pstr, int idx);
static R_INLINE int re_string_char_size_at (const re_string_t *pstr, int idx);
static R_INLINE wint_t re_string_wchar_at (const re_string_t *pstr, int idx);
# endif /* RE_ENABLE_I18N */
static unsigned int re_string_context_at (const re_string_t *input, int idx,
					  int eflags, int newline_anchor);
static unsigned char re_string_peek_byte_case (const re_string_t *pstr,
					       int idx);
static unsigned char re_string_fetch_byte_case (re_string_t *pstr);
#endif
#define re_string_peek_byte(pstr, offset) \
  ((pstr)->mbs[(pstr)->cur_idx + offset])
#define re_string_fetch_byte(pstr) \
  ((pstr)->mbs[(pstr)->cur_idx++])
#define re_string_first_byte(pstr, idx) \
  ((idx) == (pstr)->valid_len || (pstr)->wcs[idx] != WEOF)
#define re_string_is_single_byte_char(pstr, idx) \
  ((pstr)->wcs[idx] != WEOF && ((pstr)->valid_len == (idx) \
				|| (pstr)->wcs[(idx) + 1] != WEOF))
#define re_string_eoi(pstr) ((pstr)->stop <= (pstr)->cur_idx)
#define re_string_cur_idx(pstr) ((pstr)->cur_idx)
#define re_string_get_buffer(pstr) ((pstr)->mbs)
#define re_string_length(pstr) ((pstr)->len)
#define re_string_byte_at(pstr,idx) ((pstr)->mbs[idx])
#define re_string_skip_bytes(pstr,idx) ((pstr)->cur_idx += (idx))
#define re_string_set_index(pstr,idx) ((pstr)->cur_idx = (idx))

#define re_malloc(t,n) ((t *) malloc ((n) * sizeof (t)))
#define re_realloc(p,t,n) ((t *) realloc (p, (n) * sizeof (t)))
#define re_free(p) free (p)

struct bin_tree_t
{
  struct bin_tree_t *parent;
  struct bin_tree_t *left;
  struct bin_tree_t *right;

  /* `node_idx' is the index in dfa->nodes, if `type' == 0.
     Otherwise `type' indicate the type of this node.  */
  re_token_type_t type;
  int node_idx;

  int first;
  int next;
  re_node_set eclosure;
};
typedef struct bin_tree_t bin_tree_t;

#define BIN_TREE_STORAGE_SIZE \
  ((1024 - sizeof (void *)) / sizeof (bin_tree_t))

struct bin_tree_storage_t
{
  struct bin_tree_storage_t *next;
  bin_tree_t data[BIN_TREE_STORAGE_SIZE];
};
typedef struct bin_tree_storage_t bin_tree_storage_t;

#define CONTEXT_WORD 1
#define CONTEXT_NEWLINE (CONTEXT_WORD << 1)
#define CONTEXT_BEGBUF (CONTEXT_NEWLINE << 1)
#define CONTEXT_ENDBUF (CONTEXT_BEGBUF << 1)

#define IS_WORD_CONTEXT(c) ((c) & CONTEXT_WORD)
#define IS_NEWLINE_CONTEXT(c) ((c) & CONTEXT_NEWLINE)
#define IS_BEGBUF_CONTEXT(c) ((c) & CONTEXT_BEGBUF)
#define IS_ENDBUF_CONTEXT(c) ((c) & CONTEXT_ENDBUF)
#define IS_ORDINARY_CONTEXT(c) ((c) == 0)

#define IS_WORD_CHAR(ch) (isalnum (ch) || (ch) == '_')
#define IS_NEWLINE(ch) ((ch) == NEWLINE_CHAR)
#define IS_WIDE_WORD_CHAR(ch) (iswalnum (ch) || (ch) == L'_')
#define IS_WIDE_NEWLINE(ch) ((ch) == WIDE_NEWLINE_CHAR)

#define NOT_SATISFY_PREV_CONSTRAINT(constraint,context) \
 ((((constraint) & PREV_WORD_CONSTRAINT) && !IS_WORD_CONTEXT (context)) \
  || ((constraint & PREV_NOTWORD_CONSTRAINT) && IS_WORD_CONTEXT (context)) \
  || ((constraint & PREV_NEWLINE_CONSTRAINT) && !IS_NEWLINE_CONTEXT (context))\
  || ((constraint & PREV_BEGBUF_CONSTRAINT) && !IS_BEGBUF_CONTEXT (context)))

#define NOT_SATISFY_NEXT_CONSTRAINT(constraint,context) \
 ((((constraint) & NEXT_WORD_CONSTRAINT) && !IS_WORD_CONTEXT (context)) \
  || (((constraint) & NEXT_NOTWORD_CONSTRAINT) && IS_WORD_CONTEXT (context)) \
  || (((constraint) & NEXT_NEWLINE_CONSTRAINT) && !IS_NEWLINE_CONTEXT (context)) \
  || (((constraint) & NEXT_ENDBUF_CONSTRAINT) && !IS_ENDBUF_CONTEXT (context)))

struct re_dfastate_t
{
  unsigned int hash;
  re_node_set nodes;
  re_node_set *entrance_nodes;
  struct re_dfastate_t **trtable;
  /* If this state is a special state.
     A state is a special state if the state is the halt state, or
     a anchor.  */
  unsigned int context : 2;
  unsigned int halt : 1;
  /* If this state can accept `multi byte'.
     Note that we refer to multibyte characters, and multi character
     collating elements as `multi byte'.  */
  unsigned int accept_mb : 1;
  /* If this state has backreference node(s).  */
  unsigned int has_backref : 1;
  unsigned int has_constraint : 1;
  unsigned int word_trtable : 1;
};
typedef struct re_dfastate_t re_dfastate_t;

typedef struct
{
  /* start <= node < end  */
  int start;
  int end;
} re_subexp_t;

struct re_state_table_entry
{
  int num;
  int alloc;
  re_dfastate_t **array;
};

/* Array type used in re_sub_match_last_t and re_sub_match_top_t.  */

typedef struct
{
  int next_idx;
  int alloc;
  re_dfastate_t **array;
} state_array_t;

/* Store information about the node NODE whose type is OP_CLOSE_SUBEXP.  */

typedef struct
{
  int node;
  int str_idx; /* The position NODE match at.  */
  state_array_t path;
} re_sub_match_last_t;

/* Store information about the node NODE whose type is OP_OPEN_SUBEXP.
   And information about the node, whose type is OP_CLOSE_SUBEXP,
   corresponding to NODE is stored in LASTS.  */

typedef struct
{
  int str_idx;
  int node;
  int next_last_offset;
  state_array_t *path;
  int alasts; /* Allocation size of LASTS.  */
  int nlasts; /* The number of LASTS.  */
  re_sub_match_last_t **lasts;
} re_sub_match_top_t;

struct re_backref_cache_entry
{
  int node;
  int str_idx;
  int subexp_from;
  int subexp_to;
  int flag;
};

typedef struct
{
  /* EFLAGS of the argument of regexec.  */
  int eflags;
  /* Where the matching ends.  */
  int match_last;
  int last_node;
  /* The string object corresponding to the input string.  */
  re_string_t *input;
  /* The state log used by the matcher.  */
  re_dfastate_t **state_log;
  int state_log_top;
  /* Back reference cache.  */
  int nbkref_ents;
  int abkref_ents;
  struct re_backref_cache_entry *bkref_ents;
  int max_mb_elem_len;
  int nsub_tops;
  int asub_tops;
  re_sub_match_top_t **sub_tops;
} re_match_context_t;

typedef struct
{
  int cur_bkref;
  int cls_subexp_idx;

  re_dfastate_t **sifted_states;
  re_dfastate_t **limited_states;

  re_node_set limits;

  int last_node;
  int last_str_idx;
  int check_subexp;
} re_sift_context_t;

struct re_fail_stack_ent_t
{
  int idx;
  int node;
  regmatch_t *regs;
  re_node_set eps_via_nodes;
};

struct re_fail_stack_t
{
  int num;
  int alloc;
  struct re_fail_stack_ent_t *stack;
};

struct re_dfa_t
{
  re_bitset_ptr_t word_char;
  re_subexp_t *subexps;
  re_token_t *nodes;
  int nodes_alloc;
  int nodes_len;
  int *nexts;
  int *org_indices;
  re_node_set *edests;
  re_node_set *eclosures;
  re_node_set *inveclosures;
  struct re_state_table_entry *state_table;
  re_dfastate_t *init_state;
  re_dfastate_t *init_state_word;
  re_dfastate_t *init_state_nl;
  re_dfastate_t *init_state_begbuf;
  bin_tree_t *str_tree;
  bin_tree_storage_t *str_tree_storage;
  re_bitset_ptr_t sb_char;
  int str_tree_storage_idx;

  /* number of subexpressions `re_nsub' is in regex_t.  */
  int subexps_alloc;
  unsigned int state_hash_mask;
  int states_alloc;
  int init_node;
  int nbackref; /* The number of backreference in this dfa.  */
  /* Bitmap expressing which backreference is used.  */
  unsigned int used_bkref_map;
  unsigned int has_plural_match : 1;
  /* If this dfa has "multibyte node", which is a backreference or
     a node which can accept multibyte character or multi character
     collating element.  */
  unsigned int has_mb_node : 1;
  unsigned int is_utf8 : 1;
  unsigned int map_notascii : 1;
  int mb_cur_max;
#ifdef DEBUG
  char* re_str;
#endif
};

#ifndef RE_NO_INTERNAL_PROTOTYPES
static reg_errcode_t re_node_set_alloc (re_node_set *set, int size);
static reg_errcode_t re_node_set_init_1 (re_node_set *set, int elem);
static reg_errcode_t re_node_set_init_2 (re_node_set *set, int elem1,
					 int elem2);
#define re_node_set_init_empty(set) memset (set, '\0', sizeof (re_node_set))
static reg_errcode_t re_node_set_init_copy (re_node_set *dest,
					    const re_node_set *src);
static reg_errcode_t re_node_set_add_intersect (re_node_set *dest,
						const re_node_set *src1,
						const re_node_set *src2);
static reg_errcode_t re_node_set_init_union (re_node_set *dest,
					     const re_node_set *src1,
					     const re_node_set *src2);
static reg_errcode_t re_node_set_merge (re_node_set *dest,
					const re_node_set *src);
static int re_node_set_insert (re_node_set *set, int elem);
static int re_node_set_compare (const re_node_set *set1,
				const re_node_set *set2);
static int re_node_set_contains (const re_node_set *set, int elem);
static void re_node_set_remove_at (re_node_set *set, int idx);
#define re_node_set_remove(set,id) \
  (re_node_set_remove_at (set, re_node_set_contains (set, id) - 1))
#define re_node_set_empty(p) ((p)->nelem = 0)
#define re_node_set_free(set) re_free ((set)->elems)
static int re_dfa_add_node (re_dfa_t *dfa, re_token_t token, int mode);
static re_dfastate_t *re_acquire_state (reg_errcode_t *err, re_dfa_t *dfa,
					const re_node_set *nodes);
static re_dfastate_t *re_acquire_state_context (reg_errcode_t *err,
						re_dfa_t *dfa,
						const re_node_set *nodes,
						unsigned int context);
static void free_state (re_dfastate_t *state);
#endif


typedef enum
{
  SB_CHAR,
  MB_CHAR,
  EQUIV_CLASS,
  COLL_SYM,
  CHAR_CLASS
} bracket_elem_type;

typedef struct
{
  bracket_elem_type type;
  union
  {
    unsigned char ch;
    unsigned char *name;
    wchar_t wch;
  } opr;
} bracket_elem_t;


/* Inline functions for bitset operation.  */
static R_INLINE void
bitset_not (set)
     bitset set;
{
  int bitset_i;
  for (bitset_i = 0; bitset_i < BITSET_UINTS; ++bitset_i)
    set[bitset_i] = ~set[bitset_i];
}

static R_INLINE void
bitset_merge (dest, src)
     bitset dest;
     const bitset src;
{
  int bitset_i;
  for (bitset_i = 0; bitset_i < BITSET_UINTS; ++bitset_i)
    dest[bitset_i] |= src[bitset_i];
}

static R_INLINE void
bitset_not_merge (dest, src)
     bitset dest;
     const bitset src;
{
  int i;
  for (i = 0; i < BITSET_UINTS; ++i)
    dest[i] |= ~src[i];
}

static R_INLINE void
bitset_mask (dest, src)
     bitset dest;
     const bitset src;
{
  int bitset_i;
  for (bitset_i = 0; bitset_i < BITSET_UINTS; ++bitset_i)
    dest[bitset_i] &= src[bitset_i];
}

#if defined RE_ENABLE_I18N && !defined RE_NO_INTERNAL_PROTOTYPES
/* Inline functions for re_string.  */
static R_INLINE int
re_string_char_size_at (pstr, idx)
     const re_string_t *pstr;
     int idx;
{
  int byte_idx;
  if (pstr->mb_cur_max == 1)
    return 1;
  for (byte_idx = 1; idx + byte_idx < pstr->valid_len; ++byte_idx)
    if (pstr->wcs[idx + byte_idx] != WEOF)
      break;
  return byte_idx;
}

static R_INLINE wint_t
re_string_wchar_at (pstr, idx)
     const re_string_t *pstr;
     int idx;
{
  if (pstr->mb_cur_max == 1)
    return (wint_t) pstr->mbs[idx];
  return (wint_t) pstr->wcs[idx];
}

static int
re_string_elem_size_at (pstr, idx)
     const re_string_t *pstr;
     int idx;
{
#ifdef _LIBC
  const unsigned char *p, *extra;
  const int32_t *table, *indirect;
  int32_t tmp;
# include <locale/weight.h>
  uint_fast32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);

  if (nrules != 0)
    {
      table = (const int32_t *) _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
      extra = (const unsigned char *)
	_NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
      indirect = (const int32_t *) _NL_CURRENT (LC_COLLATE,
						_NL_COLLATE_INDIRECTMB);
      p = pstr->mbs + idx;
      tmp = findidx (&p);
      return p - pstr->mbs - idx;
    }
  else
#endif /* _LIBC */
    return 1;
}
#endif /* RE_ENABLE_I18N */

static void re_string_construct_common (const char *str, int len,
					re_string_t *pstr,
					RE_TRANSLATE_TYPE trans, int icase,
					const re_dfa_t *dfa);
#ifdef RE_ENABLE_I18N
static int re_string_skip_chars (re_string_t *pstr, int new_raw_idx,
				 wint_t *last_wc);
#endif /* RE_ENABLE_I18N */
static re_dfastate_t *create_newstate_common (re_dfa_t *dfa,
					      const re_node_set *nodes,
					      unsigned int hash);
static reg_errcode_t register_state (re_dfa_t *dfa, re_dfastate_t *newstate,
				     unsigned int hash);
static re_dfastate_t *create_ci_newstate (re_dfa_t *dfa,
					  const re_node_set *nodes,
					  unsigned int hash);
static re_dfastate_t *create_cd_newstate (re_dfa_t *dfa,
					  const re_node_set *nodes,
					  unsigned int context,
					  unsigned int hash);
static unsigned int R_INLINE calc_state_hash (const re_node_set *nodes,
					    unsigned int context);

/* Functions for string operation.  */

/* This function allocate the buffers.  It is necessary to call
   re_string_reconstruct before using the object.  */

static reg_errcode_t
re_string_allocate (pstr, str, len, init_len, trans, icase, dfa)
     re_string_t *pstr;
     const char *str;
     int len, init_len, icase;
     RE_TRANSLATE_TYPE trans;
     const re_dfa_t *dfa;
{
  reg_errcode_t ret;
  int init_buf_len;

  /* Ensure at least one character fits into the buffers.  */
  if (init_len < dfa->mb_cur_max)
    init_len = dfa->mb_cur_max;
  init_buf_len = (len + 1 < init_len) ? len + 1: init_len;
  re_string_construct_common (str, len, pstr, trans, icase, dfa);

  ret = re_string_realloc_buffers (pstr, init_buf_len);
  if (BE (ret != REG_NOERROR, 0))
    return ret;

  pstr->mbs = pstr->mbs_allocated ? pstr->mbs : (unsigned char *) str;
  pstr->valid_len = (pstr->mbs_allocated || dfa->mb_cur_max > 1) ? 0 : len;
  pstr->valid_raw_len = pstr->valid_len;
  return REG_NOERROR;
}

/* This function allocate the buffers, and initialize them.  */

static reg_errcode_t
re_string_construct (pstr, str, len, trans, icase, dfa)
     re_string_t *pstr;
     const char *str;
     int len, icase;
     RE_TRANSLATE_TYPE trans;
     const re_dfa_t *dfa;
{
  reg_errcode_t ret;
  re_string_construct_common (str, len, pstr, trans, icase, dfa);

  if (len > 0)
    {
      ret = re_string_realloc_buffers (pstr, len + 1);
      if (BE (ret != REG_NOERROR, 0))
	return ret;
    }
  pstr->mbs = pstr->mbs_allocated ? pstr->mbs : (unsigned char *) str;

  if (icase)
    {
#ifdef RE_ENABLE_I18N
      if (dfa->mb_cur_max > 1)
	{
	  while (1)
	    {
	      ret = build_wcs_upper_buffer (pstr);
	      if (BE (ret != REG_NOERROR, 0))
		return ret;
	      if (pstr->valid_raw_len >= len)
		break;
	      if (pstr->bufs_len > pstr->valid_len + dfa->mb_cur_max)
		break;
	      ret = re_string_realloc_buffers (pstr, pstr->bufs_len * 2);
	      if (BE (ret != REG_NOERROR, 0))
		return ret;
	    }
	}
      else
#endif /* RE_ENABLE_I18N  */
	build_upper_buffer (pstr);
    }
  else
    {
#ifdef RE_ENABLE_I18N
      if (dfa->mb_cur_max > 1)
	build_wcs_buffer (pstr);
      else
#endif /* RE_ENABLE_I18N  */
	{
	  if (trans != NULL)
	    re_string_translate_buffer (pstr);
	  else
	    {
	      pstr->valid_len = pstr->bufs_len;
	      pstr->valid_raw_len = pstr->bufs_len;
	    }
	}
    }

  return REG_NOERROR;
}

/* Helper functions for re_string_allocate, and re_string_construct.  */

static reg_errcode_t
re_string_realloc_buffers (pstr, new_buf_len)
     re_string_t *pstr;
     int new_buf_len;
{
#ifdef RE_ENABLE_I18N
  if (pstr->mb_cur_max > 1)
    {
      wint_t *new_array = re_realloc (pstr->wcs, wint_t, new_buf_len);
      if (BE (new_array == NULL, 0))
	return REG_ESPACE;
      pstr->wcs = new_array;
      if (pstr->offsets != NULL)
	{
	  int *new_array = re_realloc (pstr->offsets, int, new_buf_len);
	  if (BE (new_array == NULL, 0))
	    return REG_ESPACE;
	  pstr->offsets = new_array;
	}
    }
#endif /* RE_ENABLE_I18N  */
  if (pstr->mbs_allocated)
    {
      unsigned char *new_array = re_realloc (pstr->mbs, unsigned char,
					     new_buf_len);
      if (BE (new_array == NULL, 0))
	return REG_ESPACE;
      pstr->mbs = new_array;
    }
  pstr->bufs_len = new_buf_len;
  return REG_NOERROR;
}


static void
re_string_construct_common (str, len, pstr, trans, icase, dfa)
     const char *str;
     int len;
     re_string_t *pstr;
     RE_TRANSLATE_TYPE trans;
     int icase;
     const re_dfa_t *dfa;
{
  memset (pstr, '\0', sizeof (re_string_t));
  pstr->raw_mbs = (const unsigned char *) str;
  pstr->len = len;
  pstr->raw_len = len;
  pstr->trans = trans;
  pstr->icase = icase ? 1 : 0;
  pstr->mbs_allocated = (trans != NULL || icase);
  pstr->mb_cur_max = dfa->mb_cur_max;
  pstr->is_utf8 = dfa->is_utf8;
  pstr->map_notascii = dfa->map_notascii;
  pstr->stop = pstr->len;
  pstr->raw_stop = pstr->stop;
}

#ifdef RE_ENABLE_I18N

/* Build wide character buffer PSTR->WCS.
   If the byte sequence of the string are:
     <mb1>(0), <mb1>(1), <mb2>(0), <mb2>(1), <sb3>
   Then wide character buffer will be:
     <wc1>   , WEOF    , <wc2>   , WEOF    , <wc3>
   We use WEOF for padding, they indicate that the position isn't
   a first byte of a multibyte character.

   Note that this function assumes PSTR->VALID_LEN elements are already
   built and starts from PSTR->VALID_LEN.  */

static void
build_wcs_buffer (pstr)
     re_string_t *pstr;
{
#ifdef _LIBC
  unsigned char buf[pstr->mb_cur_max];
#else
  unsigned char buf[64];
#endif
  mbstate_t prev_st;
  int byte_idx, end_idx, mbclen, remain_len;

  /* Build the buffers from pstr->valid_len to either pstr->len or
     pstr->bufs_len.  */
  end_idx = (pstr->bufs_len > pstr->len) ? pstr->len : pstr->bufs_len;
  for (byte_idx = pstr->valid_len; byte_idx < end_idx;)
    {
      wchar_t wc;
      const char *p;

      remain_len = end_idx - byte_idx;
      prev_st = pstr->cur_state;
      /* Apply the translation if we need.  */
      if (BE (pstr->trans != NULL, 0))
	{
	  int i, ch;

	  for (i = 0; i < pstr->mb_cur_max && i < remain_len; ++i)
	    {
	      ch = pstr->raw_mbs [pstr->raw_mbs_idx + byte_idx + i];
	      buf[i] = pstr->trans[ch];
	    }
	  p = (const char *) buf;
	}
      else
	p = (const char *) pstr->raw_mbs + pstr->raw_mbs_idx + byte_idx;
      mbclen = mbrtowc (&wc, p, remain_len, &pstr->cur_state);
      if (BE (mbclen == (size_t) -2, 0))
	{
	  /* The buffer doesn't have enough space, finish to build.  */
	  pstr->cur_state = prev_st;
	  break;
	}
      else if (BE (mbclen == (size_t) -1 || mbclen == 0, 0))
	{
	  /* We treat these cases as a singlebyte character.  */
	  mbclen = 1;
	  wc = (wchar_t) pstr->raw_mbs[pstr->raw_mbs_idx + byte_idx];
	  if (BE (pstr->trans != NULL, 0))
	    wc = pstr->trans[wc];
	  pstr->cur_state = prev_st;
	}

      /* Write wide character and padding.  */
      pstr->wcs[byte_idx++] = wc;
      /* Write paddings.  */
      for (remain_len = byte_idx + mbclen - 1; byte_idx < remain_len ;)
	pstr->wcs[byte_idx++] = WEOF;
    }
  pstr->valid_len = byte_idx;
  pstr->valid_raw_len = byte_idx;
}

/* Build wide character buffer PSTR->WCS like build_wcs_buffer,
   but for REG_ICASE.  */

static int
build_wcs_upper_buffer (pstr)
     re_string_t *pstr;
{
  mbstate_t prev_st;
  int src_idx, byte_idx, end_idx, mbclen, remain_len;
#ifdef _LIBC
  unsigned char buf[pstr->mb_cur_max];
#else
  unsigned char buf[64];
#endif

  byte_idx = pstr->valid_len;
  end_idx = (pstr->bufs_len > pstr->len) ? pstr->len : pstr->bufs_len;

#ifdef _LIBC
  /* The following optimization assumes that the wchar_t encoding is
     always ISO 10646.  */
  if (! pstr->map_notascii && pstr->trans == NULL && !pstr->offsets_needed)
    {
      while (byte_idx < end_idx)
	{
	  wchar_t wc;

	  if (isascii (pstr->raw_mbs[pstr->raw_mbs_idx + byte_idx])
	      && mbsinit (&pstr->cur_state))
	    {
	      /* In case of a singlebyte character.  */
	      pstr->mbs[byte_idx]
		= toupper (pstr->raw_mbs[pstr->raw_mbs_idx + byte_idx]);
	      /* The next step uses the assumption that wchar_t is encoded
		 with ISO 10646: all ASCII values can be converted like
		 this.  */
	      pstr->wcs[byte_idx] = (wchar_t) pstr->mbs[byte_idx];
	      ++byte_idx;
	      continue;
	    }

	  remain_len = end_idx - byte_idx;
	  prev_st = pstr->cur_state;
	  mbclen = mbrtowc (&wc,
			    ((const char *) pstr->raw_mbs + pstr->raw_mbs_idx
			     + byte_idx), remain_len, &pstr->cur_state);
	  if (BE (mbclen > 0, 1))
	    {
	      wchar_t wcu = wc;
	      if (iswlower (wc))
		{
		  int mbcdlen;

		  wcu = towupper (wc);
		  mbcdlen = wcrtomb (buf, wcu, &prev_st);
		  if (BE (mbclen == mbcdlen, 1))
		    memcpy (pstr->mbs + byte_idx, buf, mbclen);
		  else
		    {
		      src_idx = byte_idx;
		      goto offsets_needed;
		    }
		}
	      else
		memcpy (pstr->mbs + byte_idx,
			pstr->raw_mbs + pstr->raw_mbs_idx + byte_idx, mbclen);
	      pstr->wcs[byte_idx++] = wcu;
	      /* Write paddings.  */
	      for (remain_len = byte_idx + mbclen - 1; byte_idx < remain_len ;)
		pstr->wcs[byte_idx++] = WEOF;
	    }
	  else if (mbclen == (size_t) -1 || mbclen == 0)
	    {
	      /* It is an invalid character or '\0'.  Just use the byte.  */
	      int ch = pstr->raw_mbs[pstr->raw_mbs_idx + byte_idx];
	      pstr->mbs[byte_idx] = ch;
	      /* And also cast it to wide char.  */
	      pstr->wcs[byte_idx++] = (wchar_t) ch;
	      if (BE (mbclen == (size_t) -1, 0))
		pstr->cur_state = prev_st;
	    }
	  else
	    {
	      /* The buffer doesn't have enough space, finish to build.  */
	      pstr->cur_state = prev_st;
	      break;
	    }
	}
      pstr->valid_len = byte_idx;
      pstr->valid_raw_len = byte_idx;
      return REG_NOERROR;
    }
  else
#endif
    for (src_idx = pstr->valid_raw_len; byte_idx < end_idx;)
      {
	wchar_t wc;
	const char *p;
/* offsets_needed: unused */
	remain_len = end_idx - byte_idx;
	prev_st = pstr->cur_state;
	if (BE (pstr->trans != NULL, 0))
	  {
	    int i, ch;

	    for (i = 0; i < pstr->mb_cur_max && i < remain_len; ++i)
	      {
		ch = pstr->raw_mbs [pstr->raw_mbs_idx + src_idx + i];
		buf[i] = pstr->trans[ch];
	      }
	    p = (const char *) buf;
	  }
	else
	  p = (const char *) pstr->raw_mbs + pstr->raw_mbs_idx + src_idx;
	mbclen = mbrtowc (&wc, p, remain_len, &pstr->cur_state);
	if (BE (mbclen > 0, 1))
	  {
	    wchar_t wcu = wc;
	    if (iswlower (wc))
	      {
		int mbcdlen;

		wcu = towupper (wc);
		mbcdlen = wcrtomb ((char *)buf, wcu, &prev_st);
		if (BE (mbclen == mbcdlen, 1))
		  memcpy (pstr->mbs + byte_idx, buf, mbclen);
		else
		  {
		    int i;

		    if (byte_idx + mbcdlen > pstr->bufs_len)
		      {
			pstr->cur_state = prev_st;
			break;
		      }

		    if (pstr->offsets == NULL)
		      {
			pstr->offsets = re_malloc (int, pstr->bufs_len);

			if (pstr->offsets == NULL)
			  return REG_ESPACE;
		      }
		    if (!pstr->offsets_needed)
		      {
			for (i = 0; i < byte_idx; ++i)
			  pstr->offsets[i] = i;
			pstr->offsets_needed = 1;
		      }

		    memcpy (pstr->mbs + byte_idx, buf, mbcdlen);
		    pstr->wcs[byte_idx] = wcu;
		    pstr->offsets[byte_idx] = src_idx;
		    for (i = 1; i < mbcdlen; ++i)
		      {
			pstr->offsets[byte_idx + i]
			  = src_idx + (i < mbclen ? i : mbclen - 1);
			pstr->wcs[byte_idx + i] = WEOF;
		      }
		    pstr->len += mbcdlen - mbclen;
		    if (pstr->raw_stop > src_idx)
		      pstr->stop += mbcdlen - mbclen;
		    end_idx = (pstr->bufs_len > pstr->len)
			      ? pstr->len : pstr->bufs_len;
		    byte_idx += mbcdlen;
		    src_idx += mbclen;
		    continue;
		  }
	      }
	    else
	      memcpy (pstr->mbs + byte_idx, p, mbclen);

	    if (BE (pstr->offsets_needed != 0, 0))
	      {
		int i;
		for (i = 0; i < mbclen; ++i)
		  pstr->offsets[byte_idx + i] = src_idx + i;
	      }
	    src_idx += mbclen;

	    pstr->wcs[byte_idx++] = wcu;
	    /* Write paddings.  */
	    for (remain_len = byte_idx + mbclen - 1; byte_idx < remain_len ;)
	      pstr->wcs[byte_idx++] = WEOF;
	  }
	else if (mbclen == (size_t) -1 || mbclen == 0)
	  {
	    /* It is an invalid character or '\0'.  Just use the byte.  */
	    int ch = pstr->raw_mbs[pstr->raw_mbs_idx + src_idx];

	    if (BE (pstr->trans != NULL, 0))
	      ch = pstr->trans [ch];
	    pstr->mbs[byte_idx] = ch;

	    if (BE (pstr->offsets_needed != 0, 0))
	      pstr->offsets[byte_idx] = src_idx;
	    ++src_idx;

	    /* And also cast it to wide char.  */
	    pstr->wcs[byte_idx++] = (wchar_t) ch;
	    if (BE (mbclen == (size_t) -1, 0))
	      pstr->cur_state = prev_st;
	  }
	else
	  {
	    /* The buffer doesn't have enough space, finish to build.  */
	    pstr->cur_state = prev_st;
	    break;
	  }
      }
  pstr->valid_len = byte_idx;
  pstr->valid_raw_len = src_idx;
  return REG_NOERROR;
}

/* Skip characters until the index becomes greater than NEW_RAW_IDX.
   Return the index.  */

static int
re_string_skip_chars (pstr, new_raw_idx, last_wc)
     re_string_t *pstr;
     int new_raw_idx;
     wint_t *last_wc;
{
  mbstate_t prev_st;
  int rawbuf_idx, mbclen;
  wchar_t wc = 0;

  /* Skip the characters which are not necessary to check.  */
  for (rawbuf_idx = pstr->raw_mbs_idx + pstr->valid_raw_len;
       rawbuf_idx < new_raw_idx;)
    {
      int remain_len;
      remain_len = pstr->len - rawbuf_idx;
      prev_st = pstr->cur_state;
      mbclen = mbrtowc (&wc, (const char *) pstr->raw_mbs + rawbuf_idx,
			remain_len, &pstr->cur_state);
      if (BE (mbclen == (size_t) -2 || mbclen == (size_t) -1 || mbclen == 0, 0))
	{
	  /* We treat these cases as a singlebyte character.  */
	  mbclen = 1;
	  pstr->cur_state = prev_st;
	}
      /* Then proceed the next character.  */
      rawbuf_idx += mbclen;
    }
  *last_wc = (wint_t) wc;
  return rawbuf_idx;
}
#endif /* RE_ENABLE_I18N  */

/* Build the buffer PSTR->MBS, and apply the translation if we need.
   This function is used in case of REG_ICASE.  */

static void
build_upper_buffer (pstr)
     re_string_t *pstr;
{
  int char_idx, end_idx;
  end_idx = (pstr->bufs_len > pstr->len) ? pstr->len : pstr->bufs_len;

  for (char_idx = pstr->valid_len; char_idx < end_idx; ++char_idx)
    {
      int ch = pstr->raw_mbs[pstr->raw_mbs_idx + char_idx];
      if (BE (pstr->trans != NULL, 0))
	ch = pstr->trans[ch];
      if (islower (ch))
	pstr->mbs[char_idx] = toupper (ch);
      else
	pstr->mbs[char_idx] = ch;
    }
  pstr->valid_len = char_idx;
  pstr->valid_raw_len = char_idx;
}

/* Apply TRANS to the buffer in PSTR.  */

static void
re_string_translate_buffer (pstr)
     re_string_t *pstr;
{
  int buf_idx, end_idx;
  end_idx = (pstr->bufs_len > pstr->len) ? pstr->len : pstr->bufs_len;

  for (buf_idx = pstr->valid_len; buf_idx < end_idx; ++buf_idx)
    {
      int ch = pstr->raw_mbs[pstr->raw_mbs_idx + buf_idx];
      pstr->mbs[buf_idx] = pstr->trans[ch];
    }

  pstr->valid_len = buf_idx;
  pstr->valid_raw_len = buf_idx;
}

/* This function re-construct the buffers.
   Concretely, convert to wide character in case of pstr->mb_cur_max > 1,
   convert to upper case in case of REG_ICASE, apply translation.  */

static reg_errcode_t
re_string_reconstruct (pstr, idx, eflags, newline)
     re_string_t *pstr;
     int idx, eflags, newline;
{
  int offset = idx - pstr->raw_mbs_idx;
  if (offset < 0)
    {
      /* Reset buffer.  */
#ifdef RE_ENABLE_I18N
      if (pstr->mb_cur_max > 1)
	memset (&pstr->cur_state, '\0', sizeof (mbstate_t));
#endif /* RE_ENABLE_I18N */
      pstr->len = pstr->raw_len;
      pstr->stop = pstr->raw_stop;
      pstr->valid_len = 0;
      pstr->raw_mbs_idx = 0;
      pstr->valid_raw_len = 0;
      pstr->offsets_needed = 0;
      pstr->tip_context = ((eflags & REG_NOTBOL) ? CONTEXT_BEGBUF
			   : CONTEXT_NEWLINE | CONTEXT_BEGBUF);
      if (!pstr->mbs_allocated)
	pstr->mbs = (unsigned char *) pstr->raw_mbs;
      offset = idx;
    }

  if (offset != 0)
    {
      /* Are the characters which are already checked remain?  */
      if (offset < pstr->valid_raw_len
#ifdef RE_ENABLE_I18N
	  /* Handling this would enlarge the code too much.
	     Accept a slowdown in that case.  */
	  && pstr->offsets_needed == 0
#endif
	 )
	{
	  /* Yes, move them to the front of the buffer.  */
	  pstr->tip_context = re_string_context_at (pstr, offset - 1, eflags,
						    newline);
#ifdef RE_ENABLE_I18N
	  if (pstr->mb_cur_max > 1)
	    memmove (pstr->wcs, pstr->wcs + offset,
		     (pstr->valid_len - offset) * sizeof (wint_t));
#endif /* RE_ENABLE_I18N */
	  if (pstr->mbs_allocated)
	    memmove (pstr->mbs, pstr->mbs + offset,
		     pstr->valid_len - offset);
	  pstr->valid_len -= offset;
	  pstr->valid_raw_len -= offset;
#if DEBUG
	  assert (pstr->valid_len > 0);
#endif
	}
      else
	{
	  /* No, skip all characters until IDX.  */
#ifdef RE_ENABLE_I18N
	  if (BE (pstr->offsets_needed, 0))
	    {
	      pstr->len = pstr->raw_len - idx + offset;
	      pstr->stop = pstr->raw_stop - idx + offset;
	      pstr->offsets_needed = 0;
	    }
#endif
	  pstr->valid_len = 0;
	  pstr->valid_raw_len = 0;
#ifdef RE_ENABLE_I18N
	  if (pstr->mb_cur_max > 1)
	    {
	      int wcs_idx;
	      wint_t wc = WEOF;

#ifdef _LIBC
	      if (pstr->is_utf8)
		{
		  const unsigned char *raw, *p, *q, *end;

		  /* Special case UTF-8.  Multi-byte chars start with any
		     byte other than 0x80 - 0xbf.  */
		  raw = pstr->raw_mbs + pstr->raw_mbs_idx;
		  end = raw + (offset - pstr->mb_cur_max);
		  for (p = raw + offset - 1; p >= end; --p)
		    if ((*p & 0xc0) != 0x80)
		      {
			mbstate_t cur_state;
			wchar_t wc2;
			int mlen = raw + pstr->len - p;
			unsigned char buf[6];

			q = p;
			if (BE (pstr->trans != NULL, 0))
			  {
			    int i = mlen < 6 ? mlen : 6;
			    while (--i >= 0)
			      buf[i] = pstr->trans[p[i]];
			    q = buf;
			  }
			/* XXX Don't use mbrtowc, we know which conversion
			   to use (UTF-8 -> UCS4).  */
			memset (&cur_state, 0, sizeof (cur_state));
			mlen = mbrtowc (&wc2, p, mlen, &cur_state)
			       - (raw + offset - p);
			if (mlen >= 0)
			  {
			    memset (&pstr->cur_state, '\0',
				    sizeof (mbstate_t));
			    pstr->valid_len = mlen;
			    wc = wc2;
			  }
			break;
		      }
		}
#endif
	      if (wc == WEOF)
		pstr->valid_len = re_string_skip_chars (pstr, idx, &wc) - idx;
	      if (BE (pstr->valid_len, 0))
		{
		  for (wcs_idx = 0; wcs_idx < pstr->valid_len; ++wcs_idx)
		    pstr->wcs[wcs_idx] = WEOF;
		  if (pstr->mbs_allocated)
		    memset (pstr->mbs, 255, pstr->valid_len);
		}
	      pstr->valid_raw_len = pstr->valid_len;
	      pstr->tip_context = (IS_WIDE_WORD_CHAR (wc) ? CONTEXT_WORD
				   : ((newline && IS_WIDE_NEWLINE (wc))
				      ? CONTEXT_NEWLINE : 0));
	    }
	  else
#endif /* RE_ENABLE_I18N */
	    {
	      int c = pstr->raw_mbs[pstr->raw_mbs_idx + offset - 1];
	      if (pstr->trans)
		c = pstr->trans[c];
	      pstr->tip_context = (IS_WORD_CHAR (c) ? CONTEXT_WORD
				   : ((newline && IS_NEWLINE (c))
				      ? CONTEXT_NEWLINE : 0));
	    }
	}
      if (!pstr->mbs_allocated)
	pstr->mbs += offset;
    }
  pstr->raw_mbs_idx = idx;
  pstr->len -= offset;
  pstr->stop -= offset;

  /* Then build the buffers.  */
#ifdef RE_ENABLE_I18N
  if (pstr->mb_cur_max > 1)
    {
      if (pstr->icase)
	{
	  int ret = build_wcs_upper_buffer (pstr);
	  if (BE (ret != REG_NOERROR, 0))
	    return ret;
	}
      else
	build_wcs_buffer (pstr);
    }
  else
#endif /* RE_ENABLE_I18N */
    {
      if (pstr->icase)
	build_upper_buffer (pstr);
      else if (pstr->trans != NULL)
	re_string_translate_buffer (pstr);
      else
	pstr->valid_len = pstr->len;
    }
  pstr->cur_idx = 0;

  return REG_NOERROR;
}

static unsigned char
re_string_peek_byte_case (const re_string_t *pstr,
			  int idx)
{
  int ch, off;

  /* Handle the common (easiest) cases first.  */
  if (BE (!pstr->icase, 1))
    return re_string_peek_byte (pstr, idx);

#ifdef RE_ENABLE_I18N
  if (pstr->mb_cur_max > 1
      && ! re_string_is_single_byte_char (pstr, pstr->cur_idx + idx))
    return re_string_peek_byte (pstr, idx);
#endif

  off = pstr->cur_idx + idx;
#ifdef RE_ENABLE_I18N
  if (pstr->offsets_needed)
    off = pstr->offsets[off];
#endif

  ch = pstr->raw_mbs[pstr->raw_mbs_idx + off];
  if (pstr->trans)
    ch = pstr->trans[ch];

#ifdef RE_ENABLE_I18N
  /* Ensure that e.g. for tr_TR.UTF-8 BACKSLASH DOTLESS SMALL LETTER I
     this function returns CAPITAL LETTER I instead of first byte of
     DOTLESS SMALL LETTER I.  The latter would confuse the parser,
     since peek_byte_case doesn't advance cur_idx in any way.  */
  if (pstr->offsets_needed && !isascii (ch))
    return re_string_peek_byte (pstr, idx);
#endif

  return ch;
}

static unsigned char
re_string_fetch_byte_case (re_string_t *pstr)
{
  int ch;
#ifdef RE_ENABLE_I18N
  int off;
#endif

  if (BE (!pstr->icase, 1))
    return re_string_fetch_byte (pstr);

#ifdef RE_ENABLE_I18N
  if (pstr->offsets_needed)
    {
      /* For tr_TR.UTF-8 [[:islower:]] there is
	 [[: CAPITAL LETTER I WITH DOT lower:]] in mbs.  Skip
	 in that case the whole multi-byte character and return
	 the original letter.  On the other side, with
	 [[: DOTLESS SMALL LETTER I return [[:I, as doing
	 anything else would complicate things too much.  */

      if (!re_string_first_byte (pstr, pstr->cur_idx))
	return re_string_fetch_byte (pstr);

      off = pstr->offsets[pstr->cur_idx];
      ch = pstr->raw_mbs[pstr->raw_mbs_idx + off];
      if (pstr->trans)
	ch = pstr->trans[ch];

      if (! isascii (ch))
	return re_string_fetch_byte (pstr);

      re_string_skip_bytes (pstr,
			    re_string_char_size_at (pstr, pstr->cur_idx));
      return ch;
    }
#endif

  ch = pstr->raw_mbs[pstr->raw_mbs_idx + pstr->cur_idx++];
  if (pstr->trans)
    ch = pstr->trans[ch];
  return ch;
}

static void
re_string_destruct (pstr)
     re_string_t *pstr;
{
#ifdef RE_ENABLE_I18N
  re_free (pstr->wcs);
  re_free (pstr->offsets);
#endif /* RE_ENABLE_I18N  */
  if (pstr->mbs_allocated)
    re_free (pstr->mbs);
}

/* Return the context at IDX in INPUT.  */

static unsigned int
re_string_context_at (input, idx, eflags, newline_anchor)
     const re_string_t *input;
     int idx, eflags, newline_anchor;
{
  int c;
  if (idx < 0 || idx == input->len)
    {
      if (idx < 0)
	/* In this case, we use the value stored in input->tip_context,
	   since we can't know the character in input->mbs[-1] here.  */
	return input->tip_context;
      else /* (idx == input->len) */
	return ((eflags & REG_NOTEOL) ? CONTEXT_ENDBUF
		: CONTEXT_NEWLINE | CONTEXT_ENDBUF);
    }
#ifdef RE_ENABLE_I18N
  if (input->mb_cur_max > 1)
    {
      wint_t wc;
      int wc_idx = idx;
      while(input->wcs[wc_idx] == WEOF)
	{
#ifdef DEBUG
	  /* It must not happen.  */
	  assert (wc_idx >= 0);
#endif
	  --wc_idx;
	  if (wc_idx < 0)
	    return input->tip_context;
	}
      wc = input->wcs[wc_idx];
      if (IS_WIDE_WORD_CHAR (wc))
	return CONTEXT_WORD;
      return (newline_anchor && IS_WIDE_NEWLINE (wc)) ? CONTEXT_NEWLINE : 0;
    }
  else
#endif
    {
      c = re_string_byte_at (input, idx);
      if (IS_WORD_CHAR (c))
	return CONTEXT_WORD;
      return (newline_anchor && IS_NEWLINE (c)) ? CONTEXT_NEWLINE : 0;
    }
}

/* Functions for set operation.  */

static reg_errcode_t
re_node_set_alloc (set, size)
     re_node_set *set;
     int size;
{
  set->alloc = size;
  set->nelem = 0;
  set->elems = re_malloc (int, size);
  if (BE (set->elems == NULL, 0))
    return REG_ESPACE;
  return REG_NOERROR;
}

static reg_errcode_t
re_node_set_init_1 (set, elem)
     re_node_set *set;
     int elem;
{
  set->alloc = 1;
  set->nelem = 1;
  set->elems = re_malloc (int, 1);
  if (BE (set->elems == NULL, 0))
    {
      set->alloc = set->nelem = 0;
      return REG_ESPACE;
    }
  set->elems[0] = elem;
  return REG_NOERROR;
}

static reg_errcode_t
re_node_set_init_2 (set, elem1, elem2)
     re_node_set *set;
     int elem1, elem2;
{
  set->alloc = 2;
  set->elems = re_malloc (int, 2);
  if (BE (set->elems == NULL, 0))
    return REG_ESPACE;
  if (elem1 == elem2)
    {
      set->nelem = 1;
      set->elems[0] = elem1;
    }
  else
    {
      set->nelem = 2;
      if (elem1 < elem2)
	{
	  set->elems[0] = elem1;
	  set->elems[1] = elem2;
	}
      else
	{
	  set->elems[0] = elem2;
	  set->elems[1] = elem1;
	}
    }
  return REG_NOERROR;
}

static reg_errcode_t
re_node_set_init_copy (dest, src)
     re_node_set *dest;
     const re_node_set *src;
{
  dest->nelem = src->nelem;
  if (src->nelem > 0)
    {
      dest->alloc = dest->nelem;
      dest->elems = re_malloc (int, dest->alloc);
      if (BE (dest->elems == NULL, 0))
	{
	  dest->alloc = dest->nelem = 0;
	  return REG_ESPACE;
	}
      memcpy (dest->elems, src->elems, src->nelem * sizeof (int));
    }
  else
    re_node_set_init_empty (dest);
  return REG_NOERROR;
}

/* Calculate the intersection of the sets SRC1 and SRC2. And merge it to
   DEST. Return value indicate the error code or REG_NOERROR if succeeded.
   Note: We assume dest->elems is NULL, when dest->alloc is 0.  */

static reg_errcode_t
re_node_set_add_intersect (dest, src1, src2)
     re_node_set *dest;
     const re_node_set *src1, *src2;
{
  int i1, i2, id;
  if (src1->nelem > 0 && src2->nelem > 0)
    {
      if (src1->nelem + src2->nelem + dest->nelem > dest->alloc)
	{
	  int new_alloc = src1->nelem + src2->nelem + dest->nelem;
	  int *new_elems = re_realloc (dest->elems, int, new_alloc);
	  if (BE (new_elems == NULL, 0))
	    return REG_ESPACE;
	  dest->elems = new_elems;
	  dest->alloc = new_alloc;
	}
    }
  else
    return REG_NOERROR;

  for (i1 = i2 = id = 0 ; i1 < src1->nelem && i2 < src2->nelem ;)
    {
      if (src1->elems[i1] > src2->elems[i2])
	{
	  ++i2;
	  continue;
	}
      if (src1->elems[i1] == src2->elems[i2])
	{
	  while (id < dest->nelem && dest->elems[id] < src2->elems[i2])
	    ++id;
	  if (id < dest->nelem && dest->elems[id] == src2->elems[i2])
	    ++id;
	  else
	    {
	      memmove (dest->elems + id + 1, dest->elems + id,
		       sizeof (int) * (dest->nelem - id));
	      dest->elems[id++] = src2->elems[i2++];
	      ++dest->nelem;
	    }
	}
      ++i1;
    }
  return REG_NOERROR;
}

/* Calculate the union set of the sets SRC1 and SRC2. And store it to
   DEST. Return value indicate the error code or REG_NOERROR if succeeded.  */

static reg_errcode_t
re_node_set_init_union (dest, src1, src2)
     re_node_set *dest;
     const re_node_set *src1, *src2;
{
  int i1, i2, id;
  if (src1 != NULL && src1->nelem > 0 && src2 != NULL && src2->nelem > 0)
    {
      dest->alloc = src1->nelem + src2->nelem;
      dest->elems = re_malloc (int, dest->alloc);
      if (BE (dest->elems == NULL, 0))
	return REG_ESPACE;
    }
  else
    {
      if (src1 != NULL && src1->nelem > 0)
	return re_node_set_init_copy (dest, src1);
      else if (src2 != NULL && src2->nelem > 0)
	return re_node_set_init_copy (dest, src2);
      else
	re_node_set_init_empty (dest);
      return REG_NOERROR;
    }
  for (i1 = i2 = id = 0 ; i1 < src1->nelem && i2 < src2->nelem ;)
    {
      if (src1->elems[i1] > src2->elems[i2])
	{
	  dest->elems[id++] = src2->elems[i2++];
	  continue;
	}
      if (src1->elems[i1] == src2->elems[i2])
	++i2;
      dest->elems[id++] = src1->elems[i1++];
    }
  if (i1 < src1->nelem)
    {
      memcpy (dest->elems + id, src1->elems + i1,
	     (src1->nelem - i1) * sizeof (int));
      id += src1->nelem - i1;
    }
  else if (i2 < src2->nelem)
    {
      memcpy (dest->elems + id, src2->elems + i2,
	     (src2->nelem - i2) * sizeof (int));
      id += src2->nelem - i2;
    }
  dest->nelem = id;
  return REG_NOERROR;
}

/* Calculate the union set of the sets DEST and SRC. And store it to
   DEST. Return value indicate the error code or REG_NOERROR if succeeded.  */

static reg_errcode_t
re_node_set_merge (dest, src)
     re_node_set *dest;
     const re_node_set *src;
{
  int si, di;
  if (src == NULL || src->nelem == 0)
    return REG_NOERROR;
  if (dest->alloc < src->nelem + dest->nelem)
    {
      int new_alloc = 2 * (src->nelem + dest->alloc);
      int *new_buffer = re_realloc (dest->elems, int, new_alloc);
      if (BE (new_buffer == NULL, 0))
	return REG_ESPACE;
      dest->elems = new_buffer;
      dest->alloc = new_alloc;
    }

  for (si = 0, di = 0 ; si < src->nelem && di < dest->nelem ;)
    {
      int cp_from, ncp, mid, right, src_elem = src->elems[si];
      /* Binary search the spot we will add the new element.  */
      right = dest->nelem;
      while (di < right)
	{
	  mid = (di + right) / 2;
	  if (dest->elems[mid] < src_elem)
	    di = mid + 1;
	  else
	    right = mid;
	}
      if (di >= dest->nelem)
	break;

      if (dest->elems[di] == src_elem)
	{
	  /* Skip since, DEST already has the element.  */
	  ++di;
	  ++si;
	  continue;
	}

      /* Skip the src elements which are less than dest->elems[di].  */
      cp_from = si;
      while (si < src->nelem && src->elems[si] < dest->elems[di])
	++si;
      /* Copy these src elements.  */
      ncp = si - cp_from;
      memmove (dest->elems + di + ncp, dest->elems + di,
	       sizeof (int) * (dest->nelem - di));
      memcpy (dest->elems + di, src->elems + cp_from,
	      sizeof (int) * ncp);
      /* Update counters.  */
      di += ncp;
      dest->nelem += ncp;
    }

  /* Copy remaining src elements.  */
  if (si < src->nelem)
    {
      memcpy (dest->elems + di, src->elems + si,
	      sizeof (int) * (src->nelem - si));
      dest->nelem += src->nelem - si;
    }
  return REG_NOERROR;
}

/* Insert the new element ELEM to the re_node_set* SET.
   return 0 if SET already has ELEM,
   return -1 if an error is occured, return 1 otherwise.  */

static int
re_node_set_insert (set, elem)
     re_node_set *set;
     int elem;
{
  int idx, right, mid;
  /* In case of the set is empty.  */
  if (set->elems == NULL || set->alloc == 0)
    {
      if (BE (re_node_set_init_1 (set, elem) == REG_NOERROR, 1))
	return 1;
      else
	return -1;
    }

  /* Binary search the spot we will add the new element.  */
  idx = 0;
  right = set->nelem;
  while (idx < right)
    {
      mid = (idx + right) / 2;
      if (set->elems[mid] < elem)
	idx = mid + 1;
      else
	right = mid;
    }

  /* Realloc if we need.  */
  if (set->alloc < set->nelem + 1)
    {
      int *new_array;
      set->alloc = set->alloc * 2;
      new_array = re_malloc (int, set->alloc);
      if (BE (new_array == NULL, 0))
	return -1;
      /* Copy the elements they are followed by the new element.  */
      if (idx > 0)
	memcpy (new_array, set->elems, sizeof (int) * (idx));
      /* Copy the elements which follows the new element.  */
      if (set->nelem - idx > 0)
	memcpy (new_array + idx + 1, set->elems + idx,
		sizeof (int) * (set->nelem - idx));
      re_free (set->elems);
      set->elems = new_array;
    }
  else
    {
      /* Move the elements which follows the new element.  */
      if (set->nelem - idx > 0)
	memmove (set->elems + idx + 1, set->elems + idx,
		 sizeof (int) * (set->nelem - idx));
    }
  /* Insert the new element.  */
  set->elems[idx] = elem;
  ++set->nelem;
  return 1;
}

/* Compare two node sets SET1 and SET2.
   return 1 if SET1 and SET2 are equivalent, retrun 0 otherwise.  */

static int
re_node_set_compare (set1, set2)
     const re_node_set *set1, *set2;
{
  int i;
  if (set1 == NULL || set2 == NULL || set1->nelem != set2->nelem)
    return 0;
  for (i = 0 ; i < set1->nelem ; i++)
    if (set1->elems[i] != set2->elems[i])
      return 0;
  return 1;
}

/* Return (idx + 1) if SET contains the element ELEM, return 0 otherwise.  */

static int
re_node_set_contains (set, elem)
     const re_node_set *set;
     int elem;
{
  int idx, right, mid;
  if (set->nelem <= 0)
    return 0;

  /* Binary search the element.  */
  idx = 0;
  right = set->nelem - 1;
  while (idx < right)
    {
      mid = (idx + right) / 2;
      if (set->elems[mid] < elem)
	idx = mid + 1;
      else
	right = mid;
    }
  return set->elems[idx] == elem ? idx + 1 : 0;
}

static void
re_node_set_remove_at (set, idx)
     re_node_set *set;
     int idx;
{
  if (idx < 0 || idx >= set->nelem)
    return;
  if (idx < set->nelem - 1)
    memmove (set->elems + idx, set->elems + idx + 1,
	     sizeof (int) * (set->nelem - idx - 1));
  --set->nelem;
}


/* Add the token TOKEN to dfa->nodes, and return the index of the token.
   Or return -1, if an error will be occured.  */

static int
re_dfa_add_node (dfa, token, mode)
     re_dfa_t *dfa;
     re_token_t token;
     int mode;
{
  if (BE (dfa->nodes_len >= dfa->nodes_alloc, 0))
    {
      int new_nodes_alloc = dfa->nodes_alloc * 2;
      re_token_t *new_array = re_realloc (dfa->nodes, re_token_t,
					  new_nodes_alloc);
      if (BE (new_array == NULL, 0))
	return -1;
      dfa->nodes = new_array;
      if (mode)
	{
	  int *new_nexts, *new_indices;
	  re_node_set *new_edests, *new_eclosures, *new_inveclosures;

	  new_nexts = re_realloc (dfa->nexts, int, new_nodes_alloc);
	  new_indices = re_realloc (dfa->org_indices, int, new_nodes_alloc);
	  new_edests = re_realloc (dfa->edests, re_node_set, new_nodes_alloc);
	  new_eclosures = re_realloc (dfa->eclosures, re_node_set,
				      new_nodes_alloc);
	  new_inveclosures = re_realloc (dfa->inveclosures, re_node_set,
					 new_nodes_alloc);
	  if (BE (new_nexts == NULL || new_indices == NULL
		  || new_edests == NULL || new_eclosures == NULL
		  || new_inveclosures == NULL, 0))
	    return -1;
	  dfa->nexts = new_nexts;
	  dfa->org_indices = new_indices;
	  dfa->edests = new_edests;
	  dfa->eclosures = new_eclosures;
	  dfa->inveclosures = new_inveclosures;
	}
      dfa->nodes_alloc = new_nodes_alloc;
    }
  dfa->nodes[dfa->nodes_len] = token;
  dfa->nodes[dfa->nodes_len].duplicated = 0;
  dfa->nodes[dfa->nodes_len].constraint = 0;
  return dfa->nodes_len++;
}

static unsigned int R_INLINE
calc_state_hash (nodes, context)
     const re_node_set *nodes;
     unsigned int context;
{
  unsigned int hash = nodes->nelem + context;
  int i;
  for (i = 0 ; i < nodes->nelem ; i++)
    hash += nodes->elems[i];
  return hash;
}

/* Search for the state whose node_set is equivalent to NODES.
   Return the pointer to the state, if we found it in the DFA.
   Otherwise create the new one and return it.  In case of an error
   return NULL and set the error code in ERR.
   Note: - We assume NULL as the invalid state, then it is possible that
	   return value is NULL and ERR is REG_NOERROR.
	 - We never return non-NULL value in case of any errors, it is for
	   optimization.  */

static re_dfastate_t*
re_acquire_state (err, dfa, nodes)
     reg_errcode_t *err;
     re_dfa_t *dfa;
     const re_node_set *nodes;
{
  unsigned int hash;
  re_dfastate_t *new_state;
  struct re_state_table_entry *spot;
  int i;
  if (BE (nodes->nelem == 0, 0))
    {
      *err = REG_NOERROR;
      return NULL;
    }
  hash = calc_state_hash (nodes, 0);
  spot = dfa->state_table + (hash & dfa->state_hash_mask);

  for (i = 0 ; i < spot->num ; i++)
    {
      re_dfastate_t *state = spot->array[i];
      if (hash != state->hash)
	continue;
      if (re_node_set_compare (&state->nodes, nodes))
	return state;
    }

  /* There are no appropriate state in the dfa, create the new one.  */
  new_state = create_ci_newstate (dfa, nodes, hash);
  if (BE (new_state != NULL, 1))
    return new_state;
  else
    {
      *err = REG_ESPACE;
      return NULL;
    }
}

/* Search for the state whose node_set is equivalent to NODES and
   whose context is equivalent to CONTEXT.
   Return the pointer to the state, if we found it in the DFA.
   Otherwise create the new one and return it.  In case of an error
   return NULL and set the error code in ERR.
   Note: - We assume NULL as the invalid state, then it is possible that
	   return value is NULL and ERR is REG_NOERROR.
	 - We never return non-NULL value in case of any errors, it is for
	   optimization.  */

static re_dfastate_t*
re_acquire_state_context (err, dfa, nodes, context)
     reg_errcode_t *err;
     re_dfa_t *dfa;
     const re_node_set *nodes;
     unsigned int context;
{
  unsigned int hash;
  re_dfastate_t *new_state;
  struct re_state_table_entry *spot;
  int i;
  if (nodes->nelem == 0)
    {
      *err = REG_NOERROR;
      return NULL;
    }
  hash = calc_state_hash (nodes, context);
  spot = dfa->state_table + (hash & dfa->state_hash_mask);

  for (i = 0 ; i < spot->num ; i++)
    {
      re_dfastate_t *state = spot->array[i];
      if (hash != state->hash)
	continue;
      if (re_node_set_compare (state->entrance_nodes, nodes)
	  && state->context == context)
	return state;
    }
  /* There are no appropriate state in `dfa', create the new one.  */
  new_state = create_cd_newstate (dfa, nodes, context, hash);
  if (BE (new_state != NULL, 1))
    return new_state;
  else
    {
      *err = REG_ESPACE;
      return NULL;
    }
}

/* Allocate memory for DFA state and initialize common properties.
   Return the new state if succeeded, otherwise return NULL.  */

static re_dfastate_t *
create_newstate_common (dfa, nodes, hash)
     re_dfa_t *dfa;
     const re_node_set *nodes;
     unsigned int hash;
{
  re_dfastate_t *newstate;
  reg_errcode_t err;
  newstate = (re_dfastate_t *) calloc (sizeof (re_dfastate_t), 1);
  if (BE (newstate == NULL, 0))
    return NULL;
  err = re_node_set_init_copy (&newstate->nodes, nodes);
  if (BE (err != REG_NOERROR, 0))
    {
      re_free (newstate);
      return NULL;
    }
  newstate->trtable = NULL;
  newstate->hash = hash;
  return newstate;
}

/* Store the new state NEWSTATE whose hash value is HASH in appropriate
   position.  Return value indicate the error code if failed.  */

static reg_errcode_t
register_state (dfa, newstate, hash)
     re_dfa_t *dfa;
     re_dfastate_t *newstate;
     unsigned int hash;
{
  struct re_state_table_entry *spot;
  spot = dfa->state_table + (hash & dfa->state_hash_mask);

  if (BE (spot->alloc <= spot->num, 0))
    {
      int new_alloc = 2 * spot->num + 2;
      re_dfastate_t **new_array = re_realloc (spot->array, re_dfastate_t *,
					      new_alloc);
      if (BE (new_array == NULL, 0))
	return REG_ESPACE;
      spot->array = new_array;
      spot->alloc = new_alloc;
    }
  spot->array[spot->num++] = newstate;
  return REG_NOERROR;
}

/* Create the new state which is independ of contexts.
   Return the new state if succeeded, otherwise return NULL.  */

static re_dfastate_t *
create_ci_newstate (dfa, nodes, hash)
     re_dfa_t *dfa;
     const re_node_set *nodes;
     unsigned int hash;
{
  int i;
  reg_errcode_t err;
  re_dfastate_t *newstate;
  newstate = create_newstate_common (dfa, nodes, hash);
  if (BE (newstate == NULL, 0))
    return NULL;
  newstate->entrance_nodes = &newstate->nodes;

  for (i = 0 ; i < nodes->nelem ; i++)
    {
      re_token_t *node = dfa->nodes + nodes->elems[i];
      re_token_type_t type = node->type;
      if (type == CHARACTER && !node->constraint)
	continue;

      /* If the state has the halt node, the state is a halt state.  */
      else if (type == END_OF_RE)
	newstate->halt = 1;
#ifdef RE_ENABLE_I18N
      else if (type == COMPLEX_BRACKET
	       || type == OP_UTF8_PERIOD
	       || (type == OP_PERIOD && dfa->mb_cur_max > 1))
	newstate->accept_mb = 1;
#endif /* RE_ENABLE_I18N */
      else if (type == OP_BACK_REF)
	newstate->has_backref = 1;
      else if (type == ANCHOR || node->constraint)
	newstate->has_constraint = 1;
    }
  err = register_state (dfa, newstate, hash);
  if (BE (err != REG_NOERROR, 0))
    {
      free_state (newstate);
      newstate = NULL;
    }
  return newstate;
}

/* Create the new state which is depend on the context CONTEXT.
   Return the new state if succeeded, otherwise return NULL.  */

static re_dfastate_t *
create_cd_newstate (dfa, nodes, context, hash)
     re_dfa_t *dfa;
     const re_node_set *nodes;
     unsigned int context, hash;
{
  int i, nctx_nodes = 0;
  reg_errcode_t err;
  re_dfastate_t *newstate;

  newstate = create_newstate_common (dfa, nodes, hash);
  if (BE (newstate == NULL, 0))
    return NULL;
  newstate->context = context;
  newstate->entrance_nodes = &newstate->nodes;

  for (i = 0 ; i < nodes->nelem ; i++)
    {
      unsigned int constraint = 0;
      re_token_t *node = dfa->nodes + nodes->elems[i];
      re_token_type_t type = node->type;
      if (node->constraint)
	constraint = node->constraint;

      if (type == CHARACTER && !constraint)
	continue;
      /* If the state has the halt node, the state is a halt state.  */
      else if (type == END_OF_RE)
	newstate->halt = 1;
#ifdef RE_ENABLE_I18N
      else if (type == COMPLEX_BRACKET
	       || type == OP_UTF8_PERIOD
	       || (type == OP_PERIOD && dfa->mb_cur_max > 1))
	newstate->accept_mb = 1;
#endif /* RE_ENABLE_I18N */
      else if (type == OP_BACK_REF)
	newstate->has_backref = 1;
      else if (type == ANCHOR)
	constraint = node->opr.ctx_type;

      if (constraint)
	{
	  if (newstate->entrance_nodes == &newstate->nodes)
	    {
	      newstate->entrance_nodes = re_malloc (re_node_set, 1);
	      if (BE (newstate->entrance_nodes == NULL, 0))
		{
		  free_state (newstate);
		  return NULL;
		}
	      re_node_set_init_copy (newstate->entrance_nodes, nodes);
	      nctx_nodes = 0;
	      newstate->has_constraint = 1;
	    }

	  if (NOT_SATISFY_PREV_CONSTRAINT (constraint,context))
	    {
	      re_node_set_remove_at (&newstate->nodes, i - nctx_nodes);
	      ++nctx_nodes;
	    }
	}
    }
  err = register_state (dfa, newstate, hash);
  if (BE (err != REG_NOERROR, 0))
    {
      free_state (newstate);
      newstate = NULL;
    }
  return  newstate;
}

static void
free_state (state)
     re_dfastate_t *state;
{
  if (state->entrance_nodes != &state->nodes)
    {
      re_node_set_free (state->entrance_nodes);
      re_free (state->entrance_nodes);
    }
  re_node_set_free (&state->nodes);
  re_free (state->trtable);
  re_free (state);
}

static reg_errcode_t re_compile_internal (regex_t *preg, const char * pattern,
					  int length, reg_syntax_t syntax);
static void re_compile_fastmap_iter (regex_t *bufp,
				     const re_dfastate_t *init_state,
				     char *fastmap);
static reg_errcode_t init_dfa (re_dfa_t *dfa, int pat_len);
static reg_errcode_t init_word_char (re_dfa_t *dfa);
#ifdef RE_ENABLE_I18N
static void free_charset (re_charset_t *cset);
#endif /* RE_ENABLE_I18N */
static void free_workarea_compile (regex_t *preg);
static reg_errcode_t create_initial_state (re_dfa_t *dfa);
#ifdef RE_ENABLE_I18N
static void optimize_utf8 (re_dfa_t *dfa);
#endif
static reg_errcode_t analyze (re_dfa_t *dfa);
static reg_errcode_t analyze_tree (re_dfa_t *dfa, bin_tree_t *node);
static void calc_first (re_dfa_t *dfa, bin_tree_t *node);
static void calc_next (re_dfa_t *dfa, bin_tree_t *node);
static void calc_epsdest (re_dfa_t *dfa, bin_tree_t *node);
static reg_errcode_t duplicate_node_closure (re_dfa_t *dfa, int top_org_node,
					     int top_clone_node, int root_node,
					     unsigned int constraint);
static reg_errcode_t duplicate_node (int *new_idx, re_dfa_t *dfa, int org_idx,
				     unsigned int constraint);
static int search_duplicated_node (re_dfa_t *dfa, int org_node,
				   unsigned int constraint);
static reg_errcode_t calc_eclosure (re_dfa_t *dfa);
static reg_errcode_t calc_eclosure_iter (re_node_set *new_set, re_dfa_t *dfa,
					 int node, int root);
static void calc_inveclosure (re_dfa_t *dfa);
static int fetch_number (re_string_t *input, re_token_t *token,
			 reg_syntax_t syntax);
static void fetch_token (re_token_t *result, re_string_t *input,
			 reg_syntax_t syntax);
static int peek_token (re_token_t *token, re_string_t *input,
			reg_syntax_t syntax);
static int peek_token_bracket (re_token_t *token, re_string_t *input,
			       reg_syntax_t syntax);
static bin_tree_t *parse (re_string_t *regexp, regex_t *preg,
			  reg_syntax_t syntax, reg_errcode_t *err);
static bin_tree_t *parse_reg_exp (re_string_t *regexp, regex_t *preg,
				  re_token_t *token, reg_syntax_t syntax,
				  int nest, reg_errcode_t *err);
static bin_tree_t *parse_branch (re_string_t *regexp, regex_t *preg,
				 re_token_t *token, reg_syntax_t syntax,
				 int nest, reg_errcode_t *err);
static bin_tree_t *parse_expression (re_string_t *regexp, regex_t *preg,
				     re_token_t *token, reg_syntax_t syntax,
				     int nest, reg_errcode_t *err);
static bin_tree_t *parse_sub_exp (re_string_t *regexp, regex_t *preg,
				  re_token_t *token, reg_syntax_t syntax,
				  int nest, reg_errcode_t *err);
static bin_tree_t *parse_dup_op (bin_tree_t *dup_elem, re_string_t *regexp,
				 re_dfa_t *dfa, re_token_t *token,
				 reg_syntax_t syntax, reg_errcode_t *err);
static bin_tree_t *parse_bracket_exp (re_string_t *regexp, re_dfa_t *dfa,
				      re_token_t *token, reg_syntax_t syntax,
				      reg_errcode_t *err);
static reg_errcode_t parse_bracket_element (bracket_elem_t *elem,
					    re_string_t *regexp,
					    re_token_t *token, int token_len,
					    re_dfa_t *dfa,
					    reg_syntax_t syntax,
					    int accept_hyphen);
static reg_errcode_t parse_bracket_symbol (bracket_elem_t *elem,
					  re_string_t *regexp,
					  re_token_t *token);
#ifndef _LIBC
# ifdef RE_ENABLE_I18N
static reg_errcode_t build_range_exp (re_bitset_ptr_t sbcset,
				      re_charset_t *mbcset, int *range_alloc,
				      bracket_elem_t *start_elem,
				      bracket_elem_t *end_elem);
static reg_errcode_t build_collating_symbol (re_bitset_ptr_t sbcset,
					     re_charset_t *mbcset,
					     int *coll_sym_alloc,
					     const unsigned char *name);
# else /* not RE_ENABLE_I18N */
static reg_errcode_t build_range_exp (re_bitset_ptr_t sbcset,
				      bracket_elem_t *start_elem,
				      bracket_elem_t *end_elem);
static reg_errcode_t build_collating_symbol (re_bitset_ptr_t sbcset,
					     const unsigned char *name);
# endif /* not RE_ENABLE_I18N */
#endif /* not _LIBC */
#ifdef RE_ENABLE_I18N
static reg_errcode_t build_equiv_class (re_bitset_ptr_t sbcset,
					re_charset_t *mbcset,
					int *equiv_class_alloc,
					const unsigned char *name);
static reg_errcode_t build_charclass (RE_TRANSLATE_TYPE trans,
				      re_bitset_ptr_t sbcset,
				      re_charset_t *mbcset,
				      int *char_class_alloc,
				      const unsigned char *class_name,
				      reg_syntax_t syntax);
#else  /* not RE_ENABLE_I18N */
static reg_errcode_t build_equiv_class (re_bitset_ptr_t sbcset,
					const unsigned char *name);
static reg_errcode_t build_charclass (RE_TRANSLATE_TYPE trans,
				      re_bitset_ptr_t sbcset,
				      const unsigned char *class_name,
				      reg_syntax_t syntax);
#endif /* not RE_ENABLE_I18N */
static bin_tree_t *build_charclass_op (re_dfa_t *dfa, RE_TRANSLATE_TYPE trans,
				       const unsigned char *class_name,
				       const unsigned char *extra, int not,
				       reg_errcode_t *err);
static bin_tree_t *create_tree (re_dfa_t *dfa,
				bin_tree_t *left, bin_tree_t *right,
				re_token_type_t type, int index);
static bin_tree_t *re_dfa_add_tree_node (re_dfa_t *dfa,
					 bin_tree_t *left, bin_tree_t *right,
					 const re_token_t *token)
  __attribute ((noinline));
static bin_tree_t *duplicate_tree (const bin_tree_t *src, re_dfa_t *dfa);

/* This table gives an error message for each of the error codes listed
   in regex.h.  Obviously the order here has to be same as there.
   POSIX doesn't require that we do anything for REG_NOERROR,
   but why not be nice?  */

static const char __re_error_msgid[] attribute_hidden =
  {
#define REG_NOERROR_IDX	0
    gettext_noop ("Success")	/* REG_NOERROR */
    "\0"
#define REG_NOMATCH_IDX (REG_NOERROR_IDX + sizeof "Success")
    gettext_noop ("No match")	/* REG_NOMATCH */
    "\0"
#define REG_BADPAT_IDX	(REG_NOMATCH_IDX + sizeof "No match")
    gettext_noop ("Invalid regular expression") /* REG_BADPAT */
    "\0"
#define REG_ECOLLATE_IDX (REG_BADPAT_IDX + sizeof "Invalid regular expression")
    gettext_noop ("Invalid collation character") /* REG_ECOLLATE */
    "\0"
#define REG_ECTYPE_IDX	(REG_ECOLLATE_IDX + sizeof "Invalid collation character")
    gettext_noop ("Invalid character class name") /* REG_ECTYPE */
    "\0"
#define REG_EESCAPE_IDX	(REG_ECTYPE_IDX + sizeof "Invalid character class name")
    gettext_noop ("Trailing backslash") /* REG_EESCAPE */
    "\0"
#define REG_ESUBREG_IDX	(REG_EESCAPE_IDX + sizeof "Trailing backslash")
    gettext_noop ("Invalid back reference") /* REG_ESUBREG */
    "\0"
#define REG_EBRACK_IDX	(REG_ESUBREG_IDX + sizeof "Invalid back reference")
    gettext_noop ("Unmatched [ or [^")	/* REG_EBRACK */
    "\0"
#define REG_EPAREN_IDX	(REG_EBRACK_IDX + sizeof "Unmatched [ or [^")
    gettext_noop ("Unmatched ( or \\(") /* REG_EPAREN */
    "\0"
#define REG_EBRACE_IDX	(REG_EPAREN_IDX + sizeof "Unmatched ( or \\(")
    gettext_noop ("Unmatched \\{") /* REG_EBRACE */
    "\0"
#define REG_BADBR_IDX	(REG_EBRACE_IDX + sizeof "Unmatched \\{")
    gettext_noop ("Invalid content of \\{\\}") /* REG_BADBR */
    "\0"
#define REG_ERANGE_IDX	(REG_BADBR_IDX + sizeof "Invalid content of \\{\\}")
    gettext_noop ("Invalid range end")	/* REG_ERANGE */
    "\0"
#define REG_ESPACE_IDX	(REG_ERANGE_IDX + sizeof "Invalid range end")
    gettext_noop ("Memory exhausted") /* REG_ESPACE */
    "\0"
#define REG_BADRPT_IDX	(REG_ESPACE_IDX + sizeof "Memory exhausted")
    gettext_noop ("Invalid preceding regular expression") /* REG_BADRPT */
    "\0"
#define REG_EEND_IDX	(REG_BADRPT_IDX + sizeof "Invalid preceding regular expression")
    gettext_noop ("Premature end of regular expression") /* REG_EEND */
    "\0"
#define REG_ESIZE_IDX	(REG_EEND_IDX + sizeof "Premature end of regular expression")
    gettext_noop ("Regular expression too big") /* REG_ESIZE */
    "\0"
#define REG_ERPAREN_IDX	(REG_ESIZE_IDX + sizeof "Regular expression too big")
    gettext_noop ("Unmatched ) or \\)") /* REG_ERPAREN */
  };

static const size_t __re_error_msgid_idx[] attribute_hidden =
  {
    REG_NOERROR_IDX,
    REG_NOMATCH_IDX,
    REG_BADPAT_IDX,
    REG_ECOLLATE_IDX,
    REG_ECTYPE_IDX,
    REG_EESCAPE_IDX,
    REG_ESUBREG_IDX,
    REG_EBRACK_IDX,
    REG_EPAREN_IDX,
    REG_EBRACE_IDX,
    REG_BADBR_IDX,
    REG_ERANGE_IDX,
    REG_ESPACE_IDX,
    REG_BADRPT_IDX,
    REG_EEND_IDX,
    REG_ESIZE_IDX,
    REG_ERPAREN_IDX
  };

/* Entry points for GNU code.  */

/* re_compile_pattern is the GNU regular expression compiler: it
   compiles PATTERN (of length LENGTH) and puts the result in BUFP.
   Returns 0 if the pattern was valid, otherwise an error string.

   Assumes the `allocated' (and perhaps `buffer') and `translate' fields
   are set in BUFP on entry.  */

#if 0
const char *
re_compile_pattern (pattern, length, bufp)
    const char *pattern;
    size_t length;
    struct re_pattern_buffer *bufp;
{
  reg_errcode_t ret;

  /* And GNU code determines whether or not to get register information
     by passing null for the REGS argument to re_match, etc., not by
     setting no_sub.  */
  bufp->no_sub = 0;

  /* Match anchors at newline.  */
  bufp->newline_anchor = 1;

  ret = re_compile_internal (bufp, pattern, length, re_syntax_options);

  if (!ret)
    return NULL;
  return gettext (__re_error_msgid + __re_error_msgid_idx[(int) ret]);
}
#ifdef _LIBC
weak_alias (__re_compile_pattern, re_compile_pattern)
#endif
#endif

/* Set by `re_set_syntax' to the current regexp syntax to recognize.  Can
   also be assigned to arbitrarily: each pattern buffer stores its own
   syntax, so it can be changed between regex compilations.  */
/* This has no initializer because initialized variables in Emacs
   become read-only after dumping.  */
#if 0 /* used earlier */
static reg_syntax_t re_syntax_options;

/* Specify the precise syntax of regexps for compilation.  This provides
   for compatibility for various utilities which historically have
   different, incompatible syntaxes.

   The argument SYNTAX is a bit mask comprised of the various bits
   defined in regex.h.  We return the old syntax.  */

reg_syntax_t
re_set_syntax (syntax)
    reg_syntax_t syntax;
{
  reg_syntax_t ret = re_syntax_options;

  re_syntax_options = syntax;
  return ret;
}
#ifdef _LIBC
weak_alias (__re_set_syntax, re_set_syntax)
#endif
#endif

static int
re_compile_fastmap (bufp)
    struct re_pattern_buffer *bufp;
{
  re_dfa_t *dfa = (re_dfa_t *) bufp->buffer;
  char *fastmap = bufp->fastmap;

  memset (fastmap, '\0', sizeof (char) * SBC_MAX);
  re_compile_fastmap_iter (bufp, dfa->init_state, fastmap);
  if (dfa->init_state != dfa->init_state_word)
    re_compile_fastmap_iter (bufp, dfa->init_state_word, fastmap);
  if (dfa->init_state != dfa->init_state_nl)
    re_compile_fastmap_iter (bufp, dfa->init_state_nl, fastmap);
  if (dfa->init_state != dfa->init_state_begbuf)
    re_compile_fastmap_iter (bufp, dfa->init_state_begbuf, fastmap);
  bufp->fastmap_accurate = 1;
  return 0;
}
#ifdef _LIBC
weak_alias (__re_compile_fastmap, re_compile_fastmap)
#endif

static R_INLINE void
__attribute ((always_inline))
re_set_fastmap (char *fastmap, int icase, int ch)
{
  fastmap[ch] = 1;
  if (icase)
    fastmap[tolower (ch)] = 1;
}

/* Helper function for re_compile_fastmap.
   Compile fastmap for the initial_state INIT_STATE.  */

static void
re_compile_fastmap_iter (bufp, init_state, fastmap)
     regex_t *bufp;
     const re_dfastate_t *init_state;
     char *fastmap;
{
  re_dfa_t *dfa = (re_dfa_t *) bufp->buffer;
  int node_cnt;
  int icase = (dfa->mb_cur_max == 1 && (bufp->syntax & RE_ICASE));
  for (node_cnt = 0; node_cnt < init_state->nodes.nelem; ++node_cnt)
    {
      int node = init_state->nodes.elems[node_cnt];
      re_token_type_t type = dfa->nodes[node].type;

      if (type == CHARACTER)
	{
	  re_set_fastmap (fastmap, icase, dfa->nodes[node].opr.c);
#ifdef RE_ENABLE_I18N
	  if ((bufp->syntax & RE_ICASE) && dfa->mb_cur_max > 1)
	    {
	      unsigned char *buf = alloca (dfa->mb_cur_max), *p;
	      wchar_t wc;
	      mbstate_t state;

	      p = buf;
	      *p++ = dfa->nodes[node].opr.c;
	      while (++node < dfa->nodes_len
		     &&	dfa->nodes[node].type == CHARACTER
		     && dfa->nodes[node].mb_partial)
		*p++ = dfa->nodes[node].opr.c;
	      memset (&state, 0, sizeof (state));
	      if (mbrtowc (&wc, (const char *) buf, p - buf,
			   &state) == p - buf
		  && __wcrtomb ((char *) buf, towlower (wc), &state) > 0)
		re_set_fastmap (fastmap, 0, buf[0]);
	    }
#endif
	}
      else if (type == SIMPLE_BRACKET)
	{
	  int i, j, ch;
	  for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
	    for (j = 0; j < UINT_BITS; ++j, ++ch)
	      if (dfa->nodes[node].opr.sbcset[i] & (1 << j))
		re_set_fastmap (fastmap, icase, ch);
	}
#ifdef RE_ENABLE_I18N
      else if (type == COMPLEX_BRACKET)
	{
	  int i;
	  re_charset_t *cset = dfa->nodes[node].opr.mbcset;
	  if (cset->non_match || cset->ncoll_syms || cset->nequiv_classes
	      || cset->nranges || cset->nchar_classes)
	    {
# ifdef _LIBC
	      if (_NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES) != 0)
		{
		  /* In this case we want to catch the bytes which are
		     the first byte of any collation elements.
		     e.g. In da_DK, we want to catch 'a' since "aa"
			  is a valid collation element, and don't catch
			  'b' since 'b' is the only collation element
			  which starts from 'b'.  */
		  int j, ch;
		  const int32_t *table = (const int32_t *)
		    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
		  for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
		    for (j = 0; j < UINT_BITS; ++j, ++ch)
		      if (table[ch] < 0)
			re_set_fastmap (fastmap, icase, ch);
		}
# else
	      if (dfa->mb_cur_max > 1)
		for (i = 0; i < SBC_MAX; ++i)
		  if (__btowc (i) == WEOF)
		    re_set_fastmap (fastmap, icase, i);
# endif /* not _LIBC */
	    }
	  for (i = 0; i < cset->nmbchars; ++i)
	    {
	      char buf[256];
	      mbstate_t state;
	      memset (&state, '\0', sizeof (state));
	      __wcrtomb (buf, cset->mbchars[i], &state);
	      re_set_fastmap (fastmap, icase, *(unsigned char *) buf);
	      if ((bufp->syntax & RE_ICASE) && dfa->mb_cur_max > 1)
		{
		  __wcrtomb (buf, towlower (cset->mbchars[i]), &state);
		  re_set_fastmap (fastmap, 0, *(unsigned char *) buf);
		}
	    }
	}
#endif /* RE_ENABLE_I18N */
      else if (type == END_OF_RE || type == OP_PERIOD
	       || type == OP_UTF8_PERIOD)
	{
	  memset (fastmap, '\1', sizeof (char) * SBC_MAX);
	  if (type == END_OF_RE)
	    bufp->can_be_null = 1;
	  return;
	}
    }
}

/* Entry point for POSIX code.  */
/* regcomp takes a regular expression as a string and compiles it.

   PREG is a regex_t *.  We do not expect any fields to be initialized,
   since POSIX says we shouldn't.  Thus, we set

     `buffer' to the compiled pattern;
     `used' to the length of the compiled pattern;
     `syntax' to RE_SYNTAX_POSIX_EXTENDED if the
       REG_EXTENDED bit in CFLAGS is set; otherwise, to
       RE_SYNTAX_POSIX_BASIC;
     `newline_anchor' to REG_NEWLINE being set in CFLAGS;
     `fastmap' to an allocated space for the fastmap;
     `fastmap_accurate' to zero;
     `re_nsub' to the number of subexpressions in PATTERN.

   PATTERN is the address of the pattern string.

   CFLAGS is a series of bits which affect compilation.

     If REG_EXTENDED is set, we use POSIX extended syntax; otherwise, we
     use POSIX basic syntax.

     If REG_NEWLINE is set, then . and [^...] don't match newline.
     Also, regexec will try a match beginning after every newline.

     If REG_ICASE is set, then we considers upper- and lowercase
     versions of letters to be equivalent when matching.

     If REG_NOSUB is set, then when PREG is passed to regexec, that
     routine will report only success or failure, and nothing about the
     registers.

   It returns 0 if it succeeds, nonzero if it doesn't.  (See regex.h for
   the return codes and their meanings.)  */

int
regcomp (preg, pattern, cflags)
    regex_t *__restrict preg;
    const char *__restrict pattern;
    int cflags;
{
  reg_errcode_t ret;
  reg_syntax_t syntax = ((cflags & REG_EXTENDED) ? RE_SYNTAX_POSIX_EXTENDED
			 : RE_SYNTAX_POSIX_BASIC);

  preg->buffer = NULL;
  preg->allocated = 0;
  preg->used = 0;

  /* Try to allocate space for the fastmap.  */
  preg->fastmap = re_malloc (char, SBC_MAX);
  if (BE (preg->fastmap == NULL, 0))
    return REG_ESPACE;

  syntax |= (cflags & REG_ICASE) ? RE_ICASE : 0;

  /* If REG_NEWLINE is set, newlines are treated differently.  */
  if (cflags & REG_NEWLINE)
    { /* REG_NEWLINE implies neither . nor [^...] match newline.  */
      syntax &= ~RE_DOT_NEWLINE;
      syntax |= RE_HAT_LISTS_NOT_NEWLINE;
      /* It also changes the matching behavior.  */
      preg->newline_anchor = 1;
    }
  else
    preg->newline_anchor = 0;
  preg->no_sub = !!(cflags & REG_NOSUB);
  preg->translate = NULL;

  ret = re_compile_internal (preg, pattern, strlen (pattern), syntax);

  /* POSIX doesn't distinguish between an unmatched open-group and an
     unmatched close-group: both are REG_EPAREN.  */
  if (ret == REG_ERPAREN)
    ret = REG_EPAREN;

  /* We have already checked preg->fastmap != NULL.  */
  if (BE (ret == REG_NOERROR, 1))
    /* Compute the fastmap now, since regexec cannot modify the pattern
       buffer.  This function nevers fails in this implementation.  */
    (void) re_compile_fastmap (preg);
  else
    {
      /* Some error occurred while compiling the expression.  */
      re_free (preg->fastmap);
      preg->fastmap = NULL;
    }

  return (int) ret;
}
#ifdef _LIBC
weak_alias (__regcomp, regcomp)
#endif

/* Returns a message corresponding to an error code, ERRCODE, returned
   from either regcomp or regexec.   We don't use PREG here.  */

size_t
regerror (errcode, preg, errbuf, errbuf_size)
    int errcode;
    const regex_t *preg;
    char *errbuf;
    size_t errbuf_size;
{
  const char *msg;
  size_t msg_size;

  if (BE (errcode < 0
	  || errcode >= (int) (sizeof (__re_error_msgid_idx)
			       / sizeof (__re_error_msgid_idx[0])), 0))
    /* Only error codes returned by the rest of the code should be passed
       to this routine.  If we are given anything else, or if other regex
       code generates an invalid error code, then the program has a bug.
       Dump core so we can fix it.  */
    error ("internal error in 'regex' code");

  msg = gettext (__re_error_msgid + __re_error_msgid_idx[errcode]);

  msg_size = strlen (msg) + 1; /* Includes the null.  */

  if (BE (errbuf_size != 0, 1))
    {
      if (BE (msg_size > errbuf_size, 0))
	{
#if defined HAVE_MEMPCPY || defined _LIBC
	  *((char *) __mempcpy (errbuf, msg, errbuf_size - 1)) = '\0';
#else
	  memcpy (errbuf, msg, errbuf_size - 1);
	  errbuf[errbuf_size - 1] = 0;
#endif
	}
      else
	memcpy (errbuf, msg, msg_size);
    }

  return msg_size;
}
#ifdef _LIBC
weak_alias (__regerror, regerror)
#endif


static void
free_dfa_content (re_dfa_t *dfa)
{
  int i, j;

  re_free (dfa->subexps);

  if (dfa->nodes)
    for (i = 0; i < dfa->nodes_len; ++i)
      {
	re_token_t *node = dfa->nodes + i;
#ifdef RE_ENABLE_I18N
	if (node->type == COMPLEX_BRACKET && node->duplicated == 0)
	  free_charset (node->opr.mbcset);
	else
#endif /* RE_ENABLE_I18N */
	  if (node->type == SIMPLE_BRACKET && node->duplicated == 0)
	    re_free (node->opr.sbcset);
      }
  re_free (dfa->nexts);
  for (i = 0; i < dfa->nodes_len; ++i)
    {
      if (dfa->eclosures != NULL)
	re_node_set_free (dfa->eclosures + i);
      if (dfa->inveclosures != NULL)
	re_node_set_free (dfa->inveclosures + i);
      if (dfa->edests != NULL)
	re_node_set_free (dfa->edests + i);
    }
  re_free (dfa->edests);
  re_free (dfa->eclosures);
  re_free (dfa->inveclosures);
  re_free (dfa->nodes);

  if (dfa->state_table)
    for (i = 0; i <= dfa->state_hash_mask; ++i)
      {
	struct re_state_table_entry *entry = dfa->state_table + i;
	for (j = 0; j < entry->num; ++j)
	  {
	    re_dfastate_t *state = entry->array[j];
	    free_state (state);
	  }
        re_free (entry->array);
      }
  re_free (dfa->state_table);
  re_free (dfa->word_char);
#ifdef RE_ENABLE_I18N
  re_free (dfa->sb_char);
#endif
#ifdef DEBUG
  re_free (dfa->re_str);
#endif

  re_free (dfa);
}


/* Free dynamically allocated space used by PREG.  */

void
regfree (preg)
    regex_t *preg;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  if (BE (dfa != NULL, 1))
    free_dfa_content (dfa);

  re_free (preg->fastmap);
}
#ifdef _LIBC
weak_alias (__regfree, regfree)
#endif

/* Entry points compatible with 4.2 BSD regex library.  We don't define
   them unless specifically requested.  */

#if defined _REGEX_RE_COMP || defined _LIBC

/* BSD has one and only one pattern buffer.  */
static struct re_pattern_buffer re_comp_buf;

char *
# ifdef _LIBC
/* Make these definitions weak in libc, so POSIX programs can redefine
   these names if they don't use our functions, and still use
   regcomp/regexec above without link errors.  */
weak_function
# endif
re_comp (s)
     const char *s;
{
  reg_errcode_t ret;
  char *fastmap;

  if (!s)
    {
      if (!re_comp_buf.buffer)
	return gettext ("No previous regular expression");
      return 0;
    }

  if (re_comp_buf.buffer)
    {
      fastmap = re_comp_buf.fastmap;
      re_comp_buf.fastmap = NULL;
      __regfree (&re_comp_buf);
      memset (&re_comp_buf, '\0', sizeof (re_comp_buf));
      re_comp_buf.fastmap = fastmap;
    }

  if (re_comp_buf.fastmap == NULL)
    {
      re_comp_buf.fastmap = (char *) malloc (SBC_MAX);
      if (re_comp_buf.fastmap == NULL)
	return (char *) gettext (__re_error_msgid
				 + __re_error_msgid_idx[(int) REG_ESPACE]);
    }

  /* Since `re_exec' always passes NULL for the `regs' argument, we
     don't need to initialize the pattern buffer fields which affect it.  */

  /* Match anchors at newlines.  */
  re_comp_buf.newline_anchor = 1;

  ret = re_compile_internal (&re_comp_buf, s, strlen (s), re_syntax_options);

  if (!ret)
    return NULL;

  /* Yes, we're discarding `const' here if !HAVE_LIBINTL.  */
  return (char *) gettext (__re_error_msgid + __re_error_msgid_idx[(int) ret]);
}

#ifdef _LIBC
libc_freeres_fn (free_mem)
{
  __regfree (&re_comp_buf);
}
#endif

#endif /* _REGEX_RE_COMP */

/* Internal entry point.
   Compile the regular expression PATTERN, whose length is LENGTH.
   SYNTAX indicate regular expression's syntax.  */

static reg_errcode_t
re_compile_internal (preg, pattern, length, syntax)
     regex_t *preg;
     const char * pattern;
     int length;
     reg_syntax_t syntax;
{
  reg_errcode_t err = REG_NOERROR;
  re_dfa_t *dfa;
  re_string_t regexp;

  /* Initialize the pattern buffer.  */
  preg->fastmap_accurate = 0;
  preg->syntax = syntax;
  preg->not_bol = preg->not_eol = 0;
  preg->used = 0;
  preg->re_nsub = 0;
  preg->can_be_null = 0;
  preg->regs_allocated = REGS_UNALLOCATED;

  /* Initialize the dfa.  */
  dfa = (re_dfa_t *) preg->buffer;
  if (BE (preg->allocated < sizeof (re_dfa_t), 0))
    {
      /* If zero allocated, but buffer is non-null, try to realloc
	 enough space.  This loses if buffer's address is bogus, but
	 that is the user's responsibility.  If ->buffer is NULL this
	 is a simple allocation.  */
      dfa = re_realloc (preg->buffer, re_dfa_t, 1);
      if (dfa == NULL)
	return REG_ESPACE;
      preg->allocated = sizeof (re_dfa_t);
      preg->buffer = (unsigned char *) dfa;
    }
  preg->used = sizeof (re_dfa_t);

  err = init_dfa (dfa, length);
  if (BE (err != REG_NOERROR, 0))
    {
      free_dfa_content (dfa);
      preg->buffer = NULL;
      preg->allocated = 0;
      return err;
    }
#ifdef DEBUG
  dfa->re_str = re_malloc (char, length + 1);
  strncpy (dfa->re_str, pattern, length + 1);
#endif

  err = re_string_construct (&regexp, pattern, length, preg->translate,
			     syntax & RE_ICASE, dfa);
  if (BE (err != REG_NOERROR, 0))
    {
    re_compile_internal_free_return:
      free_workarea_compile (preg);
      re_string_destruct (&regexp);
      free_dfa_content (dfa);
      preg->buffer = NULL;
      preg->allocated = 0;
      return err;
    }

  /* Parse the regular expression, and build a structure tree.  */
  preg->re_nsub = 0;
  dfa->str_tree = parse (&regexp, preg, syntax, &err);
  if (BE (dfa->str_tree == NULL, 0))
    goto re_compile_internal_free_return;

#ifdef RE_ENABLE_I18N
  /* If possible, do searching in single byte encoding to speed things up.  */
  if (dfa->is_utf8 && !(syntax & RE_ICASE) && preg->translate == NULL)
    optimize_utf8 (dfa);
#endif

  /* Analyze the tree and collect information which is necessary to
     create the dfa.  */
  err = analyze (dfa);
  if (BE (err != REG_NOERROR, 0))
    goto re_compile_internal_free_return;

  /* Then create the initial state of the dfa.  */
  err = create_initial_state (dfa);

  /* Release work areas.  */
  free_workarea_compile (preg);
  re_string_destruct (&regexp);

  if (BE (err != REG_NOERROR, 0))
    {
      free_dfa_content (dfa);
      preg->buffer = NULL;
      preg->allocated = 0;
    }

  return err;
}

/* Initialize DFA.  We use the length of the regular expression PAT_LEN
   as the initial length of some arrays.  */

static reg_errcode_t
init_dfa (dfa, pat_len)
     re_dfa_t *dfa;
     int pat_len;
{
  int table_size;

  memset (dfa, '\0', sizeof (re_dfa_t));

  /* Force allocation of str_tree_storage the first time.  */
  dfa->str_tree_storage_idx = BIN_TREE_STORAGE_SIZE;

  dfa->nodes_alloc = pat_len + 1;
  dfa->nodes = re_malloc (re_token_t, dfa->nodes_alloc);

  dfa->states_alloc = pat_len + 1;

  /*  table_size = 2 ^ ceil(log pat_len) */
  for (table_size = 1; table_size > 0; table_size <<= 1)
    if (table_size > pat_len)
      break;

  dfa->state_table = calloc (sizeof (struct re_state_table_entry), table_size);
  dfa->state_hash_mask = table_size - 1;

  dfa->subexps_alloc = 1;
  dfa->subexps = re_malloc (re_subexp_t, dfa->subexps_alloc);
  /* dfa->word_char = NULL; */

  dfa->mb_cur_max = MB_CUR_MAX;
#ifdef _LIBC
  if (dfa->mb_cur_max == 6
      && strcmp (_NL_CURRENT (LC_CTYPE, _NL_CTYPE_CODESET_NAME), "UTF-8") == 0)
    dfa->is_utf8 = 1;
  dfa->map_notascii = (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_MAP_TO_NONASCII)
		       != 0);
#endif
#ifdef RE_ENABLE_I18N
  if (dfa->mb_cur_max > 1)
    {
      int i, j, ch;

      dfa->sb_char = (re_bitset_ptr_t) calloc (sizeof (bitset), 1);
      if (BE (dfa->sb_char == NULL, 0))
	return REG_ESPACE;
#ifdef _LIBC
      if (dfa->is_utf8)
	memset (dfa->sb_char, 255, sizeof (unsigned int) * BITSET_UINTS / 2);
      else
#endif
	for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
	  for (j = 0; j < UINT_BITS; ++j, ++ch)
	    if (btowc (ch) != WEOF)
	      dfa->sb_char[i] |= 1 << j;
    }
#endif

  if (BE (dfa->nodes == NULL || dfa->state_table == NULL
	  || dfa->subexps == NULL, 0))
    return REG_ESPACE;
  return REG_NOERROR;
}

/* Initialize WORD_CHAR table, which indicate which character is
   "word".  In this case "word" means that it is the word construction
   character used by some operators like "\<", "\>", etc.  */

static reg_errcode_t
init_word_char (dfa)
     re_dfa_t *dfa;
{
  int i, j, ch;
  dfa->word_char = (re_bitset_ptr_t) calloc (sizeof (bitset), 1);
  if (BE (dfa->word_char == NULL, 0))
    return REG_ESPACE;
  for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
    for (j = 0; j < UINT_BITS; ++j, ++ch)
      if (isalnum (ch) || ch == '_')
	dfa->word_char[i] |= 1 << j;
  return REG_NOERROR;
}

/* Free the work area which are only used while compiling.  */

static void
free_workarea_compile (preg)
     regex_t *preg;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  bin_tree_storage_t *storage, *next;
  for (storage = dfa->str_tree_storage; storage; storage = next)
    {
      next = storage->next;
      re_free (storage);
    }
  dfa->str_tree_storage = NULL;
  dfa->str_tree_storage_idx = BIN_TREE_STORAGE_SIZE;
  dfa->str_tree = NULL;
  re_free (dfa->org_indices);
  dfa->org_indices = NULL;
}

/* Create initial states for all contexts.  */

static reg_errcode_t
create_initial_state (dfa)
     re_dfa_t *dfa;
{
  int first, i;
  reg_errcode_t err;
  re_node_set init_nodes;

  /* Initial states have the epsilon closure of the node which is
     the first node of the regular expression.  */
  first = dfa->str_tree->first;
  dfa->init_node = first;
  err = re_node_set_init_copy (&init_nodes, dfa->eclosures + first);
  if (BE (err != REG_NOERROR, 0))
    return err;

  /* The back-references which are in initial states can epsilon transit,
     since in this case all of the subexpressions can be null.
     Then we add epsilon closures of the nodes which are the next nodes of
     the back-references.  */
  if (dfa->nbackref > 0)
    for (i = 0; i < init_nodes.nelem; ++i)
      {
	int node_idx = init_nodes.elems[i];
	re_token_type_t type = dfa->nodes[node_idx].type;

	int clexp_idx;
	if (type != OP_BACK_REF)
	  continue;
	for (clexp_idx = 0; clexp_idx < init_nodes.nelem; ++clexp_idx)
	  {
	    re_token_t *clexp_node;
	    clexp_node = dfa->nodes + init_nodes.elems[clexp_idx];
	    if (clexp_node->type == OP_CLOSE_SUBEXP
		&& clexp_node->opr.idx + 1 == dfa->nodes[node_idx].opr.idx)
	      break;
	  }
	if (clexp_idx == init_nodes.nelem)
	  continue;

	if (type == OP_BACK_REF)
	  {
	    int dest_idx = dfa->edests[node_idx].elems[0];
	    if (!re_node_set_contains (&init_nodes, dest_idx))
	      {
		re_node_set_merge (&init_nodes, dfa->eclosures + dest_idx);
		i = 0;
	      }
	  }
      }

  /* It must be the first time to invoke acquire_state.  */
  dfa->init_state = re_acquire_state_context (&err, dfa, &init_nodes, 0);
  /* We don't check ERR here, since the initial state must not be NULL.  */
  if (BE (dfa->init_state == NULL, 0))
    return err;
  if (dfa->init_state->has_constraint)
    {
      dfa->init_state_word = re_acquire_state_context (&err, dfa, &init_nodes,
						       CONTEXT_WORD);
      dfa->init_state_nl = re_acquire_state_context (&err, dfa, &init_nodes,
						     CONTEXT_NEWLINE);
      dfa->init_state_begbuf = re_acquire_state_context (&err, dfa,
							 &init_nodes,
							 CONTEXT_NEWLINE
							 | CONTEXT_BEGBUF);
      if (BE (dfa->init_state_word == NULL || dfa->init_state_nl == NULL
	      || dfa->init_state_begbuf == NULL, 0))
	return err;
    }
  else
    dfa->init_state_word = dfa->init_state_nl
      = dfa->init_state_begbuf = dfa->init_state;

  re_node_set_free (&init_nodes);
  return REG_NOERROR;
}

#ifdef RE_ENABLE_I18N
/* If it is possible to do searching in single byte encoding instead of UTF-8
   to speed things up, set dfa->mb_cur_max to 1, clear is_utf8 and change
   DFA nodes where needed.  */

static void
optimize_utf8 (dfa)
     re_dfa_t *dfa;
{
  int node, i, mb_chars = 0, has_period = 0;

  for (node = 0; node < dfa->nodes_len; ++node)
    switch (dfa->nodes[node].type)
      {
      case CHARACTER:
	if (dfa->nodes[node].opr.c >= 0x80)
	  mb_chars = 1;
	break;
      case ANCHOR:
	switch (dfa->nodes[node].opr.idx)
	  {
	  case LINE_FIRST:
	  case LINE_LAST:
	  case BUF_FIRST:
	  case BUF_LAST:
	    break;
	  default:
	    /* Word anchors etc. cannot be handled.  */
	    return;
	  }
	break;
      case OP_PERIOD:
        has_period = 1;
        break;
      case OP_BACK_REF:
      case OP_ALT:
      case END_OF_RE:
      case OP_DUP_ASTERISK:
      case OP_DUP_QUESTION:
      case OP_DUP_PLUS:
      case OP_OPEN_SUBEXP:
      case OP_CLOSE_SUBEXP:
	break;
      case SIMPLE_BRACKET:
	/* Just double check.  */
        for (i = 0x80 / UINT_BITS; i < BITSET_UINTS; ++i)
	  if (dfa->nodes[node].opr.sbcset[i])
	    return;
	break;
      default:
	return;
      }

  if (mb_chars || has_period)
    for (node = 0; node < dfa->nodes_len; ++node)
      {
	if (dfa->nodes[node].type == CHARACTER
	    && dfa->nodes[node].opr.c >= 0x80)
	  dfa->nodes[node].mb_partial = 0;
	else if (dfa->nodes[node].type == OP_PERIOD)
	  dfa->nodes[node].type = OP_UTF8_PERIOD;
      }

  /* The search can be in single byte locale.  */
  dfa->mb_cur_max = 1;
  dfa->is_utf8 = 0;
  dfa->has_mb_node = dfa->nbackref > 0 || has_period;
}
#endif

/* Analyze the structure tree, and calculate "first", "next", "edest",
   "eclosure", and "inveclosure".  */

static reg_errcode_t
analyze (dfa)
     re_dfa_t *dfa;
{
  int i;
  reg_errcode_t ret;

  /* Allocate arrays.  */
  dfa->nexts = re_malloc (int, dfa->nodes_alloc);
  dfa->org_indices = re_malloc (int, dfa->nodes_alloc);
  dfa->edests = re_malloc (re_node_set, dfa->nodes_alloc);
  dfa->eclosures = re_malloc (re_node_set, dfa->nodes_alloc);
  dfa->inveclosures = re_malloc (re_node_set, dfa->nodes_alloc);
  if (BE (dfa->nexts == NULL || dfa->org_indices == NULL || dfa->edests == NULL
	  || dfa->eclosures == NULL || dfa->inveclosures == NULL, 0))
    return REG_ESPACE;
  /* Initialize them.  */
  for (i = 0; i < dfa->nodes_len; ++i)
    {
      dfa->nexts[i] = -1;
      re_node_set_init_empty (dfa->edests + i);
      re_node_set_init_empty (dfa->eclosures + i);
      re_node_set_init_empty (dfa->inveclosures + i);
    }

  ret = analyze_tree (dfa, dfa->str_tree);
  if (BE (ret == REG_NOERROR, 1))
    {
      ret = calc_eclosure (dfa);
      if (ret == REG_NOERROR)
	calc_inveclosure (dfa);
    }
  return ret;
}

/* Helper functions for analyze.
   This function calculate "first", "next", and "edest" for the subtree
   whose root is NODE.  */

static reg_errcode_t
analyze_tree (dfa, node)
     re_dfa_t *dfa;
     bin_tree_t *node;
{
  reg_errcode_t ret;
  if (node->first == -1)
    calc_first (dfa, node);
  if (node->next == -1)
    calc_next (dfa, node);
  if (node->eclosure.nelem == 0)
    calc_epsdest (dfa, node);
  /* Calculate "first" etc. for the left child.  */
  if (node->left != NULL)
    {
      ret = analyze_tree (dfa, node->left);
      if (BE (ret != REG_NOERROR, 0))
	return ret;
    }
  /* Calculate "first" etc. for the right child.  */
  if (node->right != NULL)
    {
      ret = analyze_tree (dfa, node->right);
      if (BE (ret != REG_NOERROR, 0))
	return ret;
    }
  return REG_NOERROR;
}

/* Calculate "first" for the node NODE.  */
static void
calc_first (dfa, node)
     re_dfa_t *dfa;
     bin_tree_t *node;
{
  int idx, type;
  idx = node->node_idx;
  type = (node->type == 0) ? dfa->nodes[idx].type : node->type;

  switch (type)
    {
#ifdef DEBUG
    case OP_OPEN_BRACKET:
    case OP_CLOSE_BRACKET:
    case OP_OPEN_DUP_NUM:
    case OP_CLOSE_DUP_NUM:
    case OP_NON_MATCH_LIST:
    case OP_OPEN_COLL_ELEM:
    case OP_CLOSE_COLL_ELEM:
    case OP_OPEN_EQUIV_CLASS:
    case OP_CLOSE_EQUIV_CLASS:
    case OP_OPEN_CHAR_CLASS:
    case OP_CLOSE_CHAR_CLASS:
      /* These must not be appeared here.  */
      assert (0);
#endif
    case END_OF_RE:
    case CHARACTER:
    case OP_PERIOD:
    case OP_DUP_ASTERISK:
    case OP_DUP_QUESTION:
#ifdef RE_ENABLE_I18N
    case OP_UTF8_PERIOD:
    case COMPLEX_BRACKET:
#endif /* RE_ENABLE_I18N */
    case SIMPLE_BRACKET:
    case OP_BACK_REF:
    case ANCHOR:
    case OP_OPEN_SUBEXP:
    case OP_CLOSE_SUBEXP:
      node->first = idx;
      break;
    case OP_DUP_PLUS:
#ifdef DEBUG
      assert (node->left != NULL);
#endif
      if (node->left->first == -1)
	calc_first (dfa, node->left);
      node->first = node->left->first;
      break;
    case OP_ALT:
      node->first = idx;
      break;
      /* else fall through */
    default:
#ifdef DEBUG
      assert (node->left != NULL);
#endif
      if (node->left->first == -1)
	calc_first (dfa, node->left);
      node->first = node->left->first;
      break;
    }
}

/* Calculate "next" for the node NODE.  */

static void
calc_next (dfa, node)
     re_dfa_t *dfa;
     bin_tree_t *node;
{
  int idx, type;
  bin_tree_t *parent = node->parent;
  if (parent == NULL)
    {
      node->next = -1;
      idx = node->node_idx;
      if (node->type == 0)
	dfa->nexts[idx] = node->next;
      return;
    }

  idx = parent->node_idx;
  type = (parent->type == 0) ? dfa->nodes[idx].type : parent->type;

  switch (type)
    {
    case OP_DUP_ASTERISK:
    case OP_DUP_PLUS:
      node->next = idx;
      break;
    case CONCAT:
      if (parent->left == node)
	{
	  if (parent->right->first == -1)
	    calc_first (dfa, parent->right);
	  node->next = parent->right->first;
	  break;
	}
      /* else fall through */
    default:
      if (parent->next == -1)
	calc_next (dfa, parent);
      node->next = parent->next;
      break;
    }
  idx = node->node_idx;
  if (node->type == 0)
    dfa->nexts[idx] = node->next;
}

/* Calculate "edest" for the node NODE.  */

static void
calc_epsdest (dfa, node)
     re_dfa_t *dfa;
     bin_tree_t *node;
{
  int idx;
  idx = node->node_idx;
  if (node->type == 0)
    {
      if (dfa->nodes[idx].type == OP_DUP_ASTERISK
	  || dfa->nodes[idx].type == OP_DUP_PLUS
	  || dfa->nodes[idx].type == OP_DUP_QUESTION)
	{
	  if (node->left->first == -1)
	    calc_first (dfa, node->left);
	  if (node->next == -1)
	    calc_next (dfa, node);
	  re_node_set_init_2 (dfa->edests + idx, node->left->first,
			      node->next);
	}
      else if (dfa->nodes[idx].type == OP_ALT)
	{
	  int left, right;
	  if (node->left != NULL)
	    {
	      if (node->left->first == -1)
		calc_first (dfa, node->left);
	      left = node->left->first;
	    }
	  else
	    {
	      if (node->next == -1)
		calc_next (dfa, node);
	      left = node->next;
	    }
	  if (node->right != NULL)
	    {
	      if (node->right->first == -1)
		calc_first (dfa, node->right);
	      right = node->right->first;
	    }
	  else
	    {
	      if (node->next == -1)
		calc_next (dfa, node);
	      right = node->next;
	    }
	  re_node_set_init_2 (dfa->edests + idx, left, right);
	}
      else if (dfa->nodes[idx].type == ANCHOR
	       || dfa->nodes[idx].type == OP_OPEN_SUBEXP
	       || dfa->nodes[idx].type == OP_CLOSE_SUBEXP
	       || dfa->nodes[idx].type == OP_BACK_REF)
	re_node_set_init_1 (dfa->edests + idx, node->next);
      else
        assert (!IS_EPSILON_NODE (dfa->nodes[idx].type));
    }
}

/* Duplicate the epsilon closure of the node ROOT_NODE.
   Note that duplicated nodes have constraint INIT_CONSTRAINT in addition
   to their own constraint.  */

static reg_errcode_t
duplicate_node_closure (dfa, top_org_node, top_clone_node, root_node,
			init_constraint)
     re_dfa_t *dfa;
     int top_org_node, top_clone_node, root_node;
     unsigned int init_constraint;
{
  reg_errcode_t err;
  int org_node, clone_node, ret;
  unsigned int constraint = init_constraint;
  for (org_node = top_org_node, clone_node = top_clone_node;;)
    {
      int org_dest, clone_dest;
      if (dfa->nodes[org_node].type == OP_BACK_REF)
	{
	  /* If the back reference epsilon-transit, its destination must
	     also have the constraint.  Then duplicate the epsilon closure
	     of the destination of the back reference, and store it in
	     edests of the back reference.  */
	  org_dest = dfa->nexts[org_node];
	  re_node_set_empty (dfa->edests + clone_node);
	  err = duplicate_node (&clone_dest, dfa, org_dest, constraint);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	  dfa->nexts[clone_node] = dfa->nexts[org_node];
	  if (clone_dest == -1)
	    break;
	  ret = re_node_set_insert (dfa->edests + clone_node, clone_dest);
	  if (BE (ret < 0, 0))
	    return REG_ESPACE;
	}
      else if (dfa->edests[org_node].nelem == 0)
	{
	  /* In case of the node can't epsilon-transit, don't duplicate the
	     destination and store the original destination as the
	     destination of the node.  */
	  dfa->nexts[clone_node] = dfa->nexts[org_node];
	  break;
	}
      else if (dfa->edests[org_node].nelem == 1)
	{
	  /* In case of the node can epsilon-transit, and it has only one
	     destination.  */
	  org_dest = dfa->edests[org_node].elems[0];
	  re_node_set_empty (dfa->edests + clone_node);
	  if (dfa->nodes[org_node].type == ANCHOR)
	    {
	      /* In case of the node has another constraint, append it.  */
	      if (org_node == root_node && clone_node != org_node)
		{
		  /* ...but if the node is root_node itself, it means the
		     epsilon closure have a loop, then tie it to the
		     destination of the root_node.  */
		  ret = re_node_set_insert (dfa->edests + clone_node,
					    org_dest);
		  if (BE (ret < 0, 0))
		    return REG_ESPACE;
		  break;
		}
	      constraint |= dfa->nodes[org_node].opr.ctx_type;
	    }
	  err = duplicate_node (&clone_dest, dfa, org_dest, constraint);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	  if (clone_dest == -1)
	    break;
	  ret = re_node_set_insert (dfa->edests + clone_node, clone_dest);
	  if (BE (ret < 0, 0))
	    return REG_ESPACE;
	}
      else /* dfa->edests[org_node].nelem == 2 */
	{
	  /* In case of the node can epsilon-transit, and it has two
	     destinations. E.g. '|', '*', '+', '?'.   */
	  org_dest = dfa->edests[org_node].elems[0];
	  re_node_set_empty (dfa->edests + clone_node);
	  /* Search for a duplicated node which satisfies the constraint.  */
	  clone_dest = search_duplicated_node (dfa, org_dest, constraint);
	  if (clone_dest == -1)
	    {
	      /* There are no such a duplicated node, create a new one.  */
	      err = duplicate_node (&clone_dest, dfa, org_dest, constraint);
	      if (BE (err != REG_NOERROR, 0))
		return err;
	      if (clone_dest != -1)
		{
		  ret = re_node_set_insert (dfa->edests + clone_node, clone_dest);
		  if (BE (ret < 0, 0))
		    return REG_ESPACE;
		  err = duplicate_node_closure (dfa, org_dest, clone_dest,
						root_node, constraint);
		  if (BE (err != REG_NOERROR, 0))
		    return err;
		}
	    }
	  else
	    {
	      /* There are a duplicated node which satisfy the constraint,
		 use it to avoid infinite loop.  */
	      ret = re_node_set_insert (dfa->edests + clone_node, clone_dest);
	      if (BE (ret < 0, 0))
		return REG_ESPACE;
	    }

	  org_dest = dfa->edests[org_node].elems[1];
	  err = duplicate_node (&clone_dest, dfa, org_dest, constraint);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	  if (clone_dest == -1)
	    break;
	  ret = re_node_set_insert (dfa->edests + clone_node, clone_dest);
	  if (BE (ret < 0, 0))
	    return REG_ESPACE;
	}
      org_node = org_dest;
      clone_node = clone_dest;
    }
  return REG_NOERROR;
}

/* Search for a node which is duplicated from the node ORG_NODE, and
   satisfies the constraint CONSTRAINT.  */

static int
search_duplicated_node (dfa, org_node, constraint)
     re_dfa_t *dfa;
     int org_node;
     unsigned int constraint;
{
  int idx;
  for (idx = dfa->nodes_len - 1; dfa->nodes[idx].duplicated && idx > 0; --idx)
    {
      if (org_node == dfa->org_indices[idx]
	  && constraint == dfa->nodes[idx].constraint)
	return idx; /* Found.  */
    }
  return -1; /* Not found.  */
}

/* Duplicate the node whose index is ORG_IDX and set the constraint CONSTRAINT.
   The new index will be stored in NEW_IDX and return REG_NOERROR if succeeded,
   otherwise return the error code.  */

static reg_errcode_t
duplicate_node (new_idx, dfa, org_idx, constraint)
     re_dfa_t *dfa;
     int *new_idx, org_idx;
     unsigned int constraint;
{
  int dup_idx;

  if (dfa->nodes[org_idx].type == CHARACTER
      && (((constraint & NEXT_WORD_CONSTRAINT)
	   && !dfa->nodes[org_idx].word_char)
	  || ((constraint & NEXT_NOTWORD_CONSTRAINT)
	      && dfa->nodes[org_idx].word_char)))
    {
      /* \<!, \>W etc. can never match.  Don't duplicate them, instead
	 tell the caller they shouldn't be added to edests.  */
      *new_idx = -1;
      return REG_NOERROR;
    }

  dup_idx = re_dfa_add_node (dfa, dfa->nodes[org_idx], 1);
  if (BE (dup_idx == -1, 0))
    return REG_ESPACE;
  dfa->nodes[dup_idx].constraint = constraint;
  if (dfa->nodes[org_idx].type == ANCHOR)
    dfa->nodes[dup_idx].constraint |= dfa->nodes[org_idx].opr.ctx_type;
  dfa->nodes[dup_idx].duplicated = 1;
  re_node_set_init_empty (dfa->edests + dup_idx);
  re_node_set_init_empty (dfa->eclosures + dup_idx);
  re_node_set_init_empty (dfa->inveclosures + dup_idx);

  /* Store the index of the original node.  */
  dfa->org_indices[dup_idx] = org_idx;
  *new_idx = dup_idx;
  return REG_NOERROR;
}

static void
calc_inveclosure (dfa)
     re_dfa_t *dfa;
{
  int src, idx, dest;
  for (src = 0; src < dfa->nodes_len; ++src)
    {
      for (idx = 0; idx < dfa->eclosures[src].nelem; ++idx)
	{
	  dest = dfa->eclosures[src].elems[idx];
	  re_node_set_insert (dfa->inveclosures + dest, src);
	}
    }
}

/* Calculate "eclosure" for all the node in DFA.  */

static reg_errcode_t
calc_eclosure (dfa)
     re_dfa_t *dfa;
{
  int node_idx, incomplete;
#ifdef DEBUG
  assert (dfa->nodes_len > 0);
#endif
  incomplete = 0;
  /* For each nodes, calculate epsilon closure.  */
  for (node_idx = 0; ; ++node_idx)
    {
      reg_errcode_t err;
      re_node_set eclosure_elem;
      if (node_idx == dfa->nodes_len)
	{
	  if (!incomplete)
	    break;
	  incomplete = 0;
	  node_idx = 0;
	}

#ifdef DEBUG
      assert (dfa->eclosures[node_idx].nelem != -1);
#endif
      /* If we have already calculated, skip it.  */
      if (dfa->eclosures[node_idx].nelem != 0)
	continue;
      /* Calculate epsilon closure of `node_idx'.  */
      err = calc_eclosure_iter (&eclosure_elem, dfa, node_idx, 1);
      if (BE (err != REG_NOERROR, 0))
	return err;

      if (dfa->eclosures[node_idx].nelem == 0)
	{
	  incomplete = 1;
	  re_node_set_free (&eclosure_elem);
	}
    }
  return REG_NOERROR;
}

/* Calculate epsilon closure of NODE.  */

static reg_errcode_t
calc_eclosure_iter (new_set, dfa, node, root)
     re_node_set *new_set;
     re_dfa_t *dfa;
     int node, root;
{
  reg_errcode_t err;
  unsigned int constraint;
  int i, incomplete;
  re_node_set eclosure;
  incomplete = 0;
  err = re_node_set_alloc (&eclosure, dfa->edests[node].nelem + 1);
  if (BE (err != REG_NOERROR, 0))
    return err;

  /* This indicates that we are calculating this node now.
     We reference this value to avoid infinite loop.  */
  dfa->eclosures[node].nelem = -1;

  constraint = ((dfa->nodes[node].type == ANCHOR)
		? dfa->nodes[node].opr.ctx_type : 0);
  /* If the current node has constraints, duplicate all nodes.
     Since they must inherit the constraints.  */
  if (constraint && !dfa->nodes[dfa->edests[node].elems[0]].duplicated)
    {
      int org_node, cur_node;
      org_node = cur_node = node;
      err = duplicate_node_closure (dfa, node, node, node, constraint);
      if (BE (err != REG_NOERROR, 0))
	return err;
    }

  /* Expand each epsilon destination nodes.  */
  if (IS_EPSILON_NODE(dfa->nodes[node].type))
    for (i = 0; i < dfa->edests[node].nelem; ++i)
      {
	re_node_set eclosure_elem;
	int edest = dfa->edests[node].elems[i];
	/* If calculating the epsilon closure of `edest' is in progress,
	   return intermediate result.  */
	if (dfa->eclosures[edest].nelem == -1)
	  {
	    incomplete = 1;
	    continue;
	  }
	/* If we haven't calculated the epsilon closure of `edest' yet,
	   calculate now. Otherwise use calculated epsilon closure.  */
	if (dfa->eclosures[edest].nelem == 0)
	  {
	    err = calc_eclosure_iter (&eclosure_elem, dfa, edest, 0);
	    if (BE (err != REG_NOERROR, 0))
	      return err;
	  }
	else
	  eclosure_elem = dfa->eclosures[edest];
	/* Merge the epsilon closure of `edest'.  */
	re_node_set_merge (&eclosure, &eclosure_elem);
	/* If the epsilon closure of `edest' is incomplete,
	   the epsilon closure of this node is also incomplete.  */
	if (dfa->eclosures[edest].nelem == 0)
	  {
	    incomplete = 1;
	    re_node_set_free (&eclosure_elem);
	  }
      }

  /* Epsilon closures include itself.  */
  re_node_set_insert (&eclosure, node);
  if (incomplete && !root)
    dfa->eclosures[node].nelem = 0;
  else
    dfa->eclosures[node] = eclosure;
  *new_set = eclosure;
  return REG_NOERROR;
}

/* Functions for token which are used in the parser.  */

/* Fetch a token from INPUT.
   We must not use this function inside bracket expressions.  */

static void
fetch_token (result, input, syntax)
     re_token_t *result;
     re_string_t *input;
     reg_syntax_t syntax;
{
  re_string_skip_bytes (input, peek_token (result, input, syntax));
}

/* Peek a token from INPUT, and return the length of the token.
   We must not use this function inside bracket expressions.  */

static int
peek_token (token, input, syntax)
     re_token_t *token;
     re_string_t *input;
     reg_syntax_t syntax;
{
  unsigned char c;

  if (re_string_eoi (input))
    {
      token->type = END_OF_RE;
      return 0;
    }

  c = re_string_peek_byte (input, 0);
  token->opr.c = c;

  token->word_char = 0;
#ifdef RE_ENABLE_I18N
  token->mb_partial = 0;
  if (input->mb_cur_max > 1 &&
      !re_string_first_byte (input, re_string_cur_idx (input)))
    {
      token->type = CHARACTER;
      token->mb_partial = 1;
      return 1;
    }
#endif
  if (c == '\\')
    {
      unsigned char c2;
      if (re_string_cur_idx (input) + 1 >= re_string_length (input))
	{
	  token->type = BACK_SLASH;
	  return 1;
	}

      c2 = re_string_peek_byte_case (input, 1);
      token->opr.c = c2;
      token->type = CHARACTER;
#ifdef RE_ENABLE_I18N
      if (input->mb_cur_max > 1)
	{
	  wint_t wc = re_string_wchar_at (input,
					  re_string_cur_idx (input) + 1);
	  token->word_char = IS_WIDE_WORD_CHAR (wc) != 0;
	}
      else
#endif
	token->word_char = IS_WORD_CHAR (c2) != 0;

      switch (c2)
	{
	case '|':
	  if (!(syntax & RE_LIMITED_OPS) && !(syntax & RE_NO_BK_VBAR))
	    token->type = OP_ALT;
	  break;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
	  if (!(syntax & RE_NO_BK_REFS))
	    {
	      token->type = OP_BACK_REF;
	      token->opr.idx = c2 - '0';
	    }
	  break;
	case '<':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = WORD_FIRST;
	    }
	  break;
	case '>':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = WORD_LAST;
	    }
	  break;
	case 'b':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = WORD_DELIM;
	    }
	  break;
	case 'B':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = INSIDE_WORD;
	    }
	  break;
	case 'w':
	  if (!(syntax & RE_NO_GNU_OPS))
	    token->type = OP_WORD;
	  break;
	case 'W':
	  if (!(syntax & RE_NO_GNU_OPS))
	    token->type = OP_NOTWORD;
	  break;
	case 's':
	  if (!(syntax & RE_NO_GNU_OPS))
	    token->type = OP_SPACE;
	  break;
	case 'S':
	  if (!(syntax & RE_NO_GNU_OPS))
	    token->type = OP_NOTSPACE;
	  break;
	case '`':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = BUF_FIRST;
	    }
	  break;
	case '\'':
	  if (!(syntax & RE_NO_GNU_OPS))
	    {
	      token->type = ANCHOR;
	      token->opr.ctx_type = BUF_LAST;
	    }
	  break;
	case '(':
	  if (!(syntax & RE_NO_BK_PARENS))
	    token->type = OP_OPEN_SUBEXP;
	  break;
	case ')':
	  if (!(syntax & RE_NO_BK_PARENS))
	    token->type = OP_CLOSE_SUBEXP;
	  break;
	case '+':
	  if (!(syntax & RE_LIMITED_OPS) && (syntax & RE_BK_PLUS_QM))
	    token->type = OP_DUP_PLUS;
	  break;
	case '?':
	  if (!(syntax & RE_LIMITED_OPS) && (syntax & RE_BK_PLUS_QM))
	    token->type = OP_DUP_QUESTION;
	  break;
	case '{':
	  if ((syntax & RE_INTERVALS) && (!(syntax & RE_NO_BK_BRACES)))
	    token->type = OP_OPEN_DUP_NUM;
	  break;
	case '}':
	  if ((syntax & RE_INTERVALS) && (!(syntax & RE_NO_BK_BRACES)))
	    token->type = OP_CLOSE_DUP_NUM;
	  break;
	default:
	  break;
	}
      return 2;
    }

  token->type = CHARACTER;
#ifdef RE_ENABLE_I18N
  if (input->mb_cur_max > 1)
    {
      wint_t wc = re_string_wchar_at (input, re_string_cur_idx (input));
      token->word_char = IS_WIDE_WORD_CHAR (wc) != 0;
    }
  else
#endif
    token->word_char = IS_WORD_CHAR (token->opr.c);

  switch (c)
    {
    case '\n':
      if (syntax & RE_NEWLINE_ALT)
	token->type = OP_ALT;
      break;
    case '|':
      if (!(syntax & RE_LIMITED_OPS) && (syntax & RE_NO_BK_VBAR))
	token->type = OP_ALT;
      break;
    case '*':
      token->type = OP_DUP_ASTERISK;
      break;
    case '+':
      if (!(syntax & RE_LIMITED_OPS) && !(syntax & RE_BK_PLUS_QM))
	token->type = OP_DUP_PLUS;
      break;
    case '?':
      if (!(syntax & RE_LIMITED_OPS) && !(syntax & RE_BK_PLUS_QM))
	token->type = OP_DUP_QUESTION;
      break;
    case '{':
      if ((syntax & RE_INTERVALS) && (syntax & RE_NO_BK_BRACES))
	token->type = OP_OPEN_DUP_NUM;
      break;
    case '}':
      if ((syntax & RE_INTERVALS) && (syntax & RE_NO_BK_BRACES))
	token->type = OP_CLOSE_DUP_NUM;
      break;
    case '(':
      if (syntax & RE_NO_BK_PARENS)
	token->type = OP_OPEN_SUBEXP;
      break;
    case ')':
      if (syntax & RE_NO_BK_PARENS)
	token->type = OP_CLOSE_SUBEXP;
      break;
    case '[':
      token->type = OP_OPEN_BRACKET;
      break;
    case '.':
      token->type = OP_PERIOD;
      break;
    case '^':
      if (!(syntax & (RE_CONTEXT_INDEP_ANCHORS | RE_CARET_ANCHORS_HERE)) &&
	  re_string_cur_idx (input) != 0)
	{
	  char prev = re_string_peek_byte (input, -1);
	  if (!(syntax & RE_NEWLINE_ALT) || prev != '\n')
	    break;
	}
      token->type = ANCHOR;
      token->opr.ctx_type = LINE_FIRST;
      break;
    case '$':
      if (!(syntax & RE_CONTEXT_INDEP_ANCHORS) &&
	  re_string_cur_idx (input) + 1 != re_string_length (input))
	{
	  re_token_t next;
	  re_string_skip_bytes (input, 1);
	  peek_token (&next, input, syntax);
	  re_string_skip_bytes (input, -1);
	  if (next.type != OP_ALT && next.type != OP_CLOSE_SUBEXP)
	    break;
	}
      token->type = ANCHOR;
      token->opr.ctx_type = LINE_LAST;
      break;
    default:
      break;
    }
  return 1;
}

/* Peek a token from INPUT, and return the length of the token.
   We must not use this function out of bracket expressions.  */

static int
peek_token_bracket (token, input, syntax)
     re_token_t *token;
     re_string_t *input;
     reg_syntax_t syntax;
{
  unsigned char c;
  if (re_string_eoi (input))
    {
      token->type = END_OF_RE;
      return 0;
    }
  c = re_string_peek_byte (input, 0);
  token->opr.c = c;

#ifdef RE_ENABLE_I18N
  if (input->mb_cur_max > 1 &&
      !re_string_first_byte (input, re_string_cur_idx (input)))
    {
      token->type = CHARACTER;
      return 1;
    }
#endif /* RE_ENABLE_I18N */

  if (c == '\\' && (syntax & RE_BACKSLASH_ESCAPE_IN_LISTS))
    {
      /* In this case, '\' escape a character.  */
      unsigned char c2;
      re_string_skip_bytes (input, 1);
      c2 = re_string_peek_byte (input, 0);
      token->opr.c = c2;
      token->type = CHARACTER;
      return 1;
    }
  if (c == '[') /* '[' is a special char in a bracket exps.  */
    {
      unsigned char c2;
      int token_len;
      c2 = re_string_peek_byte (input, 1);
      token->opr.c = c2;
      token_len = 2;
      switch (c2)
	{
	case '.':
	  token->type = OP_OPEN_COLL_ELEM;
	  break;
	case '=':
	  token->type = OP_OPEN_EQUIV_CLASS;
	  break;
	case ':':
	  if (syntax & RE_CHAR_CLASSES)
	    {
	      token->type = OP_OPEN_CHAR_CLASS;
	      break;
	    }
	  /* else fall through.  */
	default:
	  token->type = CHARACTER;
	  token->opr.c = c;
	  token_len = 1;
	  break;
	}
      return token_len;
    }
  switch (c)
    {
    case '-':
      token->type = OP_CHARSET_RANGE;
      break;
    case ']':
      token->type = OP_CLOSE_BRACKET;
      break;
    case '^':
      token->type = OP_NON_MATCH_LIST;
      break;
    default:
      token->type = CHARACTER;
    }
  return 1;
}

/* Functions for parser.  */

/* Entry point of the parser.
   Parse the regular expression REGEXP and return the structure tree.
   If an error is occured, ERR is set by error code, and return NULL.
   This function build the following tree, from regular expression <reg_exp>:
	   CAT
	   / \
	  /   \
   <reg_exp>  EOR

   CAT means concatenation.
   EOR means end of regular expression.  */

static bin_tree_t *
parse (regexp, preg, syntax, err)
     re_string_t *regexp;
     regex_t *preg;
     reg_syntax_t syntax;
     reg_errcode_t *err;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  bin_tree_t *tree, *eor, *root;
  re_token_t current_token;
  fetch_token (&current_token, regexp, syntax | RE_CARET_ANCHORS_HERE);
  tree = parse_reg_exp (regexp, preg, &current_token, syntax, 0, err);
  if (BE (*err != REG_NOERROR && tree == NULL, 0))
    return NULL;
  eor = re_dfa_add_tree_node (dfa, NULL, NULL, &current_token);
  if (tree != NULL)
    root = create_tree (dfa, tree, eor, CONCAT, 0);
  else
    root = eor;
  if (BE (eor == NULL || root == NULL, 0))
    {
      *err = REG_ESPACE;
      return NULL;
    }
  return root;
}

/* This function build the following tree, from regular expression
   <branch1>|<branch2>:
	   ALT
	   / \
	  /   \
   <branch1> <branch2>

   ALT means alternative, which represents the operator `|'.  */

static bin_tree_t *
parse_reg_exp (regexp, preg, token, syntax, nest, err)
     re_string_t *regexp;
     regex_t *preg;
     re_token_t *token;
     reg_syntax_t syntax;
     int nest;
     reg_errcode_t *err;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  bin_tree_t *tree, *branch = NULL;
  tree = parse_branch (regexp, preg, token, syntax, nest, err);
  if (BE (*err != REG_NOERROR && tree == NULL, 0))
    return NULL;

  while (token->type == OP_ALT)
    {
      re_token_t alt_token = *token;
      fetch_token (token, regexp, syntax | RE_CARET_ANCHORS_HERE);
      if (token->type != OP_ALT && token->type != END_OF_RE
	  && (nest == 0 || token->type != OP_CLOSE_SUBEXP))
	{
	  branch = parse_branch (regexp, preg, token, syntax, nest, err);
	  if (BE (*err != REG_NOERROR && branch == NULL, 0))
	    return NULL;
	}
      else
	branch = NULL;
      tree = re_dfa_add_tree_node (dfa, tree, branch, &alt_token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
      dfa->has_plural_match = 1;
    }
  return tree;
}

/* This function build the following tree, from regular expression
   <exp1><exp2>:
	CAT
	/ \
       /   \
   <exp1> <exp2>

   CAT means concatenation.  */

static bin_tree_t *
parse_branch (regexp, preg, token, syntax, nest, err)
     re_string_t *regexp;
     regex_t *preg;
     re_token_t *token;
     reg_syntax_t syntax;
     int nest;
     reg_errcode_t *err;
{
  bin_tree_t *tree, *exp;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  tree = parse_expression (regexp, preg, token, syntax, nest, err);
  if (BE (*err != REG_NOERROR && tree == NULL, 0))
    return NULL;

  while (token->type != OP_ALT && token->type != END_OF_RE
	 && (nest == 0 || token->type != OP_CLOSE_SUBEXP))
    {
      exp = parse_expression (regexp, preg, token, syntax, nest, err);
      if (BE (*err != REG_NOERROR && exp == NULL, 0))
	{
	  return NULL;
	}
      if (tree != NULL && exp != NULL)
	{
	  tree = create_tree (dfa, tree, exp, CONCAT, 0);
	  if (tree == NULL)
	    {
	      *err = REG_ESPACE;
	      return NULL;
	    }
	}
      else if (tree == NULL)
	tree = exp;
      /* Otherwise exp == NULL, we don't need to create new tree.  */
    }
  return tree;
}

/* This function build the following tree, from regular expression a*:
	 *
	 |
	 a
*/

static bin_tree_t *
parse_expression (regexp, preg, token, syntax, nest, err)
     re_string_t *regexp;
     regex_t *preg;
     re_token_t *token;
     reg_syntax_t syntax;
     int nest;
     reg_errcode_t *err;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  bin_tree_t *tree;
  switch (token->type)
    {
    case CHARACTER:
      tree = re_dfa_add_tree_node (dfa, NULL, NULL, token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
#ifdef RE_ENABLE_I18N
      if (dfa->mb_cur_max > 1)
	{
	  while (!re_string_eoi (regexp)
		 && !re_string_first_byte (regexp, re_string_cur_idx (regexp)))
	    {
	      bin_tree_t *mbc_remain;
	      fetch_token (token, regexp, syntax);
	      mbc_remain = re_dfa_add_tree_node (dfa, NULL, NULL, token);
	      tree = create_tree (dfa, tree, mbc_remain, CONCAT, 0);
	      if (BE (mbc_remain == NULL || tree == NULL, 0))
		{
		  *err = REG_ESPACE;
		  return NULL;
		}
	    }
	}
#endif
      break;
    case OP_OPEN_SUBEXP:
      tree = parse_sub_exp (regexp, preg, token, syntax, nest + 1, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_OPEN_BRACKET:
      tree = parse_bracket_exp (regexp, dfa, token, syntax, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_BACK_REF:
      if (BE (preg->re_nsub < token->opr.idx
	      || dfa->subexps[token->opr.idx - 1].end == -1, 0))
	{
	  *err = REG_ESUBREG;
	  return NULL;
	}
      dfa->used_bkref_map |= 1 << (token->opr.idx - 1);
      tree = re_dfa_add_tree_node (dfa, NULL, NULL, token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
      ++dfa->nbackref;
      dfa->has_mb_node = 1;
      break;
    case OP_OPEN_DUP_NUM:
      if (syntax & RE_CONTEXT_INVALID_DUP)
	{
	  *err = REG_BADRPT;
	  return NULL;
	}
      /* FALLTHROUGH */
    case OP_DUP_ASTERISK:
    case OP_DUP_PLUS:
    case OP_DUP_QUESTION:
      if (syntax & RE_CONTEXT_INVALID_OPS)
	{
	  *err = REG_BADRPT;
	  return NULL;
	}
      else if (syntax & RE_CONTEXT_INDEP_OPS)
	{
	  fetch_token (token, regexp, syntax);
	  return parse_expression (regexp, preg, token, syntax, nest, err);
	}
      /* else fall through  */
    case OP_CLOSE_SUBEXP:
      if ((token->type == OP_CLOSE_SUBEXP) &&
	  !(syntax & RE_UNMATCHED_RIGHT_PAREN_ORD))
	{
	  *err = REG_ERPAREN;
	  return NULL;
	}
      /* else fall through  */
    case OP_CLOSE_DUP_NUM:
      /* We treat it as a normal character.  */

      /* Then we can these characters as normal characters.  */
      token->type = CHARACTER;
      /* mb_partial and word_char bits should be initialized already
	 by peek_token.  */
      tree = re_dfa_add_tree_node (dfa, NULL, NULL, token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
      break;
    case ANCHOR:
      if ((token->opr.ctx_type
	   & (WORD_DELIM | INSIDE_WORD | WORD_FIRST | WORD_LAST))
	  && dfa->word_char == NULL)
	{
	  *err = init_word_char (dfa);
	  if (BE (*err != REG_NOERROR, 0))
	    return NULL;
	}
      if (token->opr.ctx_type == WORD_DELIM)
	{
	  bin_tree_t *tree_first, *tree_last;
	  token->opr.ctx_type = WORD_FIRST;
	  tree_first = re_dfa_add_tree_node (dfa, NULL, NULL, token);
	  token->opr.ctx_type = WORD_LAST;
	  tree_last = re_dfa_add_tree_node (dfa, NULL, NULL, token);
	  token->type = OP_ALT;
	  tree = re_dfa_add_tree_node (dfa, tree_first, tree_last, token);
	  if (BE (tree_first == NULL || tree_last == NULL || tree == NULL, 0))
	    {
	      *err = REG_ESPACE;
	      return NULL;
	    }
	}
      else
	{
	  tree = re_dfa_add_tree_node (dfa, NULL, NULL, token);
	  if (BE (tree == NULL, 0))
	    {
	      *err = REG_ESPACE;
	      return NULL;
	    }
	}
      /* We must return here, since ANCHORs can't be followed
	 by repetition operators.
	 eg. RE"^*" is invalid or "<ANCHOR(^)><CHAR(*)>",
	     it must not be "<ANCHOR(^)><REPEAT(*)>".  */
      fetch_token (token, regexp, syntax);
      return tree;
    case OP_PERIOD:
      tree = re_dfa_add_tree_node (dfa, NULL, NULL, token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
      if (dfa->mb_cur_max > 1)
	dfa->has_mb_node = 1;
      break;
    case OP_WORD:
      tree = build_charclass_op (dfa, regexp->trans,
				 (unsigned char *) "alnum",
				 (unsigned char *) "_", 0, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_NOTWORD:
      tree = build_charclass_op (dfa, regexp->trans,
				 (unsigned char *) "alnum",
				 (unsigned char *) "_", 1, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_SPACE:
      tree = build_charclass_op (dfa, regexp->trans,
				 (unsigned char *) "space",
				 (unsigned char *) "", 0, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_NOTSPACE:
      tree = build_charclass_op (dfa, regexp->trans,
				 (unsigned char *) "space",
				 (unsigned char *) "", 1, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      break;
    case OP_ALT:
    case END_OF_RE:
      return NULL;
    case BACK_SLASH:
      *err = REG_EESCAPE;
      return NULL;
    default:
      /* Must not happen?  */
#ifdef DEBUG
      assert (0);
#endif
      return NULL;
    }
  fetch_token (token, regexp, syntax);

  while (token->type == OP_DUP_ASTERISK || token->type == OP_DUP_PLUS
	 || token->type == OP_DUP_QUESTION || token->type == OP_OPEN_DUP_NUM)
    {
      tree = parse_dup_op (tree, regexp, dfa, token, syntax, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
      /* In BRE consecutive duplications are not allowed.  */
      if ((syntax & RE_CONTEXT_INVALID_DUP)
	  && (token->type == OP_DUP_ASTERISK
	      || token->type == OP_OPEN_DUP_NUM))
	{
	  *err = REG_BADRPT;
	  return NULL;
	}
      dfa->has_plural_match = 1;
    }

  return tree;
}

/* This function build the following tree, from regular expression
   (<reg_exp>):
	 SUBEXP
	    |
	<reg_exp>
*/

static bin_tree_t *
parse_sub_exp (regexp, preg, token, syntax, nest, err)
     re_string_t *regexp;
     regex_t *preg;
     re_token_t *token;
     reg_syntax_t syntax;
     int nest;
     reg_errcode_t *err;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  bin_tree_t *tree, *left_par, *right_par;
  size_t cur_nsub;
  cur_nsub = preg->re_nsub++;
  if (BE (dfa->subexps_alloc < preg->re_nsub, 0))
    {
      re_subexp_t *new_array;
      dfa->subexps_alloc *= 2;
      new_array = re_realloc (dfa->subexps, re_subexp_t, dfa->subexps_alloc);
      if (BE (new_array == NULL, 0))
	{
	  dfa->subexps_alloc /= 2;
	  *err = REG_ESPACE;
	  return NULL;
	}
      dfa->subexps = new_array;
    }
  dfa->subexps[cur_nsub].start = dfa->nodes_len;
  dfa->subexps[cur_nsub].end = -1;

  left_par = re_dfa_add_tree_node (dfa, NULL, NULL, token);
  if (BE (left_par == NULL, 0))
    {
      *err = REG_ESPACE;
      return NULL;
    }
  dfa->nodes[left_par->node_idx].opr.idx = cur_nsub;
  fetch_token (token, regexp, syntax | RE_CARET_ANCHORS_HERE);

  /* The subexpression may be a null string.  */
  if (token->type == OP_CLOSE_SUBEXP)
    tree = NULL;
  else
    {
      tree = parse_reg_exp (regexp, preg, token, syntax, nest, err);
      if (BE (*err != REG_NOERROR && tree == NULL, 0))
	return NULL;
    }
  if (BE (token->type != OP_CLOSE_SUBEXP, 0))
    {
      *err = REG_EPAREN;
      return NULL;
    }
  right_par = re_dfa_add_tree_node (dfa, NULL, NULL, token);
  dfa->subexps[cur_nsub].end = dfa->nodes_len;
  tree = ((tree == NULL) ? right_par
	  : create_tree (dfa, tree, right_par, CONCAT, 0));
  tree = create_tree (dfa, left_par, tree, CONCAT, 0);
  if (BE (right_par == NULL || tree == NULL, 0))
    {
      *err = REG_ESPACE;
      return NULL;
    }
  dfa->nodes[right_par->node_idx].opr.idx = cur_nsub;

  return tree;
}

/* This function parse repetition operators like "*", "+", "{1,3}" etc.  */

static bin_tree_t *
parse_dup_op (dup_elem, regexp, dfa, token, syntax, err)
     bin_tree_t *dup_elem;
     re_string_t *regexp;
     re_dfa_t *dfa;
     re_token_t *token;
     reg_syntax_t syntax;
     reg_errcode_t *err;
{
  re_token_t dup_token;
  bin_tree_t *tree = dup_elem, *work_tree;
  int start_idx = re_string_cur_idx (regexp);
  re_token_t start_token = *token;
  if (token->type == OP_OPEN_DUP_NUM)
    {
      int i;
      int end = 0;
      int start = fetch_number (regexp, token, syntax);
      bin_tree_t *elem;
      if (start == -1)
	{
	  if (token->type == CHARACTER && token->opr.c == ',')
	    start = 0; /* We treat "{,m}" as "{0,m}".  */
	  else
	    {
	      *err = REG_BADBR; /* <re>{} is invalid.  */
	      return NULL;
	    }
	}
      if (BE (start != -2, 1))
	{
	  /* We treat "{n}" as "{n,n}".  */
	  end = ((token->type == OP_CLOSE_DUP_NUM) ? start
		 : ((token->type == CHARACTER && token->opr.c == ',')
		    ? fetch_number (regexp, token, syntax) : -2));
	}
      if (BE (start == -2 || end == -2, 0))
	{
	  /* Invalid sequence.  */
	  if (token->type == OP_CLOSE_DUP_NUM)
	    goto parse_dup_op_invalid_interval;
	  else
	    goto parse_dup_op_ebrace;
	}
      if (BE (start == 0 && end == 0, 0))
	{
	  /* We treat "<re>{0}" and "<re>{0,0}" as null string.  */
	  fetch_token (token, regexp, syntax);
	  return NULL;
	}

      /* Extract "<re>{n,m}" to "<re><re>...<re><re>{0,<m-n>}".  */
      elem = tree;
      for (i = 0; i < start; ++i)
	if (i != 0)
	  {
	    work_tree = duplicate_tree (elem, dfa);
	    tree = create_tree (dfa, tree, work_tree, CONCAT, 0);
	    if (BE (work_tree == NULL || tree == NULL, 0))
	      goto parse_dup_op_espace;
	  }

      if (end == -1)
	{
	  /* We treat "<re>{0,}" as "<re>*".  */
	  dup_token.type = OP_DUP_ASTERISK;
	  if (start > 0)
	    {
	      elem = duplicate_tree (elem, dfa);
	      work_tree = re_dfa_add_tree_node (dfa, elem, NULL, &dup_token);
	      tree = create_tree (dfa, tree, work_tree, CONCAT, 0);
	      if (BE (elem == NULL || work_tree == NULL || tree == NULL, 0))
		goto parse_dup_op_espace;
	    }
	  else
	    {
	      tree = re_dfa_add_tree_node (dfa, elem, NULL, &dup_token);
	      if (BE (tree == NULL, 0))
		goto parse_dup_op_espace;
	    }
	}
      else if (BE (start > end, 0))
	{
	  /* First  number greater than first.  */
	  *err = REG_BADBR;
	  return NULL;
	}
      else if (end - start > 0)
	{
	  /* Then extract "<re>{0,m}" to "<re>?<re>?...<re>?".  */
	  dup_token.type = OP_DUP_QUESTION;
	  if (start > 0)
	    {
	      elem = duplicate_tree (elem, dfa);
	      elem = re_dfa_add_tree_node (dfa, elem, NULL, &dup_token);
	      tree = create_tree (dfa, tree, elem, CONCAT, 0);
	      if (BE (elem == NULL || tree == NULL, 0))
		goto parse_dup_op_espace;
	    }
	  else
	    {
	      tree = elem = re_dfa_add_tree_node (dfa, elem, NULL, &dup_token);
	      if (BE (tree == NULL, 0))
		goto parse_dup_op_espace;
	    }
	  for (i = 1; i < end - start; ++i)
	    {
	      work_tree = duplicate_tree (elem, dfa);
	      tree = create_tree (dfa, tree, work_tree, CONCAT, 0);
	      if (BE (work_tree == NULL || tree == NULL, 0))
		{
		  *err = REG_ESPACE;
		  return NULL;
		}
	    }
	}
    }
  else
    {
      tree = re_dfa_add_tree_node (dfa, tree, NULL, token);
      if (BE (tree == NULL, 0))
	{
	  *err = REG_ESPACE;
	  return NULL;
	}
    }
  fetch_token (token, regexp, syntax);
  return tree;

 parse_dup_op_espace:
  *err = REG_ESPACE;
  return NULL;

 parse_dup_op_ebrace:
  if (BE (!(syntax & RE_INVALID_INTERVAL_ORD), 0))
    {
      *err = REG_EBRACE;
      return NULL;
    }
  goto parse_dup_op_rollback;
 parse_dup_op_invalid_interval:
  if (BE (!(syntax & RE_INVALID_INTERVAL_ORD), 0))
    {
      *err = REG_BADBR;
      return NULL;
    }
 parse_dup_op_rollback:
  re_string_set_index (regexp, start_idx);
  *token = start_token;
  token->type = CHARACTER;
  /* mb_partial and word_char bits should be already initialized by
     peek_token.  */
  return dup_elem;
}

/* Size of the names for collating symbol/equivalence_class/character_class.
   I'm not sure, but maybe enough.  */
#define BRACKET_NAME_BUF_SIZE 32

#ifndef _LIBC
  /* Local function for parse_bracket_exp only used in case of NOT _LIBC.
     Build the range expression which starts from START_ELEM, and ends
     at END_ELEM.  The result are written to MBCSET and SBCSET.
     RANGE_ALLOC is the allocated size of mbcset->range_starts, and
     mbcset->range_ends, is a pointer argument sinse we may
     update it.  */

static reg_errcode_t
# ifdef RE_ENABLE_I18N
build_range_exp (sbcset, mbcset, range_alloc, start_elem, end_elem)
     re_charset_t *mbcset;
     int *range_alloc;
# else /* not RE_ENABLE_I18N */
build_range_exp (sbcset, start_elem, end_elem)
# endif /* not RE_ENABLE_I18N */
     re_bitset_ptr_t sbcset;
     bracket_elem_t *start_elem, *end_elem;
{
  unsigned int start_ch, end_ch;
  /* Equivalence Classes and Character Classes can't be a range start/end.  */
  if (BE (start_elem->type == EQUIV_CLASS || start_elem->type == CHAR_CLASS
	  || end_elem->type == EQUIV_CLASS || end_elem->type == CHAR_CLASS,
	  0))
    return REG_ERANGE;

  /* We can handle no multi character collating elements without libc
     support.  */
  if (BE ((start_elem->type == COLL_SYM
	   && strlen ((char *) start_elem->opr.name) > 1)
	  || (end_elem->type == COLL_SYM
	      && strlen ((char *) end_elem->opr.name) > 1), 0))
    return REG_ECOLLATE;

# ifdef RE_ENABLE_I18N
  {
    wchar_t wc, start_wc, end_wc;
    wchar_t cmp_buf[6] = {L'\0', L'\0', L'\0', L'\0', L'\0', L'\0'};

    start_ch = ((start_elem->type == SB_CHAR) ? start_elem->opr.ch
		: ((start_elem->type == COLL_SYM) ? start_elem->opr.name[0]
		   : 0));
    end_ch = ((end_elem->type == SB_CHAR) ? end_elem->opr.ch
	      : ((end_elem->type == COLL_SYM) ? end_elem->opr.name[0]
		 : 0));
    start_wc = ((start_elem->type == SB_CHAR || start_elem->type == COLL_SYM)
		? __btowc (start_ch) : start_elem->opr.wch);
    end_wc = ((end_elem->type == SB_CHAR || end_elem->type == COLL_SYM)
	      ? __btowc (end_ch) : end_elem->opr.wch);
    cmp_buf[0] = start_wc;
    cmp_buf[4] = end_wc;
    if (wcscoll (cmp_buf, cmp_buf + 4) > 0)
      return REG_ERANGE;

    /* Check the space of the arrays.  */
    if (BE (*range_alloc == mbcset->nranges, 0))
      {
	/* There are not enough space, need realloc.  */
	wchar_t *new_array_start, *new_array_end;
	int new_nranges;

	/* +1 in case of mbcset->nranges is 0.  */
	new_nranges = 2 * mbcset->nranges + 1;
	/* Use realloc since mbcset->range_starts and mbcset->range_ends
	   are NULL if *range_alloc == 0.  */
	new_array_start = re_realloc (mbcset->range_starts, wchar_t,
				      new_nranges);
	new_array_end = re_realloc (mbcset->range_ends, wchar_t,
				    new_nranges);

	if (BE (new_array_start == NULL || new_array_end == NULL, 0))
	  return REG_ESPACE;

	mbcset->range_starts = new_array_start;
	mbcset->range_ends = new_array_end;
	*range_alloc = new_nranges;
      }

    mbcset->range_starts[mbcset->nranges] = start_wc;
    mbcset->range_ends[mbcset->nranges++] = end_wc;

    /* Build the table for single byte characters.  */
    for (wc = 0; wc <= SBC_MAX; ++wc)
      {
	cmp_buf[2] = wc;
	if (wcscoll (cmp_buf, cmp_buf + 2) <= 0
	    && wcscoll (cmp_buf + 2, cmp_buf + 4) <= 0)
	  bitset_set (sbcset, wc);
      }
  }
# else /* not RE_ENABLE_I18N */
  {
    unsigned int ch;
    start_ch = ((start_elem->type == SB_CHAR ) ? start_elem->opr.ch
		: ((start_elem->type == COLL_SYM) ? start_elem->opr.name[0]
		   : 0));
    end_ch = ((end_elem->type == SB_CHAR ) ? end_elem->opr.ch
	      : ((end_elem->type == COLL_SYM) ? end_elem->opr.name[0]
		 : 0));
    if (start_ch > end_ch)
      return REG_ERANGE;
    /* Build the table for single byte characters.  */
    for (ch = 0; ch <= SBC_MAX; ++ch)
      if (start_ch <= ch  && ch <= end_ch)
	bitset_set (sbcset, ch);
  }
# endif /* not RE_ENABLE_I18N */
  return REG_NOERROR;
}
#endif /* not _LIBC */

#ifndef _LIBC
/* Helper function for parse_bracket_exp only used in case of NOT _LIBC..
   Build the collating element which is represented by NAME.
   The result are written to MBCSET and SBCSET.
   COLL_SYM_ALLOC is the allocated size of mbcset->coll_sym, is a
   pointer argument since we may update it.  */

static reg_errcode_t
# ifdef RE_ENABLE_I18N
build_collating_symbol (sbcset, mbcset, coll_sym_alloc, name)
     re_charset_t *mbcset;
     int *coll_sym_alloc;
# else /* not RE_ENABLE_I18N */
build_collating_symbol (sbcset, name)
# endif /* not RE_ENABLE_I18N */
     re_bitset_ptr_t sbcset;
     const unsigned char *name;
{
  size_t name_len = strlen ((const char *) name);
  if (BE (name_len != 1, 0))
    return REG_ECOLLATE;
  else
    {
      bitset_set (sbcset, name[0]);
      return REG_NOERROR;
    }
}
#endif /* not _LIBC */

/* This function parse bracket expression like "[abc]", "[a-c]",
   "[[.a-a.]]" etc.  */

static bin_tree_t *
parse_bracket_exp (regexp, dfa, token, syntax, err)
     re_string_t *regexp;
     re_dfa_t *dfa;
     re_token_t *token;
     reg_syntax_t syntax;
     reg_errcode_t *err;
{
#ifdef _LIBC
  const unsigned char *collseqmb;
  const char *collseqwc;
  uint32_t nrules;
  int32_t table_size;
  const int32_t *symb_table;
  const unsigned char *extra;

  /* Local function for parse_bracket_exp used in _LIBC environement.
     Seek the collating symbol entry correspondings to NAME.
     Return the index of the symbol in the SYMB_TABLE.  */

  static R_INLINE int32_t
  __attribute ((always_inline))
  seek_collating_symbol_entry (name, name_len)
	 const unsigned char *name;
	 size_t name_len;
    {
      int32_t hash = elem_hash ((const char *) name, name_len);
      int32_t elem = hash % table_size;
      int32_t second = hash % (table_size - 2);
      while (symb_table[2 * elem] != 0)
	{
	  /* First compare the hashing value.  */
	  if (symb_table[2 * elem] == hash
	      /* Compare the length of the name.  */
	      && name_len == extra[symb_table[2 * elem + 1]]
	      /* Compare the name.  */
	      && memcmp (name, &extra[symb_table[2 * elem + 1] + 1],
			 name_len) == 0)
	    {
	      /* Yep, this is the entry.  */
	      break;
	    }

	  /* Next entry.  */
	  elem += second;
	}
      return elem;
    }

  /* Local function for parse_bracket_exp used in _LIBC environement.
     Look up the collation sequence value of BR_ELEM.
     Return the value if succeeded, UINT_MAX otherwise.  */

  static R_INLINE unsigned int
  __attribute ((always_inline))
  lookup_collation_sequence_value (br_elem)
	 bracket_elem_t *br_elem;
    {
      if (br_elem->type == SB_CHAR)
	{
	  /*
	  if (MB_CUR_MAX == 1)
	  */
	  if (nrules == 0)
	    return collseqmb[br_elem->opr.ch];
	  else
	    {
	      wint_t wc = __btowc (br_elem->opr.ch);
	      return __collseq_table_lookup (collseqwc, wc);
	    }
	}
      else if (br_elem->type == MB_CHAR)
	{
	  return __collseq_table_lookup (collseqwc, br_elem->opr.wch);
	}
      else if (br_elem->type == COLL_SYM)
	{
	  size_t sym_name_len = strlen ((char *) br_elem->opr.name);
	  if (nrules != 0)
	    {
	      int32_t elem, idx;
	      elem = seek_collating_symbol_entry (br_elem->opr.name,
						  sym_name_len);
	      if (symb_table[2 * elem] != 0)
		{
		  /* We found the entry.  */
		  idx = symb_table[2 * elem + 1];
		  /* Skip the name of collating element name.  */
		  idx += 1 + extra[idx];
		  /* Skip the byte sequence of the collating element.  */
		  idx += 1 + extra[idx];
		  /* Adjust for the alignment.  */
		  idx = (idx + 3) & ~3;
		  /* Skip the multibyte collation sequence value.  */
		  idx += sizeof (unsigned int);
		  /* Skip the wide char sequence of the collating element.  */
		  idx += sizeof (unsigned int) *
		    (1 + *(unsigned int *) (extra + idx));
		  /* Return the collation sequence value.  */
		  return *(unsigned int *) (extra + idx);
		}
	      else if (symb_table[2 * elem] == 0 && sym_name_len == 1)
		{
		  /* No valid character.  Match it as a single byte
		     character.  */
		  return collseqmb[br_elem->opr.name[0]];
		}
	    }
	  else if (sym_name_len == 1)
	    return collseqmb[br_elem->opr.name[0]];
	}
      return UINT_MAX;
    }

  /* Local function for parse_bracket_exp used in _LIBC environement.
     Build the range expression which starts from START_ELEM, and ends
     at END_ELEM.  The result are written to MBCSET and SBCSET.
     RANGE_ALLOC is the allocated size of mbcset->range_starts, and
     mbcset->range_ends, is a pointer argument sinse we may
     update it.  */

  static R_INLINE reg_errcode_t
  __attribute ((always_inline))
# ifdef RE_ENABLE_I18N
  build_range_exp (sbcset, mbcset, range_alloc, start_elem, end_elem)
	 re_charset_t *mbcset;
	 int *range_alloc;
# else /* not RE_ENABLE_I18N */
  build_range_exp (sbcset, start_elem, end_elem)
# endif /* not RE_ENABLE_I18N */
	 re_bitset_ptr_t sbcset;
	 bracket_elem_t *start_elem, *end_elem;
    {
      unsigned int ch;
      uint32_t start_collseq;
      uint32_t end_collseq;

# ifdef RE_ENABLE_I18N
      /* Check the space of the arrays.  */
      if (BE (*range_alloc == mbcset->nranges, 0))
	{
	  /* There are not enough space, need realloc.  */
	  uint32_t *new_array_start;
	  uint32_t *new_array_end;
	  int new_nranges;

	  /* +1 in case of mbcset->nranges is 0.  */
	  new_nranges = 2 * mbcset->nranges + 1;
	  /* Use realloc since mbcset->range_starts and mbcset->range_ends
	     are NULL if *range_alloc == 0.  */
	  new_array_start = re_realloc (mbcset->range_starts, uint32_t,
					new_nranges);
	  new_array_end = re_realloc (mbcset->range_ends, uint32_t,
				      new_nranges);

	  if (BE (new_array_start == NULL || new_array_end == NULL, 0))
	    return REG_ESPACE;

	  mbcset->range_starts = new_array_start;
	  mbcset->range_ends = new_array_end;
	  *range_alloc = new_nranges;
	}
# endif /* RE_ENABLE_I18N */

      /* Equivalence Classes and Character Classes can't be a range
	 start/end.  */
      if (BE (start_elem->type == EQUIV_CLASS || start_elem->type == CHAR_CLASS
	      || end_elem->type == EQUIV_CLASS || end_elem->type == CHAR_CLASS,
	      0))
	return REG_ERANGE;

      start_collseq = lookup_collation_sequence_value (start_elem);
      end_collseq = lookup_collation_sequence_value (end_elem);
      /* Check start/end collation sequence values.  */
      if (BE (start_collseq == UINT_MAX || end_collseq == UINT_MAX, 0))
	return REG_ECOLLATE;
      if (BE ((syntax & RE_NO_EMPTY_RANGES) && start_collseq > end_collseq, 0))
	return REG_ERANGE;

# ifdef RE_ENABLE_I18N
      /* Got valid collation sequence values, add them as a new entry.  */
      mbcset->range_starts[mbcset->nranges] = start_collseq;
      mbcset->range_ends[mbcset->nranges++] = end_collseq;
# endif /* RE_ENABLE_I18N */

      /* Build the table for single byte characters.  */
      for (ch = 0; ch <= SBC_MAX; ch++)
	{
	  uint32_t ch_collseq;
	  /*
	  if (MB_CUR_MAX == 1)
	  */
	  if (nrules == 0)
	    ch_collseq = collseqmb[ch];
	  else
	    ch_collseq = __collseq_table_lookup (collseqwc, __btowc (ch));
	  if (start_collseq <= ch_collseq && ch_collseq <= end_collseq)
	    bitset_set (sbcset, ch);
	}
      return REG_NOERROR;
    }

  /* Local function for parse_bracket_exp used in _LIBC environement.
     Build the collating element which is represented by NAME.
     The result are written to MBCSET and SBCSET.
     COLL_SYM_ALLOC is the allocated size of mbcset->coll_sym, is a
     pointer argument sinse we may update it.  */

  static R_INLINE reg_errcode_t
  __attribute ((always_inline))
# ifdef RE_ENABLE_I18N
  build_collating_symbol (sbcset, mbcset, coll_sym_alloc, name)
	 re_charset_t *mbcset;
	 int *coll_sym_alloc;
# else /* not RE_ENABLE_I18N */
  build_collating_symbol (sbcset, name)
# endif /* not RE_ENABLE_I18N */
	 re_bitset_ptr_t sbcset;
	 const unsigned char *name;
    {
      int32_t elem, idx;
      size_t name_len = strlen ((const char *) name);
      if (nrules != 0)
	{
	  elem = seek_collating_symbol_entry (name, name_len);
	  if (symb_table[2 * elem] != 0)
	    {
	      /* We found the entry.  */
	      idx = symb_table[2 * elem + 1];
	      /* Skip the name of collating element name.  */
	      idx += 1 + extra[idx];
	    }
	  else if (symb_table[2 * elem] == 0 && name_len == 1)
	    {
	      /* No valid character, treat it as a normal
		 character.  */
	      bitset_set (sbcset, name[0]);
	      return REG_NOERROR;
	    }
	  else
	    return REG_ECOLLATE;

# ifdef RE_ENABLE_I18N
	  /* Got valid collation sequence, add it as a new entry.  */
	  /* Check the space of the arrays.  */
	  if (BE (*coll_sym_alloc == mbcset->ncoll_syms, 0))
	    {
	      /* Not enough, realloc it.  */
	      /* +1 in case of mbcset->ncoll_syms is 0.  */
	      int new_coll_sym_alloc = 2 * mbcset->ncoll_syms + 1;
	      /* Use realloc since mbcset->coll_syms is NULL
		 if *alloc == 0.  */
	      int32_t *new_coll_syms = re_realloc (mbcset->coll_syms, int32_t,
						   new_coll_sym_alloc);
	      if (BE (new_coll_syms == NULL, 0))
		return REG_ESPACE;
	      mbcset->coll_syms = new_coll_syms;
	      *coll_sym_alloc = new_coll_sym_alloc;
	    }
	  mbcset->coll_syms[mbcset->ncoll_syms++] = idx;
# endif /* RE_ENABLE_I18N */
	  return REG_NOERROR;
	}
      else
	{
	  if (BE (name_len != 1, 0))
	    return REG_ECOLLATE;
	  else
	    {
	      bitset_set (sbcset, name[0]);
	      return REG_NOERROR;
	    }
	}
    }
#endif

  re_token_t br_token;
  re_bitset_ptr_t sbcset;
#ifdef RE_ENABLE_I18N
  re_charset_t *mbcset;
  int coll_sym_alloc = 0, range_alloc = 0, mbchar_alloc = 0;
  int equiv_class_alloc = 0, char_class_alloc = 0;
#else /* not RE_ENABLE_I18N */
  int non_match = 0;
#endif /* not RE_ENABLE_I18N */
  bin_tree_t *work_tree;
  int token_len;
  int first_round = 1; 
#ifdef _LIBC
  collseqmb = (const unsigned char *)
    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQMB);
  nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
  if (nrules)
    {
      /*
      if (MB_CUR_MAX > 1)
      */
	collseqwc = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQWC);
      table_size = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_SYMB_HASH_SIZEMB);
      symb_table = (const int32_t *) _NL_CURRENT (LC_COLLATE,
						  _NL_COLLATE_SYMB_TABLEMB);
      extra = (const unsigned char *) _NL_CURRENT (LC_COLLATE,
						   _NL_COLLATE_SYMB_EXTRAMB);
    }
#endif
  sbcset = (re_bitset_ptr_t) calloc (sizeof (unsigned int), BITSET_UINTS);
#ifdef RE_ENABLE_I18N
  mbcset = (re_charset_t *) calloc (sizeof (re_charset_t), 1);
#endif /* RE_ENABLE_I18N */
#ifdef RE_ENABLE_I18N
  if (BE (sbcset == NULL || mbcset == NULL, 0))
#else
  if (BE (sbcset == NULL, 0))
#endif /* RE_ENABLE_I18N */
    {
      *err = REG_ESPACE;
      return NULL;
    }

  token_len = peek_token_bracket (token, regexp, syntax);
  if (BE (token->type == END_OF_RE, 0))
    {
      *err = REG_BADPAT;
      goto parse_bracket_exp_free_return;
    }
  if (token->type == OP_NON_MATCH_LIST)
    {
#ifdef RE_ENABLE_I18N
      mbcset->non_match = 1;
#else /* not RE_ENABLE_I18N */
      non_match = 1;
#endif /* not RE_ENABLE_I18N */
      if (syntax & RE_HAT_LISTS_NOT_NEWLINE)
	bitset_set (sbcset, '\0');
      re_string_skip_bytes (regexp, token_len); /* Skip a token.  */
      token_len = peek_token_bracket (token, regexp, syntax);
      if (BE (token->type == END_OF_RE, 0))
	{
	  *err = REG_BADPAT;
	  goto parse_bracket_exp_free_return;
	}
    }

  /* We treat the first ']' as a normal character.  */
  if (token->type == OP_CLOSE_BRACKET)
    token->type = CHARACTER;

  /* int first_round = 1; moved up */
  while (1)
    {
      bracket_elem_t start_elem, end_elem;
      unsigned char start_name_buf[BRACKET_NAME_BUF_SIZE];
      unsigned char end_name_buf[BRACKET_NAME_BUF_SIZE];
      reg_errcode_t ret;
      int token_len2 = 0, is_range_exp = 0;
      re_token_t token2;

      start_elem.opr.name = start_name_buf;
      ret = parse_bracket_element (&start_elem, regexp, token, token_len, dfa,
				   syntax, first_round);
      if (BE (ret != REG_NOERROR, 0))
	{
	  *err = ret;
	  goto parse_bracket_exp_free_return;
	}
      first_round = 0;

      /* Get information about the next token.  We need it in any case.  */
      token_len = peek_token_bracket (token, regexp, syntax);

      /* Do not check for ranges if we know they are not allowed.  */
      if (start_elem.type != CHAR_CLASS && start_elem.type != EQUIV_CLASS)
	{
	  if (BE (token->type == END_OF_RE, 0))
	    {
	      *err = REG_EBRACK;
	      goto parse_bracket_exp_free_return;
	    }
	  if (token->type == OP_CHARSET_RANGE)
	    {
	      re_string_skip_bytes (regexp, token_len); /* Skip '-'.  */
	      token_len2 = peek_token_bracket (&token2, regexp, syntax);
	      if (BE (token2.type == END_OF_RE, 0))
		{
		  *err = REG_EBRACK;
		  goto parse_bracket_exp_free_return;
		}
	      if (token2.type == OP_CLOSE_BRACKET)
		{
		  /* We treat the last '-' as a normal character.  */
		  re_string_skip_bytes (regexp, -token_len);
		  token->type = CHARACTER;
		}
	      else
		is_range_exp = 1;
	    }
	}

      if (is_range_exp == 1)
	{
	  end_elem.opr.name = end_name_buf;
	  ret = parse_bracket_element (&end_elem, regexp, &token2, token_len2,
				       dfa, syntax, 1);
	  if (BE (ret != REG_NOERROR, 0))
	    {
	      *err = ret;
	      goto parse_bracket_exp_free_return;
	    }

	  token_len = peek_token_bracket (token, regexp, syntax);

	  *err = build_range_exp (sbcset,
#ifdef RE_ENABLE_I18N
				  mbcset, &range_alloc,
#endif /* RE_ENABLE_I18N */
				  &start_elem, &end_elem);
	  if (BE (*err != REG_NOERROR, 0))
	    goto parse_bracket_exp_free_return;
	}
      else
	{
	  switch (start_elem.type)
	    {
	    case SB_CHAR:
	      bitset_set (sbcset, start_elem.opr.ch);
	      break;
#ifdef RE_ENABLE_I18N
	    case MB_CHAR:
	      /* Check whether the array has enough space.  */
	      if (BE (mbchar_alloc == mbcset->nmbchars, 0))
		{
		  wchar_t *new_mbchars;
		  /* Not enough, realloc it.  */
		  /* +1 in case of mbcset->nmbchars is 0.  */
		  mbchar_alloc = 2 * mbcset->nmbchars + 1;
		  /* Use realloc since array is NULL if *alloc == 0.  */
		  new_mbchars = re_realloc (mbcset->mbchars, wchar_t,
					    mbchar_alloc);
		  if (BE (new_mbchars == NULL, 0))
		    goto parse_bracket_exp_espace;
		  mbcset->mbchars = new_mbchars;
		}
	      mbcset->mbchars[mbcset->nmbchars++] = start_elem.opr.wch;
	      break;
#endif /* RE_ENABLE_I18N */
	    case EQUIV_CLASS:
	      *err = build_equiv_class (sbcset,
#ifdef RE_ENABLE_I18N
					mbcset, &equiv_class_alloc,
#endif /* RE_ENABLE_I18N */
					start_elem.opr.name);
	      if (BE (*err != REG_NOERROR, 0))
		goto parse_bracket_exp_free_return;
	      break;
	    case COLL_SYM:
	      *err = build_collating_symbol (sbcset,
#ifdef RE_ENABLE_I18N
					     mbcset, &coll_sym_alloc,
#endif /* RE_ENABLE_I18N */
					     start_elem.opr.name);
	      if (BE (*err != REG_NOERROR, 0))
		goto parse_bracket_exp_free_return;
	      break;
	    case CHAR_CLASS:
	      *err = build_charclass (regexp->trans, sbcset,
#ifdef RE_ENABLE_I18N
				      mbcset, &char_class_alloc,
#endif /* RE_ENABLE_I18N */
				      start_elem.opr.name, syntax);
	      if (BE (*err != REG_NOERROR, 0))
	       goto parse_bracket_exp_free_return;
	      break;
	    default:
	      assert (0);
	      break;
	    }
	}
      if (BE (token->type == END_OF_RE, 0))
	{
	  *err = REG_EBRACK;
	  goto parse_bracket_exp_free_return;
	}
      if (token->type == OP_CLOSE_BRACKET)
	break;
    }

  re_string_skip_bytes (regexp, token_len); /* Skip a token.  */

  /* If it is non-matching list.  */
#ifdef RE_ENABLE_I18N
  if (mbcset->non_match)
#else /* not RE_ENABLE_I18N */
  if (non_match)
#endif /* not RE_ENABLE_I18N */
    bitset_not (sbcset);
#ifdef RE_ENABLE_I18N
  /* Ensure only single byte characters are set.  */
  if (dfa->mb_cur_max > 1)
    bitset_mask (sbcset, dfa->sb_char);
#endif /* RE_ENABLE_I18N */

  /* Build a tree for simple bracket.  */
  br_token.type = SIMPLE_BRACKET;
  br_token.opr.sbcset = sbcset;
  work_tree = re_dfa_add_tree_node (dfa, NULL, NULL, &br_token);
  if (BE (work_tree == NULL, 0))
    goto parse_bracket_exp_espace;

#ifdef RE_ENABLE_I18N
  if (mbcset->nmbchars || mbcset->ncoll_syms || mbcset->nequiv_classes
      || mbcset->nranges || (dfa->mb_cur_max > 1 && (mbcset->nchar_classes
						     || mbcset->non_match)))
    {
      re_token_t alt_token;
      bin_tree_t *mbc_tree;
      int sbc_idx;
      /* Build a tree for complex bracket.  */
      dfa->has_mb_node = 1;
      for (sbc_idx = 0; sbc_idx < BITSET_UINTS; ++sbc_idx)
	if (sbcset[sbc_idx])
	  break;
      /* If there are no bits set in sbcset, there is no point
	 of having both SIMPLE_BRACKET and COMPLEX_BRACKET.  */
      if (sbc_idx == BITSET_UINTS)
	{
	  re_free (sbcset);
	  dfa->nodes[work_tree->node_idx].type = COMPLEX_BRACKET;
	  dfa->nodes[work_tree->node_idx].opr.mbcset = mbcset;
	  return work_tree;
	}
      br_token.type = COMPLEX_BRACKET;
      br_token.opr.mbcset = mbcset;
      mbc_tree = re_dfa_add_tree_node (dfa, NULL, NULL, &br_token);
      if (BE (mbc_tree == NULL, 0))
	goto parse_bracket_exp_espace;
      /* Then join them by ALT node.  */
      alt_token.type = OP_ALT;
      dfa->has_plural_match = 1;
      work_tree = re_dfa_add_tree_node (dfa, work_tree, mbc_tree, &alt_token);
      if (BE (mbc_tree != NULL, 1))
	return work_tree;
    }
  else
    {
      free_charset (mbcset);
      return work_tree;
    }
#else /* not RE_ENABLE_I18N */
  return work_tree;
#endif /* not RE_ENABLE_I18N */

 parse_bracket_exp_espace:
  *err = REG_ESPACE;
 parse_bracket_exp_free_return:
  re_free (sbcset);
#ifdef RE_ENABLE_I18N
  free_charset (mbcset);
#endif /* RE_ENABLE_I18N */
  return NULL;
}

/* Parse an element in the bracket expression.  */

static reg_errcode_t
parse_bracket_element (elem, regexp, token, token_len, dfa, syntax,
		       accept_hyphen)
     bracket_elem_t *elem;
     re_string_t *regexp;
     re_token_t *token;
     int token_len;
     re_dfa_t *dfa;
     reg_syntax_t syntax;
     int accept_hyphen;
{
#ifdef RE_ENABLE_I18N
  int cur_char_size;
  cur_char_size = re_string_char_size_at (regexp, re_string_cur_idx (regexp));
  if (cur_char_size > 1)
    {
      elem->type = MB_CHAR;
      elem->opr.wch = re_string_wchar_at (regexp, re_string_cur_idx (regexp));
      re_string_skip_bytes (regexp, cur_char_size);
      return REG_NOERROR;
    }
#endif /* RE_ENABLE_I18N */
  re_string_skip_bytes (regexp, token_len); /* Skip a token.  */
  if (token->type == OP_OPEN_COLL_ELEM || token->type == OP_OPEN_CHAR_CLASS
      || token->type == OP_OPEN_EQUIV_CLASS)
    return parse_bracket_symbol (elem, regexp, token);
  if (BE (token->type == OP_CHARSET_RANGE, 0) && !accept_hyphen)
    {
      /* A '-' must only appear as anything but a range indicator before
	 the closing bracket.  Everything else is an error.  */
      re_token_t token2;
      (void) peek_token_bracket (&token2, regexp, syntax);
      if (token2.type != OP_CLOSE_BRACKET)
	/* The actual error value is not standardized since this whole
	   case is undefined.  But ERANGE makes good sense.  */
	return REG_ERANGE;
    }
  elem->type = SB_CHAR;
  elem->opr.ch = token->opr.c;
  return REG_NOERROR;
}

/* Parse a bracket symbol in the bracket expression.  Bracket symbols are
   such as [:<character_class>:], [.<collating_element>.], and
   [=<equivalent_class>=].  */

static reg_errcode_t
parse_bracket_symbol (elem, regexp, token)
     bracket_elem_t *elem;
     re_string_t *regexp;
     re_token_t *token;
{
  unsigned char ch, delim = token->opr.c;
  int i = 0;
  for (;; ++i)
    {
      if (re_string_eoi(regexp) || i >= BRACKET_NAME_BUF_SIZE)
	return REG_EBRACK;
      if (token->type == OP_OPEN_CHAR_CLASS)
	ch = re_string_fetch_byte_case (regexp);
      else
	ch = re_string_fetch_byte (regexp);
      if (ch == delim && re_string_peek_byte (regexp, 0) == ']')
	break;
      elem->opr.name[i] = ch;
    }
  re_string_skip_bytes (regexp, 1);
  elem->opr.name[i] = '\0';
  switch (token->type)
    {
    case OP_OPEN_COLL_ELEM:
      elem->type = COLL_SYM;
      break;
    case OP_OPEN_EQUIV_CLASS:
      elem->type = EQUIV_CLASS;
      break;
    case OP_OPEN_CHAR_CLASS:
      elem->type = CHAR_CLASS;
      break;
    default:
      break;
    }
  return REG_NOERROR;
}

  /* Helper function for parse_bracket_exp.
     Build the equivalence class which is represented by NAME.
     The result are written to MBCSET and SBCSET.
     EQUIV_CLASS_ALLOC is the allocated size of mbcset->equiv_classes,
     is a pointer argument sinse we may update it.  */

static reg_errcode_t
#ifdef RE_ENABLE_I18N
build_equiv_class (sbcset, mbcset, equiv_class_alloc, name)
     re_charset_t *mbcset;
     int *equiv_class_alloc;
#else /* not RE_ENABLE_I18N */
build_equiv_class (sbcset, name)
#endif /* not RE_ENABLE_I18N */
     re_bitset_ptr_t sbcset;
     const unsigned char *name;
{
#if defined _LIBC && defined RE_ENABLE_I18N
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
  if (nrules != 0)
    {
      const int32_t *table, *indirect;
      const unsigned char *weights, *extra, *cp;
      unsigned char char_buf[2];
      int32_t idx1, idx2;
      unsigned int ch;
      size_t len;
      /* This #include defines a local function!  */
# include <locale/weight.h>
      /* Calculate the index for equivalence class.  */
      cp = name;
      table = (const int32_t *) _NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
      weights = (const unsigned char *) _NL_CURRENT (LC_COLLATE,
					       _NL_COLLATE_WEIGHTMB);
      extra = (const unsigned char *) _NL_CURRENT (LC_COLLATE,
						   _NL_COLLATE_EXTRAMB);
      indirect = (const int32_t *) _NL_CURRENT (LC_COLLATE,
						_NL_COLLATE_INDIRECTMB);
      idx1 = findidx (&cp);
      if (BE (idx1 == 0 || cp < name + strlen ((const char *) name), 0))
	/* This isn't a valid character.  */
	return REG_ECOLLATE;

      /* Build single byte matcing table for this equivalence class.  */
      char_buf[1] = (unsigned char) '\0';
      len = weights[idx1];
      for (ch = 0; ch < SBC_MAX; ++ch)
	{
	  char_buf[0] = ch;
	  cp = char_buf;
	  idx2 = findidx (&cp);
/*
	  idx2 = table[ch];
*/
	  if (idx2 == 0)
	    /* This isn't a valid character.  */
	    continue;
	  if (len == weights[idx2])
	    {
	      int cnt = 0;
	      while (cnt <= len &&
		     weights[idx1 + 1 + cnt] == weights[idx2 + 1 + cnt])
		++cnt;

	      if (cnt > len)
		bitset_set (sbcset, ch);
	    }
	}
      /* Check whether the array has enough space.  */
      if (BE (*equiv_class_alloc == mbcset->nequiv_classes, 0))
	{
	  /* Not enough, realloc it.  */
	  /* +1 in case of mbcset->nequiv_classes is 0.  */
	  int new_equiv_class_alloc = 2 * mbcset->nequiv_classes + 1;
	  /* Use realloc since the array is NULL if *alloc == 0.  */
	  int32_t *new_equiv_classes = re_realloc (mbcset->equiv_classes,
						   int32_t,
						   new_equiv_class_alloc);
	  if (BE (new_equiv_classes == NULL, 0))
	    return REG_ESPACE;
	  mbcset->equiv_classes = new_equiv_classes;
	  *equiv_class_alloc = new_equiv_class_alloc;
	}
      mbcset->equiv_classes[mbcset->nequiv_classes++] = idx1;
    }
  else
#endif /* _LIBC && RE_ENABLE_I18N */
    {
      if (BE (strlen ((const char *) name) != 1, 0))
	return REG_ECOLLATE;
      bitset_set (sbcset, *name);
    }
  return REG_NOERROR;
}

  /* Helper function for parse_bracket_exp.
     Build the character class which is represented by NAME.
     The result are written to MBCSET and SBCSET.
     CHAR_CLASS_ALLOC is the allocated size of mbcset->char_classes,
     is a pointer argument sinse we may update it.  */

static reg_errcode_t
#ifdef RE_ENABLE_I18N
build_charclass (trans, sbcset, mbcset, char_class_alloc, class_name, syntax)
     re_charset_t *mbcset;
     int *char_class_alloc;
#else /* not RE_ENABLE_I18N */
build_charclass (trans, sbcset, class_name, syntax)
#endif /* not RE_ENABLE_I18N */
     RE_TRANSLATE_TYPE trans;
     re_bitset_ptr_t sbcset;
     const unsigned char *class_name;
     reg_syntax_t syntax;
{
  int i;
  const char *name = (const char *) class_name;

  /* In case of REG_ICASE "upper" and "lower" match the both of
     upper and lower cases.  */
  if ((syntax & RE_ICASE)
      && (strcmp (name, "upper") == 0 || strcmp (name, "lower") == 0))
    name = "alpha";

#ifdef RE_ENABLE_I18N
  /* Check the space of the arrays.  */
  if (BE (*char_class_alloc == mbcset->nchar_classes, 0))
    {
      /* Not enough, realloc it.  */
      /* +1 in case of mbcset->nchar_classes is 0.  */
      int new_char_class_alloc = 2 * mbcset->nchar_classes + 1;
      /* Use realloc since array is NULL if *alloc == 0.  */
      wctype_t *new_char_classes = re_realloc (mbcset->char_classes, wctype_t,
					       new_char_class_alloc);
      if (BE (new_char_classes == NULL, 0))
	return REG_ESPACE;
      mbcset->char_classes = new_char_classes;
      *char_class_alloc = new_char_class_alloc;
    }
  mbcset->char_classes[mbcset->nchar_classes++] = __wctype (name);
#endif /* RE_ENABLE_I18N */

#define BUILD_CHARCLASS_LOOP(ctype_func)	\
    for (i = 0; i < SBC_MAX; ++i)		\
      {						\
	if (ctype_func (i))			\
	  {					\
	    int ch = trans ? trans[i] : i;	\
	    bitset_set (sbcset, ch);		\
	  }					\
      }

  if (strcmp (name, "alnum") == 0)
    BUILD_CHARCLASS_LOOP (isalnum)
  else if (strcmp (name, "cntrl") == 0)
    BUILD_CHARCLASS_LOOP (iscntrl)
  else if (strcmp (name, "lower") == 0)
    BUILD_CHARCLASS_LOOP (islower)
  else if (strcmp (name, "space") == 0)
    BUILD_CHARCLASS_LOOP (isspace)
  else if (strcmp (name, "alpha") == 0)
    BUILD_CHARCLASS_LOOP (isalpha)
  else if (strcmp (name, "digit") == 0)
    BUILD_CHARCLASS_LOOP (isdigit)
  else if (strcmp (name, "print") == 0)
    BUILD_CHARCLASS_LOOP (isprint)
  else if (strcmp (name, "upper") == 0)
    BUILD_CHARCLASS_LOOP (isupper)
  else if (strcmp (name, "blank") == 0)
    BUILD_CHARCLASS_LOOP (isblank)
  else if (strcmp (name, "graph") == 0)
    BUILD_CHARCLASS_LOOP (isgraph)
  else if (strcmp (name, "punct") == 0)
    BUILD_CHARCLASS_LOOP (ispunct)
  else if (strcmp (name, "xdigit") == 0)
    BUILD_CHARCLASS_LOOP (isxdigit)
  else
    return REG_ECTYPE;

  return REG_NOERROR;
}

static bin_tree_t *
build_charclass_op (dfa, trans, class_name, extra, not, err)
     re_dfa_t *dfa;
     RE_TRANSLATE_TYPE trans;
     const unsigned char *class_name;
     const unsigned char *extra;
     int not;
     reg_errcode_t *err;
{
  re_bitset_ptr_t sbcset;
#ifdef RE_ENABLE_I18N
  re_charset_t *mbcset;
  int alloc = 0;
#else /* not RE_ENABLE_I18N */
  int non_match = 0;
#endif /* not RE_ENABLE_I18N */
  reg_errcode_t ret;
  re_token_t br_token;
  bin_tree_t *tree;

  sbcset = (re_bitset_ptr_t) calloc (sizeof (unsigned int), BITSET_UINTS);
#ifdef RE_ENABLE_I18N
  mbcset = (re_charset_t *) calloc (sizeof (re_charset_t), 1);
#endif /* RE_ENABLE_I18N */

#ifdef RE_ENABLE_I18N
  if (BE (sbcset == NULL || mbcset == NULL, 0))
#else /* not RE_ENABLE_I18N */
  if (BE (sbcset == NULL, 0))
#endif /* not RE_ENABLE_I18N */
    {
      *err = REG_ESPACE;
      return NULL;
    }

  if (not)
    {
#ifdef RE_ENABLE_I18N
      /*
      if (syntax & RE_HAT_LISTS_NOT_NEWLINE)
	bitset_set(cset->sbcset, '\0');
      */
      mbcset->non_match = 1;
#else /* not RE_ENABLE_I18N */
      non_match = 1;
#endif /* not RE_ENABLE_I18N */
    }

  /* We don't care the syntax in this case.  */
  ret = build_charclass (trans, sbcset,
#ifdef RE_ENABLE_I18N
			 mbcset, &alloc,
#endif /* RE_ENABLE_I18N */
			 class_name, 0);

  if (BE (ret != REG_NOERROR, 0))
    {
      re_free (sbcset);
#ifdef RE_ENABLE_I18N
      free_charset (mbcset);
#endif /* RE_ENABLE_I18N */
      *err = ret;
      return NULL;
    }
  /* \w match '_' also.  */
  for (; *extra; extra++)
    bitset_set (sbcset, *extra);

  /* If it is non-matching list.  */
#ifdef RE_ENABLE_I18N
  if (mbcset->non_match)
#else /* not RE_ENABLE_I18N */
  if (non_match)
#endif /* not RE_ENABLE_I18N */
    bitset_not (sbcset);

#ifdef RE_ENABLE_I18N
  /* Ensure only single byte characters are set.  */
  if (dfa->mb_cur_max > 1)
    bitset_mask (sbcset, dfa->sb_char);
#endif

  /* Build a tree for simple bracket.  */
  br_token.type = SIMPLE_BRACKET;
  br_token.opr.sbcset = sbcset;
  tree = re_dfa_add_tree_node (dfa, NULL, NULL, &br_token);
  if (BE (tree == NULL, 0))
    goto build_word_op_espace;

#ifdef RE_ENABLE_I18N
  if (dfa->mb_cur_max > 1)
    {
      re_token_t alt_token;
      bin_tree_t *mbc_tree;
      /* Build a tree for complex bracket.  */
      br_token.type = COMPLEX_BRACKET;
      br_token.opr.mbcset = mbcset;
      dfa->has_mb_node = 1;
      mbc_tree = re_dfa_add_tree_node (dfa, NULL, NULL, &br_token);
      if (BE (mbc_tree == NULL, 0))
	goto build_word_op_espace;
      /* Then join them by ALT node.  */
      alt_token.type = OP_ALT;
      dfa->has_plural_match = 1;
      tree = re_dfa_add_tree_node (dfa, tree, mbc_tree, &alt_token);
      if (BE (mbc_tree != NULL, 1))
	return tree;
    }
  else
    {
      free_charset (mbcset);
      return tree;
    }
#else /* not RE_ENABLE_I18N */
  return tree;
#endif /* not RE_ENABLE_I18N */

 build_word_op_espace:
  re_free (sbcset);
#ifdef RE_ENABLE_I18N
  free_charset (mbcset);
#endif /* RE_ENABLE_I18N */
  *err = REG_ESPACE;
  return NULL;
}

/* This is intended for the expressions like "a{1,3}".
   Fetch a number from `input', and return the number.
   Return -1, if the number field is empty like "{,1}".
   Return -2, If an error is occured.  */

static int
fetch_number (input, token, syntax)
     re_string_t *input;
     re_token_t *token;
     reg_syntax_t syntax;
{
  int num = -1;
  unsigned char c;
  while (1)
    {
      fetch_token (token, input, syntax);
      c = token->opr.c;
      if (BE (token->type == END_OF_RE, 0))
	return -2;
      if (token->type == OP_CLOSE_DUP_NUM || c == ',')
	break;
      num = ((token->type != CHARACTER || c < '0' || '9' < c || num == -2)
	     ? -2 : ((num == -1) ? c - '0' : num * 10 + c - '0'));
      num = (num > RE_DUP_MAX) ? -2 : num;
    }
  return num;
}

#ifdef RE_ENABLE_I18N
static void
free_charset (re_charset_t *cset)
{
  re_free (cset->mbchars);
# ifdef _LIBC
  re_free (cset->coll_syms);
  re_free (cset->equiv_classes);
# endif
  re_free (cset->range_starts);
  re_free (cset->range_ends);
  re_free (cset->char_classes);
  re_free (cset);
}
#endif /* RE_ENABLE_I18N */

/* Functions for binary tree operation.  */

/* Create a tree node.  */

static bin_tree_t *
create_tree (dfa, left, right, type, index)
     re_dfa_t *dfa;
     bin_tree_t *left;
     bin_tree_t *right;
     re_token_type_t type;
     int index;
{
  bin_tree_t *tree;
  if (BE (dfa->str_tree_storage_idx == BIN_TREE_STORAGE_SIZE, 0))
    {
      bin_tree_storage_t *storage = re_malloc (bin_tree_storage_t, 1);

      if (storage == NULL)
	return NULL;
      storage->next = dfa->str_tree_storage;
      dfa->str_tree_storage = storage;
      dfa->str_tree_storage_idx = 0;
    }
  tree = &dfa->str_tree_storage->data[dfa->str_tree_storage_idx++];

  tree->parent = NULL;
  tree->left = left;
  tree->right = right;
  tree->type = type;
  tree->node_idx = index;
  tree->first = -1;
  tree->next = -1;
  re_node_set_init_empty (&tree->eclosure);

  if (left != NULL)
    left->parent = tree;
  if (right != NULL)
    right->parent = tree;
  return tree;
}

/* Create both a DFA node and a tree for it.  */

static bin_tree_t *
re_dfa_add_tree_node (dfa, left, right, token)
     re_dfa_t *dfa;
     bin_tree_t *left;
     bin_tree_t *right;
     const re_token_t *token;
{
  int new_idx = re_dfa_add_node (dfa, *token, 0);

  if (new_idx == -1)
    return NULL;

  return create_tree (dfa, left, right, 0, new_idx);
}


/* Duplicate the node SRC, and return new node.  */

static bin_tree_t *
duplicate_tree (src, dfa)
     const bin_tree_t *src;
     re_dfa_t *dfa;
{
  bin_tree_t *left = NULL, *right = NULL, *new_tree;
  int new_node_idx;
  /* Since node indies must be according to Post-order of the tree,
     we must duplicate the left at first.  */
  if (src->left != NULL)
    {
      left = duplicate_tree (src->left, dfa);
      if (left == NULL)
	return NULL;
    }

  /* Secondaly, duplicate the right.  */
  if (src->right != NULL)
    {
      right = duplicate_tree (src->right, dfa);
      if (right == NULL)
	return NULL;
    }

  /* At last, duplicate itself.  */
  if (src->type == NON_TYPE)
    {
      new_node_idx = re_dfa_add_node (dfa, dfa->nodes[src->node_idx], 0);
      dfa->nodes[new_node_idx].duplicated = 1;
      if (BE (new_node_idx == -1, 0))
	return NULL;
    }
  else
    new_node_idx = src->type;

  new_tree = create_tree (dfa, left, right, src->type, new_node_idx);
  return new_tree;
}

static reg_errcode_t match_ctx_init (re_match_context_t *cache, int eflags,
				     re_string_t *input, int n);
static void match_ctx_clean (re_match_context_t *mctx);
static void match_ctx_free (re_match_context_t *cache);
static void match_ctx_free_subtops (re_match_context_t *mctx);
static reg_errcode_t match_ctx_add_entry (re_match_context_t *cache, int node,
					  int str_idx, int from, int to);
static int search_cur_bkref_entry (re_match_context_t *mctx, int str_idx);
static void match_ctx_clear_flag (re_match_context_t *mctx);
static reg_errcode_t match_ctx_add_subtop (re_match_context_t *mctx, int node,
					   int str_idx);
static re_sub_match_last_t * match_ctx_add_sublast (re_sub_match_top_t *subtop,
						   int node, int str_idx);
static void sift_ctx_init (re_sift_context_t *sctx, re_dfastate_t **sifted_sts,
			   re_dfastate_t **limited_sts, int last_node,
			   int last_str_idx, int check_subexp);
static reg_errcode_t re_search_internal (const regex_t *preg,
					 const char *string, int length,
					 int start, int range, int stop,
					 size_t nmatch, regmatch_t pmatch[],
					 int eflags);
#if 0
static int re_search_2_stub (struct re_pattern_buffer *bufp,
			     const char *string1, int length1,
			     const char *string2, int length2,
			     int start, int range, struct re_registers *regs,
			     int stop, int ret_len);
static int re_search_stub (struct re_pattern_buffer *bufp,
			   const char *string, int length, int start,
			   int range, int stop, struct re_registers *regs,
			   int ret_len);
static unsigned re_copy_regs (struct re_registers *regs, regmatch_t *pmatch,
			      int nregs, int regs_allocated);
#endif
static R_INLINE re_dfastate_t *acquire_init_state_context
  (reg_errcode_t *err, const regex_t *preg, const re_match_context_t *mctx,
   int idx) __attribute ((always_inline));
static reg_errcode_t prune_impossible_nodes (const regex_t *preg,
					     re_match_context_t *mctx);
static int check_matching (const regex_t *preg, re_match_context_t *mctx,
			   int fl_longest_match);
static int check_halt_node_context (const re_dfa_t *dfa, int node,
				    unsigned int context);
static int check_halt_state_context (const regex_t *preg,
				     const re_dfastate_t *state,
				     const re_match_context_t *mctx, int idx);
static void update_regs (re_dfa_t *dfa, regmatch_t *pmatch, int cur_node,
			 int cur_idx, int nmatch);
static int proceed_next_node (const regex_t *preg, int nregs, regmatch_t *regs,
			      const re_match_context_t *mctx,
			      int *pidx, int node, re_node_set *eps_via_nodes,
			      struct re_fail_stack_t *fs);
static reg_errcode_t push_fail_stack (struct re_fail_stack_t *fs,
				      int str_idx, int *dests, int nregs,
				      regmatch_t *regs,
				      re_node_set *eps_via_nodes);
static int pop_fail_stack (struct re_fail_stack_t *fs, int *pidx, int nregs,
			   regmatch_t *regs, re_node_set *eps_via_nodes);
static reg_errcode_t set_regs (const regex_t *preg,
			       const re_match_context_t *mctx,
			       size_t nmatch, regmatch_t *pmatch,
			       int fl_backtrack);
static reg_errcode_t free_fail_stack_return (struct re_fail_stack_t *fs);

#ifdef RE_ENABLE_I18N
static int sift_states_iter_mb (const regex_t *preg,
				const re_match_context_t *mctx,
				re_sift_context_t *sctx,
				int node_idx, int str_idx, int max_str_idx);
#endif /* RE_ENABLE_I18N */
static reg_errcode_t sift_states_backward (const regex_t *preg,
					   re_match_context_t *mctx,
					   re_sift_context_t *sctx);
static reg_errcode_t update_cur_sifted_state (const regex_t *preg,
					      re_match_context_t *mctx,
					      re_sift_context_t *sctx,
					      int str_idx,
					      re_node_set *dest_nodes);
static reg_errcode_t add_epsilon_src_nodes (re_dfa_t *dfa,
					    re_node_set *dest_nodes,
					    const re_node_set *candidates);
static reg_errcode_t sub_epsilon_src_nodes (re_dfa_t *dfa, int node,
					    re_node_set *dest_nodes,
					    const re_node_set *and_nodes);
static int check_dst_limits (re_dfa_t *dfa, re_node_set *limits,
			     re_match_context_t *mctx, int dst_node,
			     int dst_idx, int src_node, int src_idx);
static int check_dst_limits_calc_pos (re_dfa_t *dfa, re_match_context_t *mctx,
				      int limit, re_node_set *eclosures,
				      int subexp_idx, int node, int str_idx);
static reg_errcode_t check_subexp_limits (re_dfa_t *dfa,
					  re_node_set *dest_nodes,
					  const re_node_set *candidates,
					  re_node_set *limits,
					  struct re_backref_cache_entry *bkref_ents,
					  int str_idx);
static reg_errcode_t sift_states_bkref (const regex_t *preg,
					re_match_context_t *mctx,
					re_sift_context_t *sctx,
					int str_idx, re_node_set *dest_nodes);
static reg_errcode_t clean_state_log_if_need (re_match_context_t *mctx,
					      int next_state_log_idx);
static reg_errcode_t merge_state_array (re_dfa_t *dfa, re_dfastate_t **dst,
					re_dfastate_t **src, int num);
static re_dfastate_t *transit_state (reg_errcode_t *err, const regex_t *preg,
				     re_match_context_t *mctx,
				     re_dfastate_t *state);
static reg_errcode_t check_subexp_matching_top (re_dfa_t *dfa,
						re_match_context_t *mctx,
						re_node_set *cur_nodes,
						int str_idx);
#if 0
static re_dfastate_t *transit_state_sb (reg_errcode_t *err, const regex_t *preg,
					re_dfastate_t *pstate,
					re_match_context_t *mctx);
#endif
#ifdef RE_ENABLE_I18N
static reg_errcode_t transit_state_mb (const regex_t *preg,
				       re_dfastate_t *pstate,
				       re_match_context_t *mctx);
#endif /* RE_ENABLE_I18N */
static reg_errcode_t transit_state_bkref (const regex_t *preg,
					  const re_node_set *nodes,
					  re_match_context_t *mctx);
static reg_errcode_t get_subexp (const regex_t *preg, re_match_context_t *mctx,
				 int bkref_node, int bkref_str_idx);
static reg_errcode_t get_subexp_sub (const regex_t *preg,
				     re_match_context_t *mctx,
				     const re_sub_match_top_t *sub_top,
				     re_sub_match_last_t *sub_last,
				     int bkref_node, int bkref_str);
static int find_subexp_node (const re_dfa_t *dfa, const re_node_set *nodes,
			     int subexp_idx, int fl_open);
static reg_errcode_t check_arrival (const regex_t *preg,
				    re_match_context_t *mctx,
				    state_array_t *path, int top_node,
				    int top_str, int last_node, int last_str,
				    int fl_open);
static reg_errcode_t check_arrival_add_next_nodes (const regex_t *preg,
						   re_dfa_t *dfa,
						   re_match_context_t *mctx,
						   int str_idx,
						   re_node_set *cur_nodes,
						   re_node_set *next_nodes);
static reg_errcode_t check_arrival_expand_ecl (re_dfa_t *dfa,
					       re_node_set *cur_nodes,
					       int ex_subexp, int fl_open);
static reg_errcode_t check_arrival_expand_ecl_sub (re_dfa_t *dfa,
						   re_node_set *dst_nodes,
						   int target, int ex_subexp,
						   int fl_open);
static reg_errcode_t expand_bkref_cache (const regex_t *preg,
					 re_match_context_t *mctx,
					 re_node_set *cur_nodes, int cur_str,
					 int last_str, int subexp_num,
					 int fl_open);
static re_dfastate_t **build_trtable (const regex_t *dfa,
				      re_dfastate_t *state);
#ifdef RE_ENABLE_I18N
static int check_node_accept_bytes (const regex_t *preg, int node_idx,
				    const re_string_t *input, int idx);
# ifdef _LIBC
static unsigned int find_collation_sequence_value (const unsigned char *mbs,
						   size_t name_len);
# endif /* _LIBC */
#endif /* RE_ENABLE_I18N */
static int group_nodes_into_DFAstates (const regex_t *dfa,
				       const re_dfastate_t *state,
				       re_node_set *states_node,
				       bitset *states_ch);
static int check_node_accept (const regex_t *preg, const re_token_t *node,
			      const re_match_context_t *mctx, int idx);
static reg_errcode_t extend_buffers (re_match_context_t *mctx);

/* Entry point for POSIX code.  */

/* regexec searches for a given pattern, specified by PREG, in the
   string STRING.

   If NMATCH is zero or REG_NOSUB was set in the cflags argument to
   `regcomp', we ignore PMATCH.  Otherwise, we assume PMATCH has at
   least NMATCH elements, and we set them to the offsets of the
   corresponding matched substrings.

   EFLAGS specifies `execution flags' which affect matching: if
   REG_NOTBOL is set, then ^ does not match at the beginning of the
   string; if REG_NOTEOL is set, then $ does not match at the end.

   We return 0 if we find a match and REG_NOMATCH if not.  */

int
regexec (preg, string, nmatch, pmatch, eflags)
    const regex_t *__restrict preg;
    const char *__restrict string;
    size_t nmatch;
    regmatch_t pmatch[];
    int eflags;
{
  reg_errcode_t err;
  int length = strlen (string);
  if (preg->no_sub)
    err = re_search_internal (preg, string, length, 0, length, length, 0,
			      NULL, eflags);
  else
    err = re_search_internal (preg, string, length, 0, length, length, nmatch,
			      pmatch, eflags);
  return err != REG_NOERROR;
}
#ifdef _LIBC
weak_alias (__regexec, regexec)
#endif

/* Entry points for GNU code.  */

#if 0
/* re_match, re_search, re_match_2, re_search_2

   The former two functions operate on STRING with length LENGTH,
   while the later two operate on concatenation of STRING1 and STRING2
   with lengths LENGTH1 and LENGTH2, respectively.

   re_match() matches the compiled pattern in BUFP against the string,
   starting at index START.

   re_search() first tries matching at index START, then it tries to match
   starting from index START + 1, and so on.  The last start position tried
   is START + RANGE.  (Thus RANGE = 0 forces re_search to operate the same
   way as re_match().)

   The parameter STOP of re_{match,search}_2 specifies that no match exceeding
   the first STOP characters of the concatenation of the strings should be
   concerned.

   If REGS is not NULL, and BUFP->no_sub is not set, the offsets of the match
   and all groups is stroed in REGS.  (For the "_2" variants, the offsets are
   computed relative to the concatenation, not relative to the individual
   strings.)

   On success, re_match* functions return the length of the match, re_search*
   return the position of the start of the match.  Return value -1 means no
   match was found and -2 indicates an internal error.  */

int
re_match (bufp, string, length, start, regs)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start;
    struct re_registers *regs;
{
  return re_search_stub (bufp, string, length, start, 0, length, regs, 1);
}
#ifdef _LIBC
weak_alias (__re_match, re_match)
#endif

int
re_search (bufp, string, length, start, range, regs)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start, range;
    struct re_registers *regs;
{
  return re_search_stub (bufp, string, length, start, range, length, regs, 0);
}
#ifdef _LIBC
weak_alias (__re_search, re_search)
#endif

int
re_match_2 (bufp, string1, length1, string2, length2, start, regs, stop)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, stop;
    struct re_registers *regs;
{
  return re_search_2_stub (bufp, string1, length1, string2, length2,
			   start, 0, regs, stop, 1);
}
#ifdef _LIBC
weak_alias (__re_match_2, re_match_2)
#endif

int
re_search_2 (bufp, string1, length1, string2, length2, start, range, regs, stop)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, range, stop;
    struct re_registers *regs;
{
  return re_search_2_stub (bufp, string1, length1, string2, length2,
			   start, range, regs, stop, 0);
}
#ifdef _LIBC
weak_alias (__re_search_2, re_search_2)
#endif

static int
re_search_2_stub (bufp, string1, length1, string2, length2, start, range, regs,
		  stop, ret_len)
    struct re_pattern_buffer *bufp;
    const char *string1, *string2;
    int length1, length2, start, range, stop, ret_len;
    struct re_registers *regs;
{
  const char *str;
  int rval;
  int len = length1 + length2;
  int free_str = 0;

  if (BE (length1 < 0 || length2 < 0 || stop < 0, 0))
    return -2;

  /* Concatenate the strings.  */
  if (length2 > 0)
    if (length1 > 0)
      {
	char *s = re_malloc (char, len);

	if (BE (s == NULL, 0))
	  return -2;
	memcpy (s, string1, length1);
	memcpy (s + length1, string2, length2);
	str = s;
	free_str = 1;
      }
    else
      str = string2;
  else
    str = string1;

  rval = re_search_stub (bufp, str, len, start, range, stop, regs,
			 ret_len);
  if (free_str)
    re_free ((char *) str);
  return rval;
}

/* The parameters have the same meaning as those of re_search.
   Additional parameters:
   If RET_LEN is nonzero the length of the match is returned (re_match style);
   otherwise the position of the match is returned.  */

static int
re_search_stub (bufp, string, length, start, range, stop, regs, ret_len)
    struct re_pattern_buffer *bufp;
    const char *string;
    int length, start, range, stop, ret_len;
    struct re_registers *regs;
{
  reg_errcode_t result;
  regmatch_t *pmatch;
  int nregs, rval;
  int eflags = 0;

  /* Check for out-of-range.  */
  if (BE (start < 0 || start > length, 0))
    return -1;
  if (BE (start + range > length, 0))
    range = length - start;
  else if (BE (start + range < 0, 0))
    range = -start;

  eflags |= (bufp->not_bol) ? REG_NOTBOL : 0;
  eflags |= (bufp->not_eol) ? REG_NOTEOL : 0;

  /* Compile fastmap if we haven't yet.  */
  if (range > 0 && bufp->fastmap != NULL && !bufp->fastmap_accurate)
    re_compile_fastmap (bufp);

  if (BE (bufp->no_sub, 0))
    regs = NULL;

  /* We need at least 1 register.  */
  if (regs == NULL)
    nregs = 1;
  else if (BE (bufp->regs_allocated == REGS_FIXED &&
	       regs->num_regs < bufp->re_nsub + 1, 0))
    {
      nregs = regs->num_regs;
      if (BE (nregs < 1, 0))
	{
	  /* Nothing can be copied to regs.  */
	  regs = NULL;
	  nregs = 1;
	}
    }
  else
    nregs = bufp->re_nsub + 1;
  pmatch = re_malloc (regmatch_t, nregs);
  if (BE (pmatch == NULL, 0))
    return -2;

  result = re_search_internal (bufp, string, length, start, range, stop,
			       nregs, pmatch, eflags);

  rval = 0;

  /* I hope we needn't fill ther regs with -1's when no match was found.  */
  if (result != REG_NOERROR)
    rval = -1;
  else if (regs != NULL)
    {
      /* If caller wants register contents data back, copy them.  */
      bufp->regs_allocated = re_copy_regs (regs, pmatch, nregs,
					   bufp->regs_allocated);
      if (BE (bufp->regs_allocated == REGS_UNALLOCATED, 0))
	rval = -2;
    }

  if (BE (rval == 0, 1))
    {
      if (ret_len)
	{
	  assert (pmatch[0].rm_so == start);
	  rval = pmatch[0].rm_eo - start;
	}
      else
	rval = pmatch[0].rm_so;
    }
  re_free (pmatch);
  return rval;
}

static unsigned
re_copy_regs (regs, pmatch, nregs, regs_allocated)
    struct re_registers *regs;
    regmatch_t *pmatch;
    int nregs, regs_allocated;
{
  int rval = REGS_REALLOCATE;
  int i;
  int need_regs = nregs + 1;
  /* We need one extra element beyond `num_regs' for the `-1' marker GNU code
     uses.  */

  /* Have the register data arrays been allocated?  */
  if (regs_allocated == REGS_UNALLOCATED)
    { /* No.  So allocate them with malloc.  */
      regs->start = re_malloc (regoff_t, need_regs);
      if (BE (regs->start == NULL, 0))
	return REGS_UNALLOCATED;
      regs->end = re_malloc (regoff_t, need_regs);
      if (BE (regs->end == NULL, 0))
	{
	  re_free (regs->start);
	  return REGS_UNALLOCATED;
	}
      regs->num_regs = need_regs;
    }
  else if (regs_allocated == REGS_REALLOCATE)
    { /* Yes.  If we need more elements than were already
	 allocated, reallocate them.  If we need fewer, just
	 leave it alone.  */
      if (BE (need_regs > regs->num_regs, 0))
	{
	  regs->start = re_realloc (regs->start, regoff_t, need_regs);
	  if (BE (regs->start == NULL, 0))
	    {
	      if (regs->end != NULL)
		re_free (regs->end);
	      return REGS_UNALLOCATED;
	    }
	  regs->end = re_realloc (regs->end, regoff_t, need_regs);
	  if (BE (regs->end == NULL, 0))
	    {
	      re_free (regs->start);
	      return REGS_UNALLOCATED;
	    }
	  regs->num_regs = need_regs;
	}
    }
  else
    {
      assert (regs_allocated == REGS_FIXED);
      /* This function may not be called with REGS_FIXED and nregs too big.  */
      assert (regs->num_regs >= nregs);
      rval = REGS_FIXED;
    }

  /* Copy the regs.  */
  for (i = 0; i < nregs; ++i)
    {
      regs->start[i] = pmatch[i].rm_so;
      regs->end[i] = pmatch[i].rm_eo;
    }
  for ( ; i < regs->num_regs; ++i)
    regs->start[i] = regs->end[i] = -1;

  return rval;
}

/* Set REGS to hold NUM_REGS registers, storing them in STARTS and
   ENDS.  Subsequent matches using PATTERN_BUFFER and REGS will use
   this memory for recording register information.  STARTS and ENDS
   must be allocated using the malloc library routine, and must each
   be at least NUM_REGS * sizeof (regoff_t) bytes long.

   If NUM_REGS == 0, then subsequent matches should allocate their own
   register data.

   Unless this function is called, the first search or match using
   PATTERN_BUFFER will allocate its own register data, without
   freeing the old data.  */

void
re_set_registers (bufp, regs, num_regs, starts, ends)
    struct re_pattern_buffer *bufp;
    struct re_registers *regs;
    unsigned num_regs;
    regoff_t *starts, *ends;
{
  if (num_regs)
    {
      bufp->regs_allocated = REGS_REALLOCATE;
      regs->num_regs = num_regs;
      regs->start = starts;
      regs->end = ends;
    }
  else
    {
      bufp->regs_allocated = REGS_UNALLOCATED;
      regs->num_regs = 0;
      regs->start = regs->end = (regoff_t *) 0;
    }
}
#ifdef _LIBC
weak_alias (__re_set_registers, re_set_registers)
#endif
#endif

/* Entry points compatible with 4.2 BSD regex library.  We don't define
   them unless specifically requested.  */

#if defined _REGEX_RE_COMP || defined _LIBC
int
# ifdef _LIBC
weak_function
# endif
re_exec (s)
     const char *s;
{
  return 0 == regexec (&re_comp_buf, s, 0, NULL, 0);
}
#endif /* _REGEX_RE_COMP */

static re_node_set empty_set;

/* Internal entry point.  */

/* Searches for a compiled pattern PREG in the string STRING, whose
   length is LENGTH.  NMATCH, PMATCH, and EFLAGS have the same
   mingings with regexec.  START, and RANGE have the same meanings
   with re_search.
   Return REG_NOERROR if we find a match, and REG_NOMATCH if not,
   otherwise return the error code.
   Note: We assume front end functions already check ranges.
   (START + RANGE >= 0 && START + RANGE <= LENGTH)  */

static reg_errcode_t
re_search_internal (preg, string, length, start, range, stop, nmatch, pmatch,
		    eflags)
    const regex_t *preg;
    const char *string;
    int length, start, range, stop, eflags;
    size_t nmatch;
    regmatch_t pmatch[];
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_string_t input;
  int left_lim, right_lim, incr;
  int fl_longest_match, match_first, match_last = -1;
  int fast_translate, sb;
  re_match_context_t mctx;
  char *fastmap = ((preg->fastmap != NULL && preg->fastmap_accurate
		    && range && !preg->can_be_null) ? preg->fastmap : NULL);

  /* Check if the DFA haven't been compiled.  */
  if (BE (preg->used == 0 || dfa->init_state == NULL
	  || dfa->init_state_word == NULL || dfa->init_state_nl == NULL
	  || dfa->init_state_begbuf == NULL, 0))
    return REG_NOMATCH;

  re_node_set_init_empty (&empty_set);
  memset (&mctx, '\0', sizeof (re_match_context_t));

  /* We must check the longest matching, if nmatch > 0.  */
  fl_longest_match = (nmatch != 0 || dfa->nbackref);

  err = re_string_allocate (&input, string, length, dfa->nodes_len + 1,
			    preg->translate, preg->syntax & RE_ICASE, dfa);
  if (BE (err != REG_NOERROR, 0))
    goto free_return;
  input.stop = stop;
  input.raw_stop = stop;

  err = match_ctx_init (&mctx, eflags, &input, dfa->nbackref * 2);
  if (BE (err != REG_NOERROR, 0))
    goto free_return;

  /* We will log all the DFA states through which the dfa pass,
     if nmatch > 1, or this dfa has "multibyte node", which is a
     back-reference or a node which can accept multibyte character or
     multi character collating element.  */
  if (nmatch > 1 || dfa->has_mb_node)
    {
      mctx.state_log = re_malloc (re_dfastate_t *, input.bufs_len + 1);
      if (BE (mctx.state_log == NULL, 0))
	{
	  err = REG_ESPACE;
	  goto free_return;
	}
    }
  else
    mctx.state_log = NULL;

#ifdef DEBUG
  /* We assume front-end functions already check them.  */
  assert (start + range >= 0 && start + range <= length);
#endif

  match_first = start;
  input.tip_context = ((eflags & REG_NOTBOL) ? CONTEXT_BEGBUF
		       : CONTEXT_NEWLINE | CONTEXT_BEGBUF);

  /* Check incrementally whether of not the input string match.  */
  incr = (range < 0) ? -1 : 1;
  left_lim = (range < 0) ? start + range : start;
  right_lim = (range < 0) ? start : start + range;
  sb = dfa->mb_cur_max == 1;
  fast_translate = sb || !(preg->syntax & RE_ICASE || preg->translate);

  for (;;)
    {
      /* At first get the current byte from input string.  */
      if (fastmap)
	{
	  if (BE (fast_translate, 1))
	    {
	      unsigned RE_TRANSLATE_TYPE t
		= (unsigned RE_TRANSLATE_TYPE) preg->translate;
	      if (BE (range >= 0, 1))
		{
		  if (BE (t != NULL, 0))
		    {
		      while (BE (match_first < right_lim, 1)
			     && !fastmap[t[(unsigned char) string[match_first]]])
			++match_first;
		    }
		  else
		    {
		      while (BE (match_first < right_lim, 1)
			     && !fastmap[(unsigned char) string[match_first]])
			++match_first;
		    }
		  if (BE (match_first == right_lim, 0))
		    {
		      int ch = match_first >= length
			       ? 0 : (unsigned char) string[match_first];
		      if (!fastmap[t ? t[ch] : ch])
			break;
		    }
		}
	      else
		{
		  while (match_first >= left_lim)
		    {
		      int ch = match_first >= length
			       ? 0 : (unsigned char) string[match_first];
		      if (fastmap[t ? t[ch] : ch])
			break;
		      --match_first;
		    }
		  if (match_first < left_lim)
		    break;
		}
	    }
	  else
	    {
	      int ch;

	      do
		{
		  /* In this case, we can't determine easily the current byte,
		     since it might be a component byte of a multibyte
		     character.  Then we use the constructed buffer
		     instead.  */
		  /* If MATCH_FIRST is out of the valid range, reconstruct the
		     buffers.  */
		  if (input.raw_mbs_idx + input.valid_raw_len <= match_first
		      || match_first < input.raw_mbs_idx)
		    {
		      err = re_string_reconstruct (&input, match_first, eflags,
						   preg->newline_anchor);
		      if (BE (err != REG_NOERROR, 0))
			goto free_return;
		    }
		  /* If MATCH_FIRST is out of the buffer, leave it as '\0'.
		     Note that MATCH_FIRST must not be smaller than 0.  */
		  ch = ((match_first >= length) ? 0
		       : re_string_byte_at (&input,
					    match_first - input.raw_mbs_idx));
		  if (fastmap[ch])
		    break;
		  match_first += incr;
		}
	      while (match_first >= left_lim && match_first <= right_lim);
	      if (! fastmap[ch])
		break;
	    }
	}

      /* Reconstruct the buffers so that the matcher can assume that
	 the matching starts from the beginning of the buffer.  */
      err = re_string_reconstruct (&input, match_first, eflags,
				   preg->newline_anchor);
      if (BE (err != REG_NOERROR, 0))
	goto free_return;
#ifdef RE_ENABLE_I18N
     /* Eliminate it when it is a component of a multibyte character
	 and isn't the head of a multibyte character.  */
      if (sb || re_string_first_byte (&input, 0))
#endif
	{
	  /* It seems to be appropriate one, then use the matcher.  */
	  /* We assume that the matching starts from 0.  */
	  mctx.state_log_top = mctx.nbkref_ents = mctx.max_mb_elem_len = 0;
	  match_last = check_matching (preg, &mctx, fl_longest_match);
	  if (match_last != -1)
	    {
	      if (BE (match_last == -2, 0))
		{
		  err = REG_ESPACE;
		  goto free_return;
		}
	      else
		{
		  mctx.match_last = match_last;
		  if ((!preg->no_sub && nmatch > 1) || dfa->nbackref)
		    {
		      re_dfastate_t *pstate = mctx.state_log[match_last];
		      mctx.last_node = check_halt_state_context (preg, pstate,
								 &mctx, match_last);
		    }
		  if ((!preg->no_sub && nmatch > 1 && dfa->has_plural_match)
		      || dfa->nbackref)
		    {
		      err = prune_impossible_nodes (preg, &mctx);
		      if (err == REG_NOERROR)
			break;
		      if (BE (err != REG_NOMATCH, 0))
			goto free_return;
		      match_last = -1;
		    }
		  else
		    break; /* We found a match.  */
		}
	    }
	  match_ctx_clean (&mctx);
	}
      /* Update counter.  */
      match_first += incr;
      if (match_first < left_lim || right_lim < match_first)
	break;
    }

  /* Set pmatch[] if we need.  */
  if (match_last != -1 && nmatch > 0)
    {
      int reg_idx;

      /* Initialize registers.  */
      for (reg_idx = 1; reg_idx < nmatch; ++reg_idx)
	pmatch[reg_idx].rm_so = pmatch[reg_idx].rm_eo = -1;

      /* Set the points where matching start/end.  */
      pmatch[0].rm_so = 0;
      pmatch[0].rm_eo = mctx.match_last;

      if (!preg->no_sub && nmatch > 1)
	{
	  err = set_regs (preg, &mctx, nmatch, pmatch,
			  dfa->has_plural_match && dfa->nbackref > 0);
	  if (BE (err != REG_NOERROR, 0))
	    goto free_return;
	}

      /* At last, add the offset to the each registers, since we slided
	 the buffers so that we could assume that the matching starts
	 from 0.  */
      for (reg_idx = 0; reg_idx < nmatch; ++reg_idx)
	if (pmatch[reg_idx].rm_so != -1)
	  {
	    if (BE (input.offsets_needed != 0, 0))
	      {
		if (pmatch[reg_idx].rm_so == input.valid_len)
		  pmatch[reg_idx].rm_so += input.valid_raw_len - input.valid_len;
		else
		  pmatch[reg_idx].rm_so = input.offsets[pmatch[reg_idx].rm_so];
		if (pmatch[reg_idx].rm_eo == input.valid_len)
		  pmatch[reg_idx].rm_eo += input.valid_raw_len - input.valid_len;
		else
		  pmatch[reg_idx].rm_eo = input.offsets[pmatch[reg_idx].rm_eo];
	      }
	    pmatch[reg_idx].rm_so += match_first;
	    pmatch[reg_idx].rm_eo += match_first;
	  }
    }
  err = (match_last == -1) ? REG_NOMATCH : REG_NOERROR;
 free_return:
  re_free (mctx.state_log);
  if (dfa->nbackref)
    match_ctx_free (&mctx);
  re_string_destruct (&input);
  return err;
}

static reg_errcode_t
prune_impossible_nodes (preg, mctx)
     const regex_t *preg;
     re_match_context_t *mctx;
{
  int halt_node, match_last;
  reg_errcode_t ret;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  re_dfastate_t **sifted_states;
  re_dfastate_t **lim_states = NULL;
  re_sift_context_t sctx;
#ifdef DEBUG
  assert (mctx->state_log != NULL);
#endif
  match_last = mctx->match_last;
  halt_node = mctx->last_node;
  sifted_states = re_malloc (re_dfastate_t *, match_last + 1);
  if (BE (sifted_states == NULL, 0))
    {
      ret = REG_ESPACE;
      goto free_return;
    }
  if (dfa->nbackref)
    {
      lim_states = re_malloc (re_dfastate_t *, match_last + 1);
      if (BE (lim_states == NULL, 0))
	{
	  ret = REG_ESPACE;
	  goto free_return;
	}
      while (1)
	{
	  memset (lim_states, '\0',
		  sizeof (re_dfastate_t *) * (match_last + 1));
	  match_ctx_clear_flag (mctx);
	  sift_ctx_init (&sctx, sifted_states, lim_states, halt_node,
			 match_last, 0);
	  ret = sift_states_backward (preg, mctx, &sctx);
	  re_node_set_free (&sctx.limits);
	  if (BE (ret != REG_NOERROR, 0))
	      goto free_return;
	  if (sifted_states[0] != NULL || lim_states[0] != NULL)
	    break;
	  do
	    {
	      --match_last;
	      if (match_last < 0)
		{
		  ret = REG_NOMATCH;
		  goto free_return;
		}
	    } while (mctx->state_log[match_last] == NULL
		     || !mctx->state_log[match_last]->halt);
	  halt_node = check_halt_state_context (preg,
						mctx->state_log[match_last],
						mctx, match_last);
	}
      ret = merge_state_array (dfa, sifted_states, lim_states,
			       match_last + 1);
      re_free (lim_states);
      lim_states = NULL;
      if (BE (ret != REG_NOERROR, 0))
	goto free_return;
    }
  else
    {
      sift_ctx_init (&sctx, sifted_states, lim_states, halt_node,
		     match_last, 0);
      ret = sift_states_backward (preg, mctx, &sctx);
      re_node_set_free (&sctx.limits);
      if (BE (ret != REG_NOERROR, 0))
	goto free_return;
    }
  re_free (mctx->state_log);
  mctx->state_log = sifted_states;
  sifted_states = NULL;
  mctx->last_node = halt_node;
  mctx->match_last = match_last;
  ret = REG_NOERROR;
 free_return:
  re_free (sifted_states);
  re_free (lim_states);
  return ret;
}

/* Acquire an initial state and return it.
   We must select appropriate initial state depending on the context,
   since initial states may have constraints like "\<", "^", etc..  */

static R_INLINE re_dfastate_t *
acquire_init_state_context (err, preg, mctx, idx)
     reg_errcode_t *err;
     const regex_t *preg;
     const re_match_context_t *mctx;
     int idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;

  *err = REG_NOERROR;
  if (dfa->init_state->has_constraint)
    {
      unsigned int context;
      context = re_string_context_at (mctx->input, idx - 1, mctx->eflags,
				      preg->newline_anchor);
      if (IS_WORD_CONTEXT (context))
	return dfa->init_state_word;
      else if (IS_ORDINARY_CONTEXT (context))
	return dfa->init_state;
      else if (IS_BEGBUF_CONTEXT (context) && IS_NEWLINE_CONTEXT (context))
	return dfa->init_state_begbuf;
      else if (IS_NEWLINE_CONTEXT (context))
	return dfa->init_state_nl;
      else if (IS_BEGBUF_CONTEXT (context))
	{
	  /* It is relatively rare case, then calculate on demand.  */
	  return  re_acquire_state_context (err, dfa,
					    dfa->init_state->entrance_nodes,
					    context);
	}
      else
	/* Must not happen?  */
	return dfa->init_state;
    }
  else
    return dfa->init_state;
}

/* Check whether the regular expression match input string INPUT or not,
   and return the index where the matching end, return -1 if not match,
   or return -2 in case of an error.
   FL_LONGEST_MATCH means we want the POSIX longest matching.
   Note that the matcher assume that the maching starts from the current
   index of the buffer.  */

static int
check_matching (preg, mctx, fl_longest_match)
    const regex_t *preg;
    re_match_context_t *mctx;
    int fl_longest_match;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  reg_errcode_t err;
  int match = 0;
  int match_last = -1;
  int cur_str_idx = re_string_cur_idx (mctx->input);
  re_dfastate_t *cur_state;

  cur_state = acquire_init_state_context (&err, preg, mctx, cur_str_idx);
  /* An initial state must not be NULL(invalid state).  */
  if (BE (cur_state == NULL, 0))
    return -2;
  if (mctx->state_log != NULL)
    mctx->state_log[cur_str_idx] = cur_state;

  /* Check OP_OPEN_SUBEXP in the initial state in case that we use them
     later.  E.g. Processing back references.  */
  if (BE (dfa->nbackref, 0))
    {
      err = check_subexp_matching_top (dfa, mctx, &cur_state->nodes, 0);
      if (BE (err != REG_NOERROR, 0))
	return err;

      if (cur_state->has_backref)
	{
	  err = transit_state_bkref (preg, &cur_state->nodes, mctx);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
    }

  /* If the RE accepts NULL string.  */
  if (BE (cur_state->halt, 0))
    {
      if (!cur_state->has_constraint
	  || check_halt_state_context (preg, cur_state, mctx, cur_str_idx))
	{
	  if (!fl_longest_match)
	    return cur_str_idx;
	  else
	    {
	      match_last = cur_str_idx;
	      match = 1;
	    }
	}
    }

  while (!re_string_eoi (mctx->input))
    {
      cur_state = transit_state (&err, preg, mctx, cur_state);
      if (cur_state == NULL) /* Reached at the invalid state or an error.  */
	{
	  cur_str_idx = re_string_cur_idx (mctx->input);
	  if (BE (err != REG_NOERROR, 0))
	    return -2;
	  if (!fl_longest_match && match)
	    break;
	  else
	    {
	      if (mctx->state_log == NULL)
		break;
	      else
		{
		  int max = mctx->state_log_top;
		  for (; cur_str_idx <= max; ++cur_str_idx)
		    if (mctx->state_log[cur_str_idx] != NULL)
		      break;
		  if (cur_str_idx > max)
		    break;
		}
	    }
	}

      if (cur_state != NULL && cur_state->halt)
	{
	  /* Reached at a halt state.
	     Check the halt state can satisfy the current context.  */
	  if (!cur_state->has_constraint
	      || check_halt_state_context (preg, cur_state, mctx,
					   re_string_cur_idx (mctx->input)))
	    {
	      /* We found an appropriate halt state.  */
	      match_last = re_string_cur_idx (mctx->input);
	      match = 1;
	      if (!fl_longest_match)
		break;
	    }
	}
   }
  return match_last;
}

/* Check NODE match the current context.  */

static int check_halt_node_context (dfa, node, context)
    const re_dfa_t *dfa;
    int node;
    unsigned int context;
{
  re_token_type_t type = dfa->nodes[node].type;
  unsigned int constraint = dfa->nodes[node].constraint;
  if (type != END_OF_RE)
    return 0;
  if (!constraint)
    return 1;
  if (NOT_SATISFY_NEXT_CONSTRAINT (constraint, context))
    return 0;
  return 1;
}

/* Check the halt state STATE match the current context.
   Return 0 if not match, if the node, STATE has, is a halt node and
   match the context, return the node.  */

static int
check_halt_state_context (preg, state, mctx, idx)
    const regex_t *preg;
    const re_dfastate_t *state;
    const re_match_context_t *mctx;
    int idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;
  unsigned int context;
#ifdef DEBUG
  assert (state->halt);
#endif
  context = re_string_context_at (mctx->input, idx, mctx->eflags,
				  preg->newline_anchor);
  for (i = 0; i < state->nodes.nelem; ++i)
    if (check_halt_node_context (dfa, state->nodes.elems[i], context))
      return state->nodes.elems[i];
  return 0;
}

/* Compute the next node to which "NFA" transit from NODE("NFA" is a NFA
   corresponding to the DFA).
   Return the destination node, and update EPS_VIA_NODES, return -1 in case
   of errors.  */

static int
proceed_next_node (preg, nregs, regs, mctx, pidx, node, eps_via_nodes, fs)
    const regex_t *preg;
    regmatch_t *regs;
    const re_match_context_t *mctx;
    int nregs, *pidx, node;
    re_node_set *eps_via_nodes;
    struct re_fail_stack_t *fs;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int i, err, dest_node;
  dest_node = -1;
  if (IS_EPSILON_NODE (dfa->nodes[node].type))
    {
      re_node_set *cur_nodes = &mctx->state_log[*pidx]->nodes;
      int ndest, dest_nodes[2];
      err = re_node_set_insert (eps_via_nodes, node);
      if (BE (err < 0, 0))
	return -1;
      /* Pick up valid destinations.  */
      dest_nodes[0] = dest_nodes[1] = 0; /* -Wall on gcc4 */
      for (ndest = 0, i = 0; i < dfa->edests[node].nelem; ++i)
	{
	  int candidate = dfa->edests[node].elems[i];
	  if (!re_node_set_contains (cur_nodes, candidate))
	    continue;
	  dest_nodes[0] = (ndest == 0) ? candidate : dest_nodes[0];
	  dest_nodes[1] = (ndest == 1) ? candidate : dest_nodes[1];
	  ++ndest;
	}
      if (ndest <= 1)
	return ndest == 0 ? -1 : (ndest == 1 ? dest_nodes[0] : 0);
      /* In order to avoid infinite loop like "(a*)*".  */
      if (re_node_set_contains (eps_via_nodes, dest_nodes[0]))
	return dest_nodes[1];
      if (fs != NULL)
	push_fail_stack (fs, *pidx, dest_nodes, nregs, regs, eps_via_nodes);
      return dest_nodes[0];
    }
  else
    {
      int naccepted = 0;
      re_token_type_t type = dfa->nodes[node].type;

#ifdef RE_ENABLE_I18N
      if (ACCEPT_MB_NODE (type))
	naccepted = check_node_accept_bytes (preg, node, mctx->input, *pidx);
      else
#endif /* RE_ENABLE_I18N */
      if (type == OP_BACK_REF)
	{
	  int subexp_idx = dfa->nodes[node].opr.idx;
	  naccepted = regs[subexp_idx].rm_eo - regs[subexp_idx].rm_so;
	  if (fs != NULL)
	    {
	      if (regs[subexp_idx].rm_so == -1 || regs[subexp_idx].rm_eo == -1)
		return -1;
	      else if (naccepted)
		{
		  char *buf = (char *) re_string_get_buffer (mctx->input);
		  if (memcmp (buf + regs[subexp_idx].rm_so, buf + *pidx,
			      naccepted) != 0)
		    return -1;
		}
	    }

	  if (naccepted == 0)
	    {
	      err = re_node_set_insert (eps_via_nodes, node);
	      if (BE (err < 0, 0))
		return -2;
	      dest_node = dfa->edests[node].elems[0];
	      if (re_node_set_contains (&mctx->state_log[*pidx]->nodes,
					dest_node))
		return dest_node;
	    }
	}

      if (naccepted != 0
	  || check_node_accept (preg, dfa->nodes + node, mctx, *pidx))
	{
	  dest_node = dfa->nexts[node];
	  *pidx = (naccepted == 0) ? *pidx + 1 : *pidx + naccepted;
	  if (fs && (*pidx > mctx->match_last || mctx->state_log[*pidx] == NULL
		     || !re_node_set_contains (&mctx->state_log[*pidx]->nodes,
					       dest_node)))
	    return -1;
	  re_node_set_empty (eps_via_nodes);
	  return dest_node;
	}
    }
  return -1;
}

static reg_errcode_t
push_fail_stack (fs, str_idx, dests, nregs, regs, eps_via_nodes)
     struct re_fail_stack_t *fs;
     int str_idx, *dests, nregs;
     regmatch_t *regs;
     re_node_set *eps_via_nodes;
{
  reg_errcode_t err;
  int num = fs->num++;
  if (fs->num == fs->alloc)
    {
      struct re_fail_stack_ent_t *new_array;
      fs->alloc *= 2;
      new_array = realloc (fs->stack, (sizeof (struct re_fail_stack_ent_t)
				       * fs->alloc));
      if (new_array == NULL)
	return REG_ESPACE;
      fs->stack = new_array;
    }
  fs->stack[num].idx = str_idx;
  fs->stack[num].node = dests[1];
  fs->stack[num].regs = re_malloc (regmatch_t, nregs);
  memcpy (fs->stack[num].regs, regs, sizeof (regmatch_t) * nregs);
  err = re_node_set_init_copy (&fs->stack[num].eps_via_nodes, eps_via_nodes);
  return err;
}

static int
pop_fail_stack (fs, pidx, nregs, regs, eps_via_nodes)
     struct re_fail_stack_t *fs;
     int *pidx, nregs;
     regmatch_t *regs;
     re_node_set *eps_via_nodes;
{
  int num = --fs->num;
  assert (num >= 0);
 *pidx = fs->stack[num].idx;
  memcpy (regs, fs->stack[num].regs, sizeof (regmatch_t) * nregs);
  re_node_set_free (eps_via_nodes);
  re_free (fs->stack[num].regs);
  *eps_via_nodes = fs->stack[num].eps_via_nodes;
  return fs->stack[num].node;
}

/* Set the positions where the subexpressions are starts/ends to registers
   PMATCH.
   Note: We assume that pmatch[0] is already set, and
   pmatch[i].rm_so == pmatch[i].rm_eo == -1 for 0 < i < nmatch.  */

static reg_errcode_t
set_regs (preg, mctx, nmatch, pmatch, fl_backtrack)
     const regex_t *preg;
     const re_match_context_t *mctx;
     size_t nmatch;
     regmatch_t *pmatch;
     int fl_backtrack;
{
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int idx, cur_node, real_nmatch;
  re_node_set eps_via_nodes;
  struct re_fail_stack_t *fs;
  struct re_fail_stack_t fs_body = {0, 2, NULL};
#ifdef DEBUG
  assert (nmatch > 1);
  assert (mctx->state_log != NULL);
#endif
  if (fl_backtrack)
    {
      fs = &fs_body;
      fs->stack = re_malloc (struct re_fail_stack_ent_t, fs->alloc);
    }
  else
    fs = NULL;
  cur_node = dfa->init_node;
  real_nmatch = (nmatch <= preg->re_nsub) ? nmatch : preg->re_nsub + 1;
  re_node_set_init_empty (&eps_via_nodes);
  for (idx = pmatch[0].rm_so; idx <= pmatch[0].rm_eo ;)
    {
      update_regs (dfa, pmatch, cur_node, idx, real_nmatch);
      if (idx == pmatch[0].rm_eo && cur_node == mctx->last_node)
	{
	  int reg_idx;
	  if (fs)
	    {
	      for (reg_idx = 0; reg_idx < nmatch; ++reg_idx)
		if (pmatch[reg_idx].rm_so > -1 && pmatch[reg_idx].rm_eo == -1)
		  break;
	      if (reg_idx == nmatch)
		{
		  re_node_set_free (&eps_via_nodes);
		  return free_fail_stack_return (fs);
		}
	      cur_node = pop_fail_stack (fs, &idx, nmatch, pmatch,
					 &eps_via_nodes);
	    }
	  else
	    {
	      re_node_set_free (&eps_via_nodes);
	      return REG_NOERROR;
	    }
	}

      /* Proceed to next node.  */
      cur_node = proceed_next_node (preg, nmatch, pmatch, mctx, &idx, cur_node,
				    &eps_via_nodes, fs);

      if (BE (cur_node < 0, 0))
	{
	  if (cur_node == -2)
	    return REG_ESPACE;
	  if (fs)
	    cur_node = pop_fail_stack (fs, &idx, nmatch, pmatch,
				       &eps_via_nodes);
	  else
	    {
	      re_node_set_free (&eps_via_nodes);
	      return REG_NOMATCH;
	    }
	}
    }
  re_node_set_free (&eps_via_nodes);
  return free_fail_stack_return (fs);
}

static reg_errcode_t
free_fail_stack_return (fs)
     struct re_fail_stack_t *fs;
{
  if (fs)
    {
      int fs_idx;
      for (fs_idx = 0; fs_idx < fs->num; ++fs_idx)
	{
	  re_node_set_free (&fs->stack[fs_idx].eps_via_nodes);
	  re_free (fs->stack[fs_idx].regs);
	}
      re_free (fs->stack);
    }
  return REG_NOERROR;
}

static void
update_regs (dfa, pmatch, cur_node, cur_idx, nmatch)
     re_dfa_t *dfa;
     regmatch_t *pmatch;
     int cur_node, cur_idx, nmatch;
{
  int type = dfa->nodes[cur_node].type;
  int reg_num;
  if (type != OP_OPEN_SUBEXP && type != OP_CLOSE_SUBEXP)
    return;
  reg_num = dfa->nodes[cur_node].opr.idx + 1;
  if (reg_num >= nmatch)
    return;
  if (type == OP_OPEN_SUBEXP)
    {
      /* We are at the first node of this sub expression.  */
      pmatch[reg_num].rm_so = cur_idx;
      pmatch[reg_num].rm_eo = -1;
    }
  else if (type == OP_CLOSE_SUBEXP)
    /* We are at the first node of this sub expression.  */
    pmatch[reg_num].rm_eo = cur_idx;
}

#define NUMBER_OF_STATE 1

/* This function checks the STATE_LOG from the SCTX->last_str_idx to 0
   and sift the nodes in each states according to the following rules.
   Updated state_log will be wrote to STATE_LOG.

   Rules: We throw away the Node `a' in the STATE_LOG[STR_IDX] if...
     1. When STR_IDX == MATCH_LAST(the last index in the state_log):
	If `a' isn't the LAST_NODE and `a' can't epsilon transit to
	the LAST_NODE, we throw away the node `a'.
     2. When 0 <= STR_IDX < MATCH_LAST and `a' accepts
	string `s' and transit to `b':
	i. If 'b' isn't in the STATE_LOG[STR_IDX+strlen('s')], we throw
	   away the node `a'.
	ii. If 'b' is in the STATE_LOG[STR_IDX+strlen('s')] but 'b' is
	    thrown away, we throw away the node `a'.
     3. When 0 <= STR_IDX < MATCH_LAST and 'a' epsilon transit to 'b':
	i. If 'b' isn't in the STATE_LOG[STR_IDX], we throw away the
	   node `a'.
	ii. If 'b' is in the STATE_LOG[STR_IDX] but 'b' is thrown away,
	    we throw away the node `a'.  */

#define STATE_NODE_CONTAINS(state,node) \
  ((state) != NULL && re_node_set_contains (&(state)->nodes, node))

static reg_errcode_t
sift_states_backward (preg, mctx, sctx)
     const regex_t *preg;
     re_match_context_t *mctx;
     re_sift_context_t *sctx;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int null_cnt = 0;
  int str_idx = sctx->last_str_idx;
  re_node_set cur_dest;
  re_node_set *cur_src; /* Points the state_log[str_idx]->nodes  */

#ifdef DEBUG
  assert (mctx->state_log != NULL && mctx->state_log[str_idx] != NULL);
#endif
  cur_src = &mctx->state_log[str_idx]->nodes;

  /* Build sifted state_log[str_idx].  It has the nodes which can epsilon
     transit to the last_node and the last_node itself.  */
  err = re_node_set_init_1 (&cur_dest, sctx->last_node);
  if (BE (err != REG_NOERROR, 0))
    return err;
  err = update_cur_sifted_state (preg, mctx, sctx, str_idx, &cur_dest);
  if (BE (err != REG_NOERROR, 0))
    goto free_return;

  /* Then check each states in the state_log.  */
  while (str_idx > 0)
    {
      int i, ret;
      /* Update counters.  */
      null_cnt = (sctx->sifted_states[str_idx] == NULL) ? null_cnt + 1 : 0;
      if (null_cnt > mctx->max_mb_elem_len)
	{
	  memset (sctx->sifted_states, '\0',
		  sizeof (re_dfastate_t *) * str_idx);
	  re_node_set_free (&cur_dest);
	  return REG_NOERROR;
	}
      re_node_set_empty (&cur_dest);
      --str_idx;
      cur_src = ((mctx->state_log[str_idx] == NULL) ? &empty_set
		 : &mctx->state_log[str_idx]->nodes);

      /* Then build the next sifted state.
	 We build the next sifted state on `cur_dest', and update
	 `sifted_states[str_idx]' with `cur_dest'.
	 Note:
	 `cur_dest' is the sifted state from `state_log[str_idx + 1]'.
	 `cur_src' points the node_set of the old `state_log[str_idx]'.  */
      for (i = 0; i < cur_src->nelem; i++)
	{
	  int prev_node = cur_src->elems[i];
	  int naccepted = 0;
	  re_token_type_t type = dfa->nodes[prev_node].type;

	  if (IS_EPSILON_NODE (type))
	    continue;
#ifdef RE_ENABLE_I18N
	  /* If the node may accept `multi byte'.  */
	  if (ACCEPT_MB_NODE (type))
	    naccepted = sift_states_iter_mb (preg, mctx, sctx, prev_node,
					     str_idx, sctx->last_str_idx);

#endif /* RE_ENABLE_I18N */
	  /* We don't check backreferences here.
	     See update_cur_sifted_state().  */

	  if (!naccepted
	      && check_node_accept (preg, dfa->nodes + prev_node, mctx,
				    str_idx)
	      && STATE_NODE_CONTAINS (sctx->sifted_states[str_idx + 1],
				      dfa->nexts[prev_node]))
	    naccepted = 1;

	  if (naccepted == 0)
	    continue;

	  if (sctx->limits.nelem)
	    {
	      int to_idx = str_idx + naccepted;
	      if (check_dst_limits (dfa, &sctx->limits, mctx,
				    dfa->nexts[prev_node], to_idx,
				    prev_node, str_idx))
		continue;
	    }
	  ret = re_node_set_insert (&cur_dest, prev_node);
	  if (BE (ret == -1, 0))
	    {
	      err = REG_ESPACE;
	      goto free_return;
	    }
	}

      /* Add all the nodes which satisfy the following conditions:
	 - It can epsilon transit to a node in CUR_DEST.
	 - It is in CUR_SRC.
	 And update state_log.  */
      err = update_cur_sifted_state (preg, mctx, sctx, str_idx, &cur_dest);
      if (BE (err != REG_NOERROR, 0))
	goto free_return;
    }
  err = REG_NOERROR;
 free_return:
  re_node_set_free (&cur_dest);
  return err;
}

/* Helper functions.  */

static reg_errcode_t
clean_state_log_if_need (mctx, next_state_log_idx)
    re_match_context_t *mctx;
    int next_state_log_idx;
{
  int top = mctx->state_log_top;

  if (next_state_log_idx >= mctx->input->bufs_len
      || (next_state_log_idx >= mctx->input->valid_len
	  && mctx->input->valid_len < mctx->input->len))
    {
      reg_errcode_t err;
      err = extend_buffers (mctx);
      if (BE (err != REG_NOERROR, 0))
	return err;
    }

  if (top < next_state_log_idx)
    {
      memset (mctx->state_log + top + 1, '\0',
	      sizeof (re_dfastate_t *) * (next_state_log_idx - top));
      mctx->state_log_top = next_state_log_idx;
    }
  return REG_NOERROR;
}

static reg_errcode_t
merge_state_array (dfa, dst, src, num)
     re_dfa_t *dfa;
     re_dfastate_t **dst;
     re_dfastate_t **src;
     int num;
{
  int st_idx;
  reg_errcode_t err;
  for (st_idx = 0; st_idx < num; ++st_idx)
    {
      if (dst[st_idx] == NULL)
	dst[st_idx] = src[st_idx];
      else if (src[st_idx] != NULL)
	{
	  re_node_set merged_set;
	  err = re_node_set_init_union (&merged_set, &dst[st_idx]->nodes,
					&src[st_idx]->nodes);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	  dst[st_idx] = re_acquire_state (&err, dfa, &merged_set);
	  re_node_set_free (&merged_set);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
    }
  return REG_NOERROR;
}

static reg_errcode_t
update_cur_sifted_state (preg, mctx, sctx, str_idx, dest_nodes)
     const regex_t *preg;
     re_match_context_t *mctx;
     re_sift_context_t *sctx;
     int str_idx;
     re_node_set *dest_nodes;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  const re_node_set *candidates;
  candidates = ((mctx->state_log[str_idx] == NULL) ? &empty_set
		: &mctx->state_log[str_idx]->nodes);

  /* At first, add the nodes which can epsilon transit to a node in
     DEST_NODE.  */
  if (dest_nodes->nelem)
    {
      err = add_epsilon_src_nodes (dfa, dest_nodes, candidates);
      if (BE (err != REG_NOERROR, 0))
	return err;
    }

  /* Then, check the limitations in the current sift_context.  */
  if (dest_nodes->nelem && sctx->limits.nelem)
    {
      err = check_subexp_limits (dfa, dest_nodes, candidates, &sctx->limits,
				 mctx->bkref_ents, str_idx);
      if (BE (err != REG_NOERROR, 0))
	return err;
    }

  /* Update state_log.  */
  sctx->sifted_states[str_idx] = re_acquire_state (&err, dfa, dest_nodes);
  if (BE (sctx->sifted_states[str_idx] == NULL && err != REG_NOERROR, 0))
    return err;

  if ((mctx->state_log[str_idx] != NULL
       && mctx->state_log[str_idx]->has_backref))
    {
      err = sift_states_bkref (preg, mctx, sctx, str_idx, dest_nodes);
      if (BE (err != REG_NOERROR, 0))
	return err;
    }
  return REG_NOERROR;
}

static reg_errcode_t
add_epsilon_src_nodes (dfa, dest_nodes, candidates)
     re_dfa_t *dfa;
     re_node_set *dest_nodes;
     const re_node_set *candidates;
{
  reg_errcode_t err;
  int src_idx;
  re_node_set src_copy;

  err = re_node_set_init_copy (&src_copy, dest_nodes);
  if (BE (err != REG_NOERROR, 0))
    return err;
  for (src_idx = 0; src_idx < src_copy.nelem; ++src_idx)
    {
      err = re_node_set_add_intersect (dest_nodes, candidates,
				       dfa->inveclosures
				       + src_copy.elems[src_idx]);
      if (BE (err != REG_NOERROR, 0))
	{
	  re_node_set_free (&src_copy);
	  return err;
	}
    }
  re_node_set_free (&src_copy);
  return REG_NOERROR;
}

static reg_errcode_t
sub_epsilon_src_nodes (dfa, node, dest_nodes, candidates)
     re_dfa_t *dfa;
     int node;
     re_node_set *dest_nodes;
     const re_node_set *candidates;
{
    int ecl_idx;
    reg_errcode_t err;
    re_node_set *inv_eclosure = dfa->inveclosures + node;
    re_node_set except_nodes;
    re_node_set_init_empty (&except_nodes);
    for (ecl_idx = 0; ecl_idx < inv_eclosure->nelem; ++ecl_idx)
      {
	int cur_node = inv_eclosure->elems[ecl_idx];
	if (cur_node == node)
	  continue;
	if (IS_EPSILON_NODE (dfa->nodes[cur_node].type))
	  {
	    int edst1 = dfa->edests[cur_node].elems[0];
	    int edst2 = ((dfa->edests[cur_node].nelem > 1)
			 ? dfa->edests[cur_node].elems[1] : -1);
	    if ((!re_node_set_contains (inv_eclosure, edst1)
		 && re_node_set_contains (dest_nodes, edst1))
		|| (edst2 > 0
		    && !re_node_set_contains (inv_eclosure, edst2)
		    && re_node_set_contains (dest_nodes, edst2)))
	      {
		err = re_node_set_add_intersect (&except_nodes, candidates,
						 dfa->inveclosures + cur_node);
		if (BE (err != REG_NOERROR, 0))
		  {
		    re_node_set_free (&except_nodes);
		    return err;
		  }
	      }
	  }
      }
    for (ecl_idx = 0; ecl_idx < inv_eclosure->nelem; ++ecl_idx)
      {
	int cur_node = inv_eclosure->elems[ecl_idx];
	if (!re_node_set_contains (&except_nodes, cur_node))
	  {
	    int idx = re_node_set_contains (dest_nodes, cur_node) - 1;
	    re_node_set_remove_at (dest_nodes, idx);
	  }
      }
    re_node_set_free (&except_nodes);
    return REG_NOERROR;
}

static int
check_dst_limits (dfa, limits, mctx, dst_node, dst_idx, src_node, src_idx)
     re_dfa_t *dfa;
     re_node_set *limits;
     re_match_context_t *mctx;
     int dst_node, dst_idx, src_node, src_idx;
{
  int lim_idx, src_pos, dst_pos;

  for (lim_idx = 0; lim_idx < limits->nelem; ++lim_idx)
    {
      int subexp_idx;
      struct re_backref_cache_entry *ent;
      ent = mctx->bkref_ents + limits->elems[lim_idx];
      subexp_idx = dfa->nodes[ent->node].opr.idx - 1;

      dst_pos = check_dst_limits_calc_pos (dfa, mctx, limits->elems[lim_idx],
					   dfa->eclosures + dst_node,
					   subexp_idx, dst_node, dst_idx);
      src_pos = check_dst_limits_calc_pos (dfa, mctx, limits->elems[lim_idx],
					   dfa->eclosures + src_node,
					   subexp_idx, src_node, src_idx);

      /* In case of:
	 <src> <dst> ( <subexp> )
	 ( <subexp> ) <src> <dst>
	 ( <subexp1> <src> <subexp2> <dst> <subexp3> )  */
      if (src_pos == dst_pos)
	continue; /* This is unrelated limitation.  */
      else
	return 1;
    }
  return 0;
}

static int
check_dst_limits_calc_pos (dfa, mctx, limit, eclosures, subexp_idx, from_node,
			   str_idx)
     re_dfa_t *dfa;
     re_match_context_t *mctx;
     re_node_set *eclosures;
     int limit, subexp_idx, from_node, str_idx;
{
  struct re_backref_cache_entry *lim = mctx->bkref_ents + limit;
      int node_idx;

  /* If we are outside the range of the subexpression, return -1 or 1.  */
  if (str_idx < lim->subexp_from)
    return -1;

  if (lim->subexp_to < str_idx)
    return 1;

  /* If we are within the subexpression, return 0.  */
  if (str_idx != lim->subexp_from && str_idx != lim->subexp_to)
    return 0;

  /* Else, we are on the boundary: examine the nodes on the epsilon
     closure.  */
      for (node_idx = 0; node_idx < eclosures->nelem; ++node_idx)
	{
	  int node = eclosures->elems[node_idx];
      switch (dfa->nodes[node].type)
	{
	case OP_BACK_REF:
	    {
	      int bi = search_cur_bkref_entry (mctx, str_idx);
	      for (; bi < mctx->nbkref_ents; ++bi)
		{
		  struct re_backref_cache_entry *ent = mctx->bkref_ents + bi;
		int dst, cpos;

		/* If this backreference goes beyond the point we're
		   examining, don't go any further.  */
		  if (ent->str_idx > str_idx)
		    break;

		if (ent->node != node || ent->subexp_from != ent->subexp_to)
		  continue;

		/* Recurse trying to reach the OP_OPEN_SUBEXP and
		   OP_CLOSE_SUBEXP cases below.  But, if the
		   destination node is the same node as the source
		   node, don't recurse because it would cause an
		   infinite loop: a regex that exhibits this behavior
		   is ()\1*\1*  */
		      dst = dfa->edests[node].elems[0];
		if (dst == from_node)
		  {
		    if (str_idx == lim->subexp_from)
		      return -1;
		    else /* if (str_idx == lim->subexp_to) */
		      return 0;
		  }

		      cpos = check_dst_limits_calc_pos (dfa, mctx, limit,
							dfa->eclosures + dst,
							subexp_idx, dst,
							str_idx);

		if (cpos == -1 && str_idx == lim->subexp_from)
		  return -1;

		if (cpos == 0 /* && str_idx == lim->lim->subexp_to */)
		  return 0;
	    }
	      break;
	    }

	case OP_OPEN_SUBEXP:
	  if (str_idx == lim->subexp_from && subexp_idx == dfa->nodes[node].opr.idx)
	    return -1;
	  break;

	case OP_CLOSE_SUBEXP:
	  if (str_idx == lim->subexp_to && subexp_idx == dfa->nodes[node].opr.idx)
	    return 0;
	  break;

	default:
	    break;
	}
    }

  if (str_idx == lim->subexp_to)
    return 1;
  else
    return 0;
}

/* Check the limitations of sub expressions LIMITS, and remove the nodes
   which are against limitations from DEST_NODES. */

static reg_errcode_t
check_subexp_limits (dfa, dest_nodes, candidates, limits, bkref_ents, str_idx)
     re_dfa_t *dfa;
     re_node_set *dest_nodes;
     const re_node_set *candidates;
     re_node_set *limits;
     struct re_backref_cache_entry *bkref_ents;
     int str_idx;
{
  reg_errcode_t err;
  int node_idx, lim_idx;

  for (lim_idx = 0; lim_idx < limits->nelem; ++lim_idx)
    {
      int subexp_idx;
      struct re_backref_cache_entry *ent;
      ent = bkref_ents + limits->elems[lim_idx];

      if (str_idx <= ent->subexp_from || ent->str_idx < str_idx)
	continue; /* This is unrelated limitation.  */

      subexp_idx = dfa->nodes[ent->node].opr.idx - 1;
      if (ent->subexp_to == str_idx)
	{
	  int ops_node = -1;
	  int cls_node = -1;
	  for (node_idx = 0; node_idx < dest_nodes->nelem; ++node_idx)
	    {
	      int node = dest_nodes->elems[node_idx];
	      re_token_type_t type = dfa->nodes[node].type;
	      if (type == OP_OPEN_SUBEXP
		  && subexp_idx == dfa->nodes[node].opr.idx)
		ops_node = node;
	      else if (type == OP_CLOSE_SUBEXP
		       && subexp_idx == dfa->nodes[node].opr.idx)
		cls_node = node;
	    }

	  /* Check the limitation of the open subexpression.  */
	  /* Note that (ent->subexp_to = str_idx != ent->subexp_from).  */
	  if (ops_node >= 0)
	    {
	      err = sub_epsilon_src_nodes (dfa, ops_node, dest_nodes,
					   candidates);
	      if (BE (err != REG_NOERROR, 0))
		return err;
	    }

	  /* Check the limitation of the close subexpression.  */
	  if (cls_node >= 0)
	    for (node_idx = 0; node_idx < dest_nodes->nelem; ++node_idx)
	      {
		int node = dest_nodes->elems[node_idx];
		if (!re_node_set_contains (dfa->inveclosures + node,
					   cls_node)
		    && !re_node_set_contains (dfa->eclosures + node,
					      cls_node))
		  {
		    /* It is against this limitation.
		       Remove it form the current sifted state.  */
		    err = sub_epsilon_src_nodes (dfa, node, dest_nodes,
						 candidates);
		    if (BE (err != REG_NOERROR, 0))
		      return err;
		    --node_idx;
		  }
	      }
	}
      else /* (ent->subexp_to != str_idx)  */
	{
	  for (node_idx = 0; node_idx < dest_nodes->nelem; ++node_idx)
	    {
	      int node = dest_nodes->elems[node_idx];
	      re_token_type_t type = dfa->nodes[node].type;
	      if (type == OP_CLOSE_SUBEXP || type == OP_OPEN_SUBEXP)
		{
		  if (subexp_idx != dfa->nodes[node].opr.idx)
		    continue;
		  if ((type == OP_CLOSE_SUBEXP && ent->subexp_to != str_idx)
		      || (type == OP_OPEN_SUBEXP))
		    {
		      /* It is against this limitation.
			 Remove it form the current sifted state.  */
		      err = sub_epsilon_src_nodes (dfa, node, dest_nodes,
						   candidates);
		      if (BE (err != REG_NOERROR, 0))
			return err;
		    }
		}
	    }
	}
    }
  return REG_NOERROR;
}

static reg_errcode_t
sift_states_bkref (preg, mctx, sctx, str_idx, dest_nodes)
     const regex_t *preg;
     re_match_context_t *mctx;
     re_sift_context_t *sctx;
     int str_idx;
     re_node_set *dest_nodes;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *)preg->buffer;
  int node_idx, node;
  re_sift_context_t local_sctx;
  const re_node_set *candidates;
  candidates = ((mctx->state_log[str_idx] == NULL) ? &empty_set
		: &mctx->state_log[str_idx]->nodes);
  local_sctx.sifted_states = NULL; /* Mark that it hasn't been initialized.  */

  for (node_idx = 0; node_idx < candidates->nelem; ++node_idx)
    {
      int cur_bkref_idx = re_string_cur_idx (mctx->input);
      re_token_type_t type;
      node = candidates->elems[node_idx];
      type = dfa->nodes[node].type;
      if (node == sctx->cur_bkref && str_idx == cur_bkref_idx)
	continue;
      /* Avoid infinite loop for the REs like "()\1+".  */
      if (node == sctx->last_node && str_idx == sctx->last_str_idx)
	continue;
      if (type == OP_BACK_REF)
	{
	  int enabled_idx = search_cur_bkref_entry (mctx, str_idx);
	  for (; enabled_idx < mctx->nbkref_ents; ++enabled_idx)
	    {
	      int disabled_idx, subexp_len, to_idx, dst_node;
	      struct re_backref_cache_entry *entry;
	      entry = mctx->bkref_ents + enabled_idx;
	      if (entry->str_idx > str_idx)
		break;
	      if (entry->node != node)
		  continue;
	      subexp_len = entry->subexp_to - entry->subexp_from;
	      to_idx = str_idx + subexp_len;
	      dst_node = (subexp_len ? dfa->nexts[node]
			  : dfa->edests[node].elems[0]);

	      if (to_idx > sctx->last_str_idx
		  || sctx->sifted_states[to_idx] == NULL
		  || !STATE_NODE_CONTAINS (sctx->sifted_states[to_idx],
					   dst_node)
		  || check_dst_limits (dfa, &sctx->limits, mctx, node,
				       str_idx, dst_node, to_idx))
		continue;
		{
		  re_dfastate_t *cur_state;
		  entry->flag = 0;
		  for (disabled_idx = enabled_idx + 1;
		       disabled_idx < mctx->nbkref_ents; ++disabled_idx)
		    {
		      struct re_backref_cache_entry *entry2;
		      entry2 = mctx->bkref_ents + disabled_idx;
		      if (entry2->str_idx > str_idx)
			break;
		      entry2->flag = (entry2->node == node) ? 1 : entry2->flag;
		    }

		  if (local_sctx.sifted_states == NULL)
		    {
		      local_sctx = *sctx;
		      err = re_node_set_init_copy (&local_sctx.limits,
						   &sctx->limits);
		      if (BE (err != REG_NOERROR, 0))
			goto free_return;
		    }
		  local_sctx.last_node = node;
		  local_sctx.last_str_idx = str_idx;
		  err = re_node_set_insert (&local_sctx.limits, enabled_idx);
		  if (BE (err < 0, 0))
		    {
		      err = REG_ESPACE;
		      goto free_return;
		    }
		  cur_state = local_sctx.sifted_states[str_idx];
		  err = sift_states_backward (preg, mctx, &local_sctx);
		  if (BE (err != REG_NOERROR, 0))
		    goto free_return;
		  if (sctx->limited_states != NULL)
		    {
		      err = merge_state_array (dfa, sctx->limited_states,
					       local_sctx.sifted_states,
					       str_idx + 1);
		      if (BE (err != REG_NOERROR, 0))
			goto free_return;
		    }
		  local_sctx.sifted_states[str_idx] = cur_state;
		  re_node_set_remove (&local_sctx.limits, enabled_idx);
		  /* We must not use the variable entry here, since
		     mctx->bkref_ents might be realloced.  */
		  mctx->bkref_ents[enabled_idx].flag = 1;
		}
	    }
	  enabled_idx = search_cur_bkref_entry (mctx, str_idx);
	  for (; enabled_idx < mctx->nbkref_ents; ++enabled_idx)
	    {
	      struct re_backref_cache_entry *entry;
	      entry = mctx->bkref_ents + enabled_idx;
	      if (entry->str_idx > str_idx)
		break;
	      if (entry->node == node)
		entry->flag = 0;
	    }
	}
    }
  err = REG_NOERROR;
 free_return:
  if (local_sctx.sifted_states != NULL)
    {
      re_node_set_free (&local_sctx.limits);
    }

  return err;
}


#ifdef RE_ENABLE_I18N
static int
sift_states_iter_mb (preg, mctx, sctx, node_idx, str_idx, max_str_idx)
    const regex_t *preg;
    const re_match_context_t *mctx;
    re_sift_context_t *sctx;
    int node_idx, str_idx, max_str_idx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int naccepted;
  /* Check the node can accept `multi byte'.  */
  naccepted = check_node_accept_bytes (preg, node_idx, mctx->input, str_idx);
  if (naccepted > 0 && str_idx + naccepted <= max_str_idx &&
      !STATE_NODE_CONTAINS (sctx->sifted_states[str_idx + naccepted],
			    dfa->nexts[node_idx]))
    /* The node can't accept the `multi byte', or the
       destination was already thrown away, then the node
       could't accept the current input `multi byte'.   */
    naccepted = 0;
  /* Otherwise, it is sure that the node could accept
     `naccepted' bytes input.  */
  return naccepted;
}
#endif /* RE_ENABLE_I18N */


/* Functions for state transition.  */

/* Return the next state to which the current state STATE will transit by
   accepting the current input byte, and update STATE_LOG if necessary.
   If STATE can accept a multibyte char/collating element/back reference
   update the destination of STATE_LOG.  */

static re_dfastate_t *
transit_state (err, preg, mctx, state)
     reg_errcode_t *err;
     const regex_t *preg;
     re_match_context_t *mctx;
     re_dfastate_t *state;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_dfastate_t **trtable, *next_state;
  unsigned char ch;
  int cur_idx;

  if (re_string_cur_idx (mctx->input) + 1 >= mctx->input->bufs_len
      || (re_string_cur_idx (mctx->input) + 1 >= mctx->input->valid_len
	  && mctx->input->valid_len < mctx->input->len))
    {
      *err = extend_buffers (mctx);
      if (BE (*err != REG_NOERROR, 0))
	return NULL;
    }

  *err = REG_NOERROR;
  if (state == NULL)
    {
      next_state = state;
      re_string_skip_bytes (mctx->input, 1);
    }
  else
    {
#ifdef RE_ENABLE_I18N
      /* If the current state can accept multibyte.  */
      if (state->accept_mb)
	{
	  *err = transit_state_mb (preg, state, mctx);
	  if (BE (*err != REG_NOERROR, 0))
	    return NULL;
	}
#endif /* RE_ENABLE_I18N */

      /* Then decide the next state with the single byte.  */
      if (1)
	{
	  /* Use transition table  */
	  ch = re_string_fetch_byte (mctx->input);
	  trtable = state->trtable;
	  if (trtable == NULL)
	    {
	      trtable = build_trtable (preg, state);
	      if (trtable == NULL)
		{
		  *err = REG_ESPACE;
		  return NULL;
		}
	    }
	  if (BE (state->word_trtable, 0))
	    {
	      unsigned int context;
	      context
		= re_string_context_at (mctx->input,
					re_string_cur_idx (mctx->input) - 1,
					mctx->eflags, preg->newline_anchor);
	      if (IS_WORD_CONTEXT (context))
		next_state = trtable[ch + SBC_MAX];
	      else
		next_state = trtable[ch];
	    }
	  else
	    next_state = trtable[ch];
	}
#if 0
      else
	{
	  /* don't use transition table  */
	  next_state = transit_state_sb (err, preg, state, mctx);
	  if (BE (next_state == NULL && err != REG_NOERROR, 0))
	    return NULL;
	}
#endif
    }

  cur_idx = re_string_cur_idx (mctx->input);
  /* Update the state_log if we need.  */
  if (mctx->state_log != NULL)
    {
      if (cur_idx > mctx->state_log_top)
	{
	  mctx->state_log[cur_idx] = next_state;
	  mctx->state_log_top = cur_idx;
	}
      else if (mctx->state_log[cur_idx] == 0)
	{
	  mctx->state_log[cur_idx] = next_state;
	}
      else
	{
	  re_dfastate_t *pstate;
	  unsigned int context;
	  re_node_set next_nodes, *log_nodes, *table_nodes = NULL;
	  /* If (state_log[cur_idx] != 0), it implies that cur_idx is
	     the destination of a multibyte char/collating element/
	     back reference.  Then the next state is the union set of
	     these destinations and the results of the transition table.  */
	  pstate = mctx->state_log[cur_idx];
	  log_nodes = pstate->entrance_nodes;
	  if (next_state != NULL)
	    {
	      table_nodes = next_state->entrance_nodes;
	      *err = re_node_set_init_union (&next_nodes, table_nodes,
					     log_nodes);
	      if (BE (*err != REG_NOERROR, 0))
		return NULL;
	    }
	  else
	    next_nodes = *log_nodes;
	  /* Note: We already add the nodes of the initial state,
		   then we don't need to add them here.  */

	  context = re_string_context_at (mctx->input,
					  re_string_cur_idx (mctx->input) - 1,
					  mctx->eflags, preg->newline_anchor);
	  next_state = mctx->state_log[cur_idx]
	    = re_acquire_state_context (err, dfa, &next_nodes, context);
	  /* We don't need to check errors here, since the return value of
	     this function is next_state and ERR is already set.  */

	  if (table_nodes != NULL)
	    re_node_set_free (&next_nodes);
	}
    }

  if (BE (dfa->nbackref, 0) && next_state != NULL)
    {
      /* Check OP_OPEN_SUBEXP in the current state in case that we use them
	 later.  We must check them here, since the back references in the
	 next state might use them.  */
      *err = check_subexp_matching_top (dfa, mctx, &next_state->nodes,
					cur_idx);
      if (BE (*err != REG_NOERROR, 0))
	return NULL;

      /* If the next state has back references.  */
      if (next_state->has_backref)
	{
	  *err = transit_state_bkref (preg, &next_state->nodes, mctx);
	  if (BE (*err != REG_NOERROR, 0))
	    return NULL;
	  next_state = mctx->state_log[cur_idx];
	}
    }
  return next_state;
}

/* Helper functions for transit_state.  */

/* From the node set CUR_NODES, pick up the nodes whose types are
   OP_OPEN_SUBEXP and which have corresponding back references in the regular
   expression. And register them to use them later for evaluating the
   correspoding back references.  */

static reg_errcode_t
check_subexp_matching_top (dfa, mctx, cur_nodes, str_idx)
     re_dfa_t *dfa;
     re_match_context_t *mctx;
     re_node_set *cur_nodes;
     int str_idx;
{
  int node_idx;
  reg_errcode_t err;

  /* TODO: This isn't efficient.
	   Because there might be more than one nodes whose types are
	   OP_OPEN_SUBEXP and whose index is SUBEXP_IDX, we must check all
	   nodes.
	   E.g. RE: (a){2}  */
  for (node_idx = 0; node_idx < cur_nodes->nelem; ++node_idx)
    {
      int node = cur_nodes->elems[node_idx];
      if (dfa->nodes[node].type == OP_OPEN_SUBEXP
	  && dfa->nodes[node].opr.idx < (8 * sizeof (dfa->used_bkref_map))
	  && dfa->used_bkref_map & (1 << dfa->nodes[node].opr.idx))
	{
	  err = match_ctx_add_subtop (mctx, node, str_idx);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
    }
  return REG_NOERROR;
}

#if 0
/* Return the next state to which the current state STATE will transit by
   accepting the current input byte.  */

static re_dfastate_t *
transit_state_sb (err, preg, state, mctx)
     reg_errcode_t *err;
     const regex_t *preg;
     re_dfastate_t *state;
     re_match_context_t *mctx;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  re_node_set next_nodes;
  re_dfastate_t *next_state;
  int node_cnt, cur_str_idx = re_string_cur_idx (mctx->input);
  unsigned int context;

  *err = re_node_set_alloc (&next_nodes, state->nodes.nelem + 1);
  if (BE (*err != REG_NOERROR, 0))
    return NULL;
  for (node_cnt = 0; node_cnt < state->nodes.nelem; ++node_cnt)
    {
      int cur_node = state->nodes.elems[node_cnt];
      if (check_node_accept (preg, dfa->nodes + cur_node, mctx, cur_str_idx))
	{
	  *err = re_node_set_merge (&next_nodes,
				    dfa->eclosures + dfa->nexts[cur_node]);
	  if (BE (*err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return NULL;
	    }
	}
    }
  context = re_string_context_at (mctx->input, cur_str_idx, mctx->eflags,
				  preg->newline_anchor);
  next_state = re_acquire_state_context (err, dfa, &next_nodes, context);
  /* We don't need to check errors here, since the return value of
     this function is next_state and ERR is already set.  */

  re_node_set_free (&next_nodes);
  re_string_skip_bytes (mctx->input, 1);
  return next_state;
}
#endif

#ifdef RE_ENABLE_I18N
static reg_errcode_t
transit_state_mb (preg, pstate, mctx)
    const regex_t *preg;
    re_dfastate_t *pstate;
    re_match_context_t *mctx;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;

  for (i = 0; i < pstate->nodes.nelem; ++i)
    {
      re_node_set dest_nodes, *new_nodes;
      int cur_node_idx = pstate->nodes.elems[i];
      int naccepted = 0, dest_idx;
      unsigned int context;
      re_dfastate_t *dest_state;

      if (dfa->nodes[cur_node_idx].constraint)
	{
	  context = re_string_context_at (mctx->input,
					  re_string_cur_idx (mctx->input),
					  mctx->eflags, preg->newline_anchor);
	  if (NOT_SATISFY_NEXT_CONSTRAINT (dfa->nodes[cur_node_idx].constraint,
					   context))
	    continue;
	}

      /* How many bytes the node can accept?  */
      if (ACCEPT_MB_NODE (dfa->nodes[cur_node_idx].type))
	naccepted = check_node_accept_bytes (preg, cur_node_idx, mctx->input,
					     re_string_cur_idx (mctx->input));
      if (naccepted == 0)
	continue;

      /* The node can accepts `naccepted' bytes.  */
      dest_idx = re_string_cur_idx (mctx->input) + naccepted;
      mctx->max_mb_elem_len = ((mctx->max_mb_elem_len < naccepted) ? naccepted
			       : mctx->max_mb_elem_len);
      err = clean_state_log_if_need (mctx, dest_idx);
      if (BE (err != REG_NOERROR, 0))
	return err;
#ifdef DEBUG
      assert (dfa->nexts[cur_node_idx] != -1);
#endif
      /* `cur_node_idx' may point the entity of the OP_CONTEXT_NODE,
	 then we use pstate->nodes.elems[i] instead.  */
      new_nodes = dfa->eclosures + dfa->nexts[pstate->nodes.elems[i]];

      dest_state = mctx->state_log[dest_idx];
      if (dest_state == NULL)
	dest_nodes = *new_nodes;
      else
	{
	  err = re_node_set_init_union (&dest_nodes,
					dest_state->entrance_nodes, new_nodes);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
      context = re_string_context_at (mctx->input, dest_idx - 1, mctx->eflags,
				      preg->newline_anchor);
      mctx->state_log[dest_idx]
	= re_acquire_state_context (&err, dfa, &dest_nodes, context);
      if (dest_state != NULL)
	re_node_set_free (&dest_nodes);
      if (BE (mctx->state_log[dest_idx] == NULL && err != REG_NOERROR, 0))
	return err;
    }
  return REG_NOERROR;
}
#endif /* RE_ENABLE_I18N */

static reg_errcode_t
transit_state_bkref (preg, nodes, mctx)
    const regex_t *preg;
    const re_node_set *nodes;
    re_match_context_t *mctx;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i;
  int cur_str_idx = re_string_cur_idx (mctx->input);

  for (i = 0; i < nodes->nelem; ++i)
    {
      int dest_str_idx, prev_nelem, bkc_idx;
      int node_idx = nodes->elems[i];
      unsigned int context;
      const re_token_t *node = dfa->nodes + node_idx;
      re_node_set *new_dest_nodes;

      /* Check whether `node' is a backreference or not.  */
      if (node->type != OP_BACK_REF)
	continue;

      if (node->constraint)
	{
	  context = re_string_context_at (mctx->input, cur_str_idx,
					  mctx->eflags, preg->newline_anchor);
	  if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
	    continue;
	}

      /* `node' is a backreference.
	 Check the substring which the substring matched.  */
      bkc_idx = mctx->nbkref_ents;
      err = get_subexp (preg, mctx, node_idx, cur_str_idx);
      if (BE (err != REG_NOERROR, 0))
	goto free_return;

      /* And add the epsilon closures (which is `new_dest_nodes') of
	 the backreference to appropriate state_log.  */
#ifdef DEBUG
      assert (dfa->nexts[node_idx] != -1);
#endif
      for (; bkc_idx < mctx->nbkref_ents; ++bkc_idx)
	{
	  int subexp_len;
	  re_dfastate_t *dest_state;
	  struct re_backref_cache_entry *bkref_ent;
	  bkref_ent = mctx->bkref_ents + bkc_idx;
	  if (bkref_ent->node != node_idx || bkref_ent->str_idx != cur_str_idx)
	    continue;
	  subexp_len = bkref_ent->subexp_to - bkref_ent->subexp_from;
	  new_dest_nodes = (subexp_len == 0
			    ? dfa->eclosures + dfa->edests[node_idx].elems[0]
			    : dfa->eclosures + dfa->nexts[node_idx]);
	  dest_str_idx = (cur_str_idx + bkref_ent->subexp_to
			  - bkref_ent->subexp_from);
	  context = re_string_context_at (mctx->input, dest_str_idx - 1,
					  mctx->eflags, preg->newline_anchor);
	  dest_state = mctx->state_log[dest_str_idx];
	  prev_nelem = ((mctx->state_log[cur_str_idx] == NULL) ? 0
			: mctx->state_log[cur_str_idx]->nodes.nelem);
	  /* Add `new_dest_node' to state_log.  */
	  if (dest_state == NULL)
	    {
	      mctx->state_log[dest_str_idx]
		= re_acquire_state_context (&err, dfa, new_dest_nodes,
					    context);
	      if (BE (mctx->state_log[dest_str_idx] == NULL
		      && err != REG_NOERROR, 0))
		goto free_return;
	    }
	  else
	    {
	      re_node_set dest_nodes;
	      err = re_node_set_init_union (&dest_nodes,
					    dest_state->entrance_nodes,
					    new_dest_nodes);
	      if (BE (err != REG_NOERROR, 0))
		{
		  re_node_set_free (&dest_nodes);
		  goto free_return;
		}
	      mctx->state_log[dest_str_idx]
		= re_acquire_state_context (&err, dfa, &dest_nodes, context);
	      re_node_set_free (&dest_nodes);
	      if (BE (mctx->state_log[dest_str_idx] == NULL
		      && err != REG_NOERROR, 0))
		goto free_return;
	    }
	  /* We need to check recursively if the backreference can epsilon
	     transit.  */
	  if (subexp_len == 0
	      && mctx->state_log[cur_str_idx]->nodes.nelem > prev_nelem)
	    {
	      err = check_subexp_matching_top (dfa, mctx, new_dest_nodes,
					       cur_str_idx);
	      if (BE (err != REG_NOERROR, 0))
		goto free_return;
	      err = transit_state_bkref (preg, new_dest_nodes, mctx);
	      if (BE (err != REG_NOERROR, 0))
		goto free_return;
	    }
	}
    }
  err = REG_NOERROR;
 free_return:
  return err;
}

/* Enumerate all the candidates which the backreference BKREF_NODE can match
   at BKREF_STR_IDX, and register them by match_ctx_add_entry().
   Note that we might collect inappropriate candidates here.
   However, the cost of checking them strictly here is too high, then we
   delay these checking for prune_impossible_nodes().  */

static reg_errcode_t
get_subexp (preg, mctx, bkref_node, bkref_str_idx)
     const regex_t *preg;
     re_match_context_t *mctx;
     int bkref_node, bkref_str_idx;
{
  int subexp_num, sub_top_idx;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  const char *buf = (const char *) re_string_get_buffer (mctx->input);
  /* Return if we have already checked BKREF_NODE at BKREF_STR_IDX.  */
  int cache_idx = search_cur_bkref_entry (mctx, bkref_str_idx);
  for (; cache_idx < mctx->nbkref_ents; ++cache_idx)
    {
      const struct re_backref_cache_entry *entry
	= &mctx->bkref_ents[cache_idx];
      if (entry->str_idx > bkref_str_idx)
	break;
      if (entry->node == bkref_node)
	return REG_NOERROR; /* We already checked it.  */
    }
  subexp_num = dfa->nodes[bkref_node].opr.idx - 1;

  /* For each sub expression  */
  for (sub_top_idx = 0; sub_top_idx < mctx->nsub_tops; ++sub_top_idx)
    {
      reg_errcode_t err;
      re_sub_match_top_t *sub_top = mctx->sub_tops[sub_top_idx];
      re_sub_match_last_t *sub_last;
      int sub_last_idx, sl_str;
      const char *bkref_str;

      if (dfa->nodes[sub_top->node].opr.idx != subexp_num)
	continue; /* It isn't related.  */

      sl_str = sub_top->str_idx;
      bkref_str = buf + bkref_str_idx;
      /* At first, check the last node of sub expressions we already
	 evaluated.  */
      for (sub_last_idx = 0; sub_last_idx < sub_top->nlasts; ++sub_last_idx)
	{
	  int sl_str_diff;
	  sub_last = sub_top->lasts[sub_last_idx];
	  sl_str_diff = sub_last->str_idx - sl_str;
	  /* The matched string by the sub expression match with the substring
	     at the back reference?  */
	  if (sl_str_diff > 0
	      && memcmp (bkref_str, buf + sl_str, sl_str_diff) != 0)
	    break; /* We don't need to search this sub expression any more.  */
	  bkref_str += sl_str_diff;
	  sl_str += sl_str_diff;
	  err = get_subexp_sub (preg, mctx, sub_top, sub_last, bkref_node,
				bkref_str_idx);

	  /* Reload buf and bkref_str, since the preceding call might
	     have reallocated the buffer.  */
	  buf = (const char *) re_string_get_buffer (mctx->input);
	  bkref_str = buf + bkref_str_idx;

	  if (err == REG_NOMATCH)
	    continue;
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
      if (sub_last_idx < sub_top->nlasts)
	continue;
      if (sub_last_idx > 0)
	++sl_str;
      /* Then, search for the other last nodes of the sub expression.  */
      for (; sl_str <= bkref_str_idx; ++sl_str)
	{
	  int cls_node, sl_str_off;
	  const re_node_set *nodes;
	  sl_str_off = sl_str - sub_top->str_idx;
	  /* The matched string by the sub expression match with the substring
	     at the back reference?  */
	  if (sl_str_off > 0 && *bkref_str++ != buf[sl_str - 1])
	    break; /* We don't need to search this sub expression any more.  */
	  if (mctx->state_log[sl_str] == NULL)
	    continue;
	  /* Does this state have a ')' of the sub expression?  */
	  nodes = &mctx->state_log[sl_str]->nodes;
	  cls_node = find_subexp_node (dfa, nodes, subexp_num, 0);
	  if (cls_node == -1)
	    continue; /* No.  */
	  if (sub_top->path == NULL)
	    {
	      sub_top->path = calloc (sizeof (state_array_t),
				      sl_str - sub_top->str_idx + 1);
	      if (sub_top->path == NULL)
		return REG_ESPACE;
	    }
	  /* Can the OP_OPEN_SUBEXP node arrive the OP_CLOSE_SUBEXP node
	     in the current context?  */
	  err = check_arrival (preg, mctx, sub_top->path, sub_top->node,
			       sub_top->str_idx, cls_node, sl_str, 0);
	  if (err == REG_NOMATCH)
	      continue;
	  if (BE (err != REG_NOERROR, 0))
	      return err;
	  sub_last = match_ctx_add_sublast (sub_top, cls_node, sl_str);
	  if (BE (sub_last == NULL, 0))
	    return REG_ESPACE;
	  err = get_subexp_sub (preg, mctx, sub_top, sub_last, bkref_node,
				bkref_str_idx);
	  if (err == REG_NOMATCH)
	    continue;
	}
    }
  return REG_NOERROR;
}

/* Helper functions for get_subexp().  */

/* Check SUB_LAST can arrive to the back reference BKREF_NODE at BKREF_STR.
   If it can arrive, register the sub expression expressed with SUB_TOP
   and SUB_LAST.  */

static reg_errcode_t
get_subexp_sub (preg, mctx, sub_top, sub_last, bkref_node, bkref_str)
     const regex_t *preg;
     re_match_context_t *mctx;
     const re_sub_match_top_t *sub_top;
     re_sub_match_last_t *sub_last;
     int bkref_node, bkref_str;
{
  reg_errcode_t err;
  int to_idx;
  /* Can the subexpression arrive the back reference?  */
  err = check_arrival (preg, mctx, &sub_last->path, sub_last->node,
		       sub_last->str_idx, bkref_node, bkref_str, 1);
  if (err != REG_NOERROR)
    return err;
  err = match_ctx_add_entry (mctx, bkref_node, bkref_str, sub_top->str_idx,
			     sub_last->str_idx);
  if (BE (err != REG_NOERROR, 0))
    return err;
  to_idx = bkref_str + sub_last->str_idx - sub_top->str_idx;
  clean_state_log_if_need (mctx, to_idx);
  return REG_NOERROR;
}

/* Find the first node which is '(' or ')' and whose index is SUBEXP_IDX.
   Search '(' if FL_OPEN, or search ')' otherwise.
   TODO: This function isn't efficient...
	 Because there might be more than one nodes whose types are
	 OP_OPEN_SUBEXP and whose index is SUBEXP_IDX, we must check all
	 nodes.
	 E.g. RE: (a){2}  */

static int
find_subexp_node (dfa, nodes, subexp_idx, fl_open)
     const re_dfa_t *dfa;
     const re_node_set *nodes;
     int subexp_idx, fl_open;
{
  int cls_idx;
  for (cls_idx = 0; cls_idx < nodes->nelem; ++cls_idx)
    {
      int cls_node = nodes->elems[cls_idx];
      const re_token_t *node = dfa->nodes + cls_node;
      if (((fl_open && node->type == OP_OPEN_SUBEXP)
	  || (!fl_open && node->type == OP_CLOSE_SUBEXP))
	  && node->opr.idx == subexp_idx)
	return cls_node;
    }
  return -1;
}

/* Check whether the node TOP_NODE at TOP_STR can arrive to the node
   LAST_NODE at LAST_STR.  We record the path onto PATH since it will be
   heavily reused.
   Return REG_NOERROR if it can arrive, or REG_NOMATCH otherwise.  */

static reg_errcode_t
check_arrival (preg, mctx, path, top_node, top_str, last_node, last_str,
	       fl_open)
     const regex_t *preg;
     re_match_context_t *mctx;
     state_array_t *path;
     int top_node, top_str, last_node, last_str, fl_open;
{
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  reg_errcode_t err;
  int subexp_num, backup_cur_idx, str_idx, null_cnt;
  re_dfastate_t *cur_state = NULL;
  re_node_set *cur_nodes, next_nodes;
  re_dfastate_t **backup_state_log;
  unsigned int context;

  subexp_num = dfa->nodes[top_node].opr.idx;
  /* Extend the buffer if we need.  */
  if (BE (path->alloc < last_str + mctx->max_mb_elem_len + 1, 0))
    {
      re_dfastate_t **new_array;
      int old_alloc = path->alloc;
      path->alloc += last_str + mctx->max_mb_elem_len + 1;
      new_array = re_realloc (path->array, re_dfastate_t *, path->alloc);
      if (new_array == NULL)
	{
	  path->alloc = old_alloc;
	  return REG_ESPACE;
	}
      path->array = new_array;
      memset (new_array + old_alloc, '\0',
	      sizeof (re_dfastate_t *) * (path->alloc - old_alloc));
    }

  str_idx = path->next_idx == 0 ? top_str : path->next_idx;

  /* Temporary modify MCTX.  */
  backup_state_log = mctx->state_log;
  backup_cur_idx = mctx->input->cur_idx;
  mctx->state_log = path->array;
  mctx->input->cur_idx = str_idx;

  /* Setup initial node set.  */
  context = re_string_context_at (mctx->input, str_idx - 1, mctx->eflags,
				  preg->newline_anchor);
  if (str_idx == top_str)
    {
      err = re_node_set_init_1 (&next_nodes, top_node);
      if (BE (err != REG_NOERROR, 0))
	return err;
      err = check_arrival_expand_ecl (dfa, &next_nodes, subexp_num, fl_open);
      if (BE (err != REG_NOERROR, 0))
	{
	  re_node_set_free (&next_nodes);
	  return err;
	}
    }
  else
    {
      cur_state = mctx->state_log[str_idx];
      if (cur_state && cur_state->has_backref)
	{
	  err = re_node_set_init_copy (&next_nodes, &cur_state->nodes);
	  if (BE ( err != REG_NOERROR, 0))
	    return err;
	}
      else
	re_node_set_init_empty (&next_nodes);
    }
  if (str_idx == top_str || (cur_state && cur_state->has_backref))
    {
      if (next_nodes.nelem)
	{
	  err = expand_bkref_cache (preg, mctx, &next_nodes, str_idx, last_str,
				    subexp_num, fl_open);
	  if (BE ( err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return err;
	    }
	}
      cur_state = re_acquire_state_context (&err, dfa, &next_nodes, context);
      if (BE (cur_state == NULL && err != REG_NOERROR, 0))
	{
	  re_node_set_free (&next_nodes);
	  return err;
	}
      mctx->state_log[str_idx] = cur_state;
    }

  for (null_cnt = 0; str_idx < last_str && null_cnt <= mctx->max_mb_elem_len;)
    {
      re_node_set_empty (&next_nodes);
      if (mctx->state_log[str_idx + 1])
	{
	  err = re_node_set_merge (&next_nodes,
				   &mctx->state_log[str_idx + 1]->nodes);
	  if (BE (err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return err;
	    }
	}
      if (cur_state)
	{
	  err = check_arrival_add_next_nodes (preg, dfa, mctx, str_idx,
					      &cur_state->nodes, &next_nodes);
	  if (BE (err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return err;
	    }
	}
      ++str_idx;
      if (next_nodes.nelem)
	{
	  err = check_arrival_expand_ecl (dfa, &next_nodes, subexp_num,
					  fl_open);
	  if (BE (err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return err;
	    }
	  err = expand_bkref_cache (preg, mctx, &next_nodes, str_idx, last_str,
				    subexp_num, fl_open);
	  if (BE ( err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&next_nodes);
	      return err;
	    }
	}
      context = re_string_context_at (mctx->input, str_idx - 1, mctx->eflags,
				      preg->newline_anchor);
      cur_state = re_acquire_state_context (&err, dfa, &next_nodes, context);
      if (BE (cur_state == NULL && err != REG_NOERROR, 0))
	{
	  re_node_set_free (&next_nodes);
	  return err;
	}
      mctx->state_log[str_idx] = cur_state;
      null_cnt = cur_state == NULL ? null_cnt + 1 : 0;
    }
  re_node_set_free (&next_nodes);
  cur_nodes = (mctx->state_log[last_str] == NULL ? NULL
	       : &mctx->state_log[last_str]->nodes);
  path->next_idx = str_idx;

  /* Fix MCTX.  */
  mctx->state_log = backup_state_log;
  mctx->input->cur_idx = backup_cur_idx;

  if (cur_nodes == NULL)
    return REG_NOMATCH;
  /* Then check the current node set has the node LAST_NODE.  */
  return (re_node_set_contains (cur_nodes, last_node)
	  || re_node_set_contains (cur_nodes, last_node) ? REG_NOERROR
	  : REG_NOMATCH);
}

/* Helper functions for check_arrival.  */

/* Calculate the destination nodes of CUR_NODES at STR_IDX, and append them
   to NEXT_NODES.
   TODO: This function is similar to the functions transit_state*(),
	 however this function has many additional works.
	 Can't we unify them?  */

static reg_errcode_t
check_arrival_add_next_nodes (preg, dfa, mctx, str_idx, cur_nodes, next_nodes)
     const regex_t *preg;
     re_dfa_t *dfa;
     re_match_context_t *mctx;
     int str_idx;
     re_node_set *cur_nodes, *next_nodes;
{
  int cur_idx;
  reg_errcode_t err;
  re_node_set union_set;
  re_node_set_init_empty (&union_set);
  for (cur_idx = 0; cur_idx < cur_nodes->nelem; ++cur_idx)
    {
      int naccepted = 0;
      int cur_node = cur_nodes->elems[cur_idx];
      re_token_type_t type = dfa->nodes[cur_node].type;
      if (IS_EPSILON_NODE (type))
	continue;
#ifdef RE_ENABLE_I18N
      /* If the node may accept `multi byte'.  */
      if (ACCEPT_MB_NODE (type))
	{
	  naccepted = check_node_accept_bytes (preg, cur_node, mctx->input,
					       str_idx);
	  if (naccepted > 1)
	    {
	      re_dfastate_t *dest_state;
	      int next_node = dfa->nexts[cur_node];
	      int next_idx = str_idx + naccepted;
	      dest_state = mctx->state_log[next_idx];
	      re_node_set_empty (&union_set);
	      if (dest_state)
		{
		  err = re_node_set_merge (&union_set, &dest_state->nodes);
		  if (BE (err != REG_NOERROR, 0))
		    {
		      re_node_set_free (&union_set);
		      return err;
		    }
		  err = re_node_set_insert (&union_set, next_node);
		  if (BE (err < 0, 0))
		    {
		      re_node_set_free (&union_set);
		      return REG_ESPACE;
		    }
		}
	      else
		{
		  err = re_node_set_insert (&union_set, next_node);
		  if (BE (err < 0, 0))
		    {
		      re_node_set_free (&union_set);
		      return REG_ESPACE;
		    }
		}
	      mctx->state_log[next_idx] = re_acquire_state (&err, dfa,
							    &union_set);
	      if (BE (mctx->state_log[next_idx] == NULL
		      && err != REG_NOERROR, 0))
		{
		  re_node_set_free (&union_set);
		  return err;
		}
	    }
	}
#endif /* RE_ENABLE_I18N */
      if (naccepted
	  || check_node_accept (preg, dfa->nodes + cur_node, mctx,
				str_idx))
	{
	  err = re_node_set_insert (next_nodes, dfa->nexts[cur_node]);
	  if (BE (err < 0, 0))
	    {
	      re_node_set_free (&union_set);
	      return REG_ESPACE;
	    }
	}
    }
  re_node_set_free (&union_set);
  return REG_NOERROR;
}

/* For all the nodes in CUR_NODES, add the epsilon closures of them to
   CUR_NODES, however exclude the nodes which are:
    - inside the sub expression whose number is EX_SUBEXP, if FL_OPEN.
    - out of the sub expression whose number is EX_SUBEXP, if !FL_OPEN.
*/

static reg_errcode_t
check_arrival_expand_ecl (dfa, cur_nodes, ex_subexp, fl_open)
     re_dfa_t *dfa;
     re_node_set *cur_nodes;
     int ex_subexp, fl_open;
{
  reg_errcode_t err;
  int idx, outside_node;
  re_node_set new_nodes;
#ifdef DEBUG
  assert (cur_nodes->nelem);
#endif
  err = re_node_set_alloc (&new_nodes, cur_nodes->nelem);
  if (BE (err != REG_NOERROR, 0))
    return err;
  /* Create a new node set NEW_NODES with the nodes which are epsilon
     closures of the node in CUR_NODES.  */

  for (idx = 0; idx < cur_nodes->nelem; ++idx)
    {
      int cur_node = cur_nodes->elems[idx];
      re_node_set *eclosure = dfa->eclosures + cur_node;
      outside_node = find_subexp_node (dfa, eclosure, ex_subexp, fl_open);
      if (outside_node == -1)
	{
	  /* There are no problematic nodes, just merge them.  */
	  err = re_node_set_merge (&new_nodes, eclosure);
	  if (BE (err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&new_nodes);
	      return err;
	    }
	}
      else
	{
	  /* There are problematic nodes, re-calculate incrementally.  */
	  err = check_arrival_expand_ecl_sub (dfa, &new_nodes, cur_node,
					      ex_subexp, fl_open);
	  if (BE (err != REG_NOERROR, 0))
	    {
	      re_node_set_free (&new_nodes);
	      return err;
	    }
	}
    }
  re_node_set_free (cur_nodes);
  *cur_nodes = new_nodes;
  return REG_NOERROR;
}

/* Helper function for check_arrival_expand_ecl.
   Check incrementally the epsilon closure of TARGET, and if it isn't
   problematic append it to DST_NODES.  */

static reg_errcode_t
check_arrival_expand_ecl_sub (dfa, dst_nodes, target, ex_subexp, fl_open)
     re_dfa_t *dfa;
     int target, ex_subexp, fl_open;
     re_node_set *dst_nodes;
{
  int cur_node, type;
  for (cur_node = target; !re_node_set_contains (dst_nodes, cur_node);)
    {
      int err;
      type = dfa->nodes[cur_node].type;

      if (((type == OP_OPEN_SUBEXP && fl_open)
	   || (type == OP_CLOSE_SUBEXP && !fl_open))
	  && dfa->nodes[cur_node].opr.idx == ex_subexp)
	{
	  if (!fl_open)
	    {
	      err = re_node_set_insert (dst_nodes, cur_node);
	      if (BE (err == -1, 0))
		return REG_ESPACE;
	    }
	  break;
	}
      err = re_node_set_insert (dst_nodes, cur_node);
      if (BE (err == -1, 0))
	return REG_ESPACE;
      if (dfa->edests[cur_node].nelem == 0)
	break;
      if (dfa->edests[cur_node].nelem == 2)
	{
	  err = check_arrival_expand_ecl_sub (dfa, dst_nodes,
					      dfa->edests[cur_node].elems[1],
					      ex_subexp, fl_open);
	  if (BE (err != REG_NOERROR, 0))
	    return err;
	}
      cur_node = dfa->edests[cur_node].elems[0];
    }
  return REG_NOERROR;
}


/* For all the back references in the current state, calculate the
   destination of the back references by the appropriate entry
   in MCTX->BKREF_ENTS.  */

static reg_errcode_t
expand_bkref_cache (preg, mctx, cur_nodes, cur_str, last_str, subexp_num,
		    fl_open)
     const regex_t *preg;
     re_match_context_t *mctx;
     int cur_str, last_str, subexp_num, fl_open;
     re_node_set *cur_nodes;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int cache_idx, cache_idx_start;
  /* The current state.  */

  cache_idx_start = search_cur_bkref_entry (mctx, cur_str);
  for (cache_idx = cache_idx_start; cache_idx < mctx->nbkref_ents; ++cache_idx)
    {
      int to_idx, next_node;
      struct re_backref_cache_entry *ent = mctx->bkref_ents + cache_idx;
      if (ent->str_idx > cur_str)
	break;
      /* Is this entry ENT is appropriate?  */
      if (!re_node_set_contains (cur_nodes, ent->node))
	continue; /* No.  */

      to_idx = cur_str + ent->subexp_to - ent->subexp_from;
      /* Calculate the destination of the back reference, and append it
	 to MCTX->STATE_LOG.  */
      if (to_idx == cur_str)
	{
	  /* The backreference did epsilon transit, we must re-check all the
	     node in the current state.  */
	  re_node_set new_dests;
	  reg_errcode_t err2, err3;
	  next_node = dfa->edests[ent->node].elems[0];
	  if (re_node_set_contains (cur_nodes, next_node))
	    continue;
	  err = re_node_set_init_1 (&new_dests, next_node);
	  err2 = check_arrival_expand_ecl (dfa, &new_dests, subexp_num,
					   fl_open);
	  err3 = re_node_set_merge (cur_nodes, &new_dests);
	  re_node_set_free (&new_dests);
	  if (BE (err != REG_NOERROR || err2 != REG_NOERROR
		  || err3 != REG_NOERROR, 0))
	    {
	      err = (err != REG_NOERROR ? err
		     : (err2 != REG_NOERROR ? err2 : err3));
	      return err;
	    }
	  /* TODO: It is still inefficient...  */
	  cache_idx = cache_idx_start - 1;
	  continue;
	}
      else
	{
	  re_node_set union_set;
	  next_node = dfa->nexts[ent->node];
	  if (mctx->state_log[to_idx])
	    {
	      int ret;
	      if (re_node_set_contains (&mctx->state_log[to_idx]->nodes,
					next_node))
		continue;
	      err = re_node_set_init_copy (&union_set,
					   &mctx->state_log[to_idx]->nodes);
	      ret = re_node_set_insert (&union_set, next_node);
	      if (BE (err != REG_NOERROR || ret < 0, 0))
		{
		  re_node_set_free (&union_set);
		  err = err != REG_NOERROR ? err : REG_ESPACE;
		  return err;
		}
	    }
	  else
	    {
	      err = re_node_set_init_1 (&union_set, next_node);
	      if (BE (err != REG_NOERROR, 0))
		return err;
	    }
	  mctx->state_log[to_idx] = re_acquire_state (&err, dfa, &union_set);
	  re_node_set_free (&union_set);
	  if (BE (mctx->state_log[to_idx] == NULL
		  && err != REG_NOERROR, 0))
	    return err;
	}
    }
  return REG_NOERROR;
}

/* Build transition table for the state.
   Return the new table if succeeded, otherwise return NULL.  */

static re_dfastate_t **
build_trtable (preg, state)
    const regex_t *preg;
    re_dfastate_t *state;
{
  reg_errcode_t err;
  re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j, k, ch;
  int dests_node_malloced = 0, dest_states_malloced = 0;
  int ndests; /* Number of the destination states from `state'.  */
  re_dfastate_t **trtable;
  re_dfastate_t **dest_states = NULL, **dest_states_word, **dest_states_nl;
  re_node_set follows, *dests_node;
  bitset *dests_ch;
  bitset acceptable;

  /* We build DFA states which corresponds to the destination nodes
     from `state'.  `dests_node[i]' represents the nodes which i-th
     destination state contains, and `dests_ch[i]' represents the
     characters which i-th destination state accepts.  */
#ifdef _LIBC
  if (__libc_use_alloca ((sizeof (re_node_set) + sizeof (bitset)) * SBC_MAX))
    dests_node = (re_node_set *)
		 alloca ((sizeof (re_node_set) + sizeof (bitset)) * SBC_MAX);
  else
#endif
    {
      dests_node = (re_node_set *)
		   malloc ((sizeof (re_node_set) + sizeof (bitset)) * SBC_MAX);
      if (BE (dests_node == NULL, 0))
	return NULL;
      dests_node_malloced = 1;
    }
  dests_ch = (bitset *) (dests_node + SBC_MAX);

  /* Initialize transiton table.  */
  trtable = (re_dfastate_t **) calloc (sizeof (re_dfastate_t *), SBC_MAX);
  state->word_trtable = 0;
  if (BE (trtable == NULL, 0))
    {
      if (dests_node_malloced)
	free (dests_node);
      return NULL;
    }

  /* At first, group all nodes belonging to `state' into several
     destinations.  */
  ndests = group_nodes_into_DFAstates (preg, state, dests_node, dests_ch);
  if (BE (ndests <= 0, 0))
    {
      if (dests_node_malloced)
	free (dests_node);
      /* Return NULL in case of an error, trtable otherwise.  */
      if (ndests == 0)
	{
	  state->trtable = trtable;
	  return trtable;
	}
      free (trtable);
      return NULL;
    }

  err = re_node_set_alloc (&follows, ndests + 1);
  if (BE (err != REG_NOERROR, 0))
    goto out_free;

#ifdef _LIBC
  if (__libc_use_alloca ((sizeof (re_node_set) + sizeof (bitset)) * SBC_MAX
			 + ndests * 3 * sizeof (re_dfastate_t *)))
    dest_states = (re_dfastate_t **)
		  alloca (ndests * 3 * sizeof (re_dfastate_t *));
  else
#endif
    {
      dest_states = (re_dfastate_t **)
		    malloc (ndests * 3 * sizeof (re_dfastate_t *));
      if (BE (dest_states == NULL, 0))
	{
out_free:
	  if (dest_states_malloced)
	    free (dest_states);
	  re_node_set_free (&follows);
	  for (i = 0; i < ndests; ++i)
	    re_node_set_free (dests_node + i);
	  free (trtable);
	  if (dests_node_malloced)
	    free (dests_node);
	  return NULL;
	}
      dest_states_malloced = 1;
    }
  dest_states_word = dest_states + ndests;
  dest_states_nl = dest_states_word + ndests;
  bitset_empty (acceptable);

  /* Then build the states for all destinations.  */
  for (i = 0; i < ndests; ++i)
    {
      int next_node;
      re_node_set_empty (&follows);
      /* Merge the follows of this destination states.  */
      for (j = 0; j < dests_node[i].nelem; ++j)
	{
	  next_node = dfa->nexts[dests_node[i].elems[j]];
	  if (next_node != -1)
	    {
	      err = re_node_set_merge (&follows, dfa->eclosures + next_node);
	      if (BE (err != REG_NOERROR, 0))
		goto out_free;
	    }
	}
      dest_states[i] = re_acquire_state_context (&err, dfa, &follows, 0);
      if (BE (dest_states[i] == NULL && err != REG_NOERROR, 0))
	goto out_free;
      /* If the new state has context constraint,
	 build appropriate states for these contexts.  */
      if (dest_states[i]->has_constraint)
	{
	  dest_states_word[i] = re_acquire_state_context (&err, dfa, &follows,
							  CONTEXT_WORD);
	  if (BE (dest_states_word[i] == NULL && err != REG_NOERROR, 0))
	    goto out_free;
	  dest_states_nl[i] = re_acquire_state_context (&err, dfa, &follows,
							CONTEXT_NEWLINE);
	  if (BE (dest_states_nl[i] == NULL && err != REG_NOERROR, 0))
	    goto out_free;
	}
      else
	{
	  dest_states_word[i] = dest_states[i];
	  dest_states_nl[i] = dest_states[i];
	}
      bitset_merge (acceptable, dests_ch[i]);
    }

  /* Update the transition table.  */
  /* For all characters ch...:  */
  for (i = 0, ch = 0; i < BITSET_UINTS; ++i)
    for (j = 0; j < UINT_BITS; ++j, ++ch)
      if ((acceptable[i] >> j) & 1)
	{
	  for (k = 0; k < ndests; ++k)
	    if ((dests_ch[k][i] >> j) & 1)
	      {
		/* k-th destination accepts the word character ch.  */
		if (state->word_trtable)
		  {
		    trtable[ch] = dest_states[k];
		    trtable[ch + SBC_MAX] = dest_states_word[k];
		  }
		else if (dfa->mb_cur_max > 1
			 && dest_states[k] != dest_states_word[k])
		  {
		    re_dfastate_t **new_trtable;

		    new_trtable = (re_dfastate_t **)
				  realloc (trtable,
					   sizeof (re_dfastate_t *)
					   * 2 * SBC_MAX);
		    if (BE (new_trtable == NULL, 0))
		      goto out_free;
		    memcpy (new_trtable + SBC_MAX, new_trtable,
			    sizeof (re_dfastate_t *) * SBC_MAX);
		    trtable = new_trtable;
		    state->word_trtable = 1;
		    trtable[ch] = dest_states[k];
		    trtable[ch + SBC_MAX] = dest_states_word[k];
		  }
		else if (IS_WORD_CHAR (ch))
		  trtable[ch] = dest_states_word[k];
		else
		  trtable[ch] = dest_states[k];
		/* There must be only one destination which accepts
		   character ch.  See group_nodes_into_DFAstates.  */
		break;
	      }
	}
  /* new line */
  if (bitset_contain (acceptable, NEWLINE_CHAR))
    {
      /* The current state accepts newline character.  */
      for (k = 0; k < ndests; ++k)
	if (bitset_contain (dests_ch[k], NEWLINE_CHAR))
	  {
	    /* k-th destination accepts newline character.  */
	    trtable[NEWLINE_CHAR] = dest_states_nl[k];
	    if (state->word_trtable)
	      trtable[NEWLINE_CHAR + SBC_MAX] = dest_states_nl[k];
	    /* There must be only one destination which accepts
	       newline.  See group_nodes_into_DFAstates.  */
	    break;
	  }
    }

  if (dest_states_malloced)
    free (dest_states);

  re_node_set_free (&follows);
  for (i = 0; i < ndests; ++i)
    re_node_set_free (dests_node + i);

  if (dests_node_malloced)
    free (dests_node);

  state->trtable = trtable;
  return trtable;
}

/* Group all nodes belonging to STATE into several destinations.
   Then for all destinations, set the nodes belonging to the destination
   to DESTS_NODE[i] and set the characters accepted by the destination
   to DEST_CH[i].  This function return the number of destinations.  */

static int
group_nodes_into_DFAstates (preg, state, dests_node, dests_ch)
    const regex_t *preg;
    const re_dfastate_t *state;
    re_node_set *dests_node;
    bitset *dests_ch;
{
  reg_errcode_t err;
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  int i, j, k;
  int ndests; /* Number of the destinations from `state'.  */
  bitset accepts; /* Characters a node can accept.  */
  const re_node_set *cur_nodes = &state->nodes;
  bitset_empty (accepts);
  ndests = 0;

  /* For all the nodes belonging to `state',  */
  for (i = 0; i < cur_nodes->nelem; ++i)
    {
      re_token_t *node = &dfa->nodes[cur_nodes->elems[i]];
      re_token_type_t type = node->type;
      unsigned int constraint = node->constraint;

      /* Enumerate all single byte character this node can accept.  */
      if (type == CHARACTER)
	bitset_set (accepts, node->opr.c);
      else if (type == SIMPLE_BRACKET)
	{
	  bitset_merge (accepts, node->opr.sbcset);
	}
      else if (type == OP_PERIOD)
	{
#ifdef RE_ENABLE_I18N
	  if (dfa->mb_cur_max > 1)
	    bitset_merge (accepts, dfa->sb_char);
	  else
#endif
	    bitset_set_all (accepts);
	  if (!(preg->syntax & RE_DOT_NEWLINE))
	    bitset_clear (accepts, '\n');
	  if (preg->syntax & RE_DOT_NOT_NULL)
	    bitset_clear (accepts, '\0');
	}
      else if (type == OP_UTF8_PERIOD)
        {
	  memset (accepts, 255, sizeof (unsigned int) * BITSET_UINTS / 2);
	  if (!(preg->syntax & RE_DOT_NEWLINE))
	    bitset_clear (accepts, '\n');
	  if (preg->syntax & RE_DOT_NOT_NULL)
	    bitset_clear (accepts, '\0');
        }
      else
	continue;

      /* Check the `accepts' and sift the characters which are not
	 match it the context.  */
      if (constraint)
	{
	  if (constraint & NEXT_NEWLINE_CONSTRAINT)
	    {
	      int accepts_newline = bitset_contain (accepts, NEWLINE_CHAR);
	      bitset_empty (accepts);
	      if (accepts_newline)
		bitset_set (accepts, NEWLINE_CHAR);
	      else
		continue;
	    }
	  if (constraint & NEXT_ENDBUF_CONSTRAINT)
	    {
	      bitset_empty (accepts);
	      continue;
	    }

	  if (constraint & NEXT_WORD_CONSTRAINT)
	    {
#ifdef RE_ENABLE_I18N
	      if (dfa->mb_cur_max > 1)
		for (j = 0; j < BITSET_UINTS; ++j)
		  accepts[j] &= (dfa->word_char[j] | ~dfa->sb_char[j]);
	      else
#endif
		for (j = 0; j < BITSET_UINTS; ++j)
		  accepts[j] &= dfa->word_char[j];
	    }
	  if (constraint & NEXT_NOTWORD_CONSTRAINT)
	    {
#ifdef RE_ENABLE_I18N
	      if (dfa->mb_cur_max > 1)
		for (j = 0; j < BITSET_UINTS; ++j)
		  accepts[j] &= ~(dfa->word_char[j] & dfa->sb_char[j]);
	      else
#endif
		for (j = 0; j < BITSET_UINTS; ++j)
		  accepts[j] &= ~dfa->word_char[j];
	    }
	}

      /* Then divide `accepts' into DFA states, or create a new
	 state.  */
      for (j = 0; j < ndests; ++j)
	{
	  bitset intersec; /* Intersection sets, see below.  */
	  bitset remains;
	  /* Flags, see below.  */
	  int has_intersec, not_subset, not_consumed;

	  /* Optimization, skip if this state doesn't accept the character.  */
	  if (type == CHARACTER && !bitset_contain (dests_ch[j], node->opr.c))
	    continue;

	  /* Enumerate the intersection set of this state and `accepts'.  */
	  has_intersec = 0;
	  for (k = 0; k < BITSET_UINTS; ++k)
	    has_intersec |= intersec[k] = accepts[k] & dests_ch[j][k];
	  /* And skip if the intersection set is empty.  */
	  if (!has_intersec)
	    continue;

	  /* Then check if this state is a subset of `accepts'.  */
	  not_subset = not_consumed = 0;
	  for (k = 0; k < BITSET_UINTS; ++k)
	    {
	      not_subset |= remains[k] = ~accepts[k] & dests_ch[j][k];
	      not_consumed |= accepts[k] = accepts[k] & ~dests_ch[j][k];
	    }

	  /* If this state isn't a subset of `accepts', create a
	     new group state, which has the `remains'. */
	  if (not_subset)
	    {
	      bitset_copy (dests_ch[ndests], remains);
	      bitset_copy (dests_ch[j], intersec);
	      err = re_node_set_init_copy (dests_node + ndests, &dests_node[j]);
	      if (BE (err != REG_NOERROR, 0))
		goto error_return;
	      ++ndests;
	    }

	  /* Put the position in the current group. */
	  err = re_node_set_insert (&dests_node[j], cur_nodes->elems[i]);
	  if (BE (err < 0, 0))
	    goto error_return;

	  /* If all characters are consumed, go to next node. */
	  if (!not_consumed)
	    break;
	}
      /* Some characters remain, create a new group. */
      if (j == ndests)
	{
	  bitset_copy (dests_ch[ndests], accepts);
	  err = re_node_set_init_1 (dests_node + ndests, cur_nodes->elems[i]);
	  if (BE (err != REG_NOERROR, 0))
	    goto error_return;
	  ++ndests;
	  bitset_empty (accepts);
	}
    }
  return ndests;
 error_return:
  for (j = 0; j < ndests; ++j)
    re_node_set_free (dests_node + j);
  return -1;
}

#ifdef RE_ENABLE_I18N
/* Check how many bytes the node `dfa->nodes[node_idx]' accepts.
   Return the number of the bytes the node accepts.
   STR_IDX is the current index of the input string.

   This function handles the nodes which can accept one character, or
   one collating element like '.', '[a-z]', opposite to the other nodes
   can only accept one byte.  */

static int
check_node_accept_bytes (preg, node_idx, input, str_idx)
    const regex_t *preg;
    int node_idx, str_idx;
    const re_string_t *input;
{
  const re_dfa_t *dfa = (re_dfa_t *) preg->buffer;
  const re_token_t *node = dfa->nodes + node_idx;
  int char_len, elem_len;
  int i;

  if (BE (node->type == OP_UTF8_PERIOD, 0))
    {
      unsigned char c = re_string_byte_at (input, str_idx), d;
      if (BE (c < 0xc2, 1))
	return 0;

      if (str_idx + 2 > input->len)
	return 0;

      d = re_string_byte_at (input, str_idx + 1);
      if (c < 0xe0)
	return (d < 0x80 || d > 0xbf) ? 0 : 2;
      else if (c < 0xf0)
	{
	  char_len = 3;
	  if (c == 0xe0 && d < 0xa0)
	    return 0;
	}
      else if (c < 0xf8)
	{
	  char_len = 4;
	  if (c == 0xf0 && d < 0x90)
	    return 0;
	}
      else if (c < 0xfc)
	{
	  char_len = 5;
	  if (c == 0xf8 && d < 0x88)
	    return 0;
	}
      else if (c < 0xfe)
	{
	  char_len = 6;
	  if (c == 0xfc && d < 0x84)
	    return 0;
	}
      else
	return 0;

      if (str_idx + char_len > input->len)
	return 0;

      for (i = 1; i < char_len; ++i)
	{
	  d = re_string_byte_at (input, str_idx + i);
	  if (d < 0x80 || d > 0xbf)
	    return 0;
	}
      return char_len;
    }

  char_len = re_string_char_size_at (input, str_idx);
  if (node->type == OP_PERIOD)
    {
      if (char_len <= 1)
        return 0;
      /* FIXME: I don't think this if is needed, as both '\n'
	 and '\0' are char_len == 1.  */
      /* '.' accepts any one character except the following two cases.  */
      if ((!(preg->syntax & RE_DOT_NEWLINE) &&
	   re_string_byte_at (input, str_idx) == '\n') ||
	  ((preg->syntax & RE_DOT_NOT_NULL) &&
	   re_string_byte_at (input, str_idx) == '\0'))
	return 0;
      return char_len;
    }

  elem_len = re_string_elem_size_at (input, str_idx);
  if (elem_len <= 1 && char_len <= 1)
    return 0;

  if (node->type == COMPLEX_BRACKET)
    {
      const re_charset_t *cset = node->opr.mbcset;
# ifdef _LIBC
      const unsigned char *pin = ((char *) re_string_get_buffer (input)
				  + str_idx);
      int j;
      uint32_t nrules;
# endif /* _LIBC */
      int match_len = 0;
      wchar_t wc = ((cset->nranges || cset->nchar_classes || cset->nmbchars)
		    ? re_string_wchar_at (input, str_idx) : 0);

      /* match with multibyte character?  */
      for (i = 0; i < cset->nmbchars; ++i)
	if (wc == cset->mbchars[i])
	  {
	    match_len = char_len;
	    goto check_node_accept_bytes_match;
	  }
      /* match with character_class?  */
      for (i = 0; i < cset->nchar_classes; ++i)
	{
	  wctype_t wt = cset->char_classes[i];
	  if (__iswctype (wc, wt))
	    {
	      match_len = char_len;
	      goto check_node_accept_bytes_match;
	    }
	}

# ifdef _LIBC
      nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
      if (nrules != 0)
	{
	  unsigned int in_collseq = 0;
	  const int32_t *table, *indirect;
	  const unsigned char *weights, *extra;
	  const char *collseqwc;
	  int32_t idx;
	  /* This #include defines a local function!  */
#  include <locale/weight.h>

	  /* match with collating_symbol?  */
	  if (cset->ncoll_syms)
	    extra = (const unsigned char *)
	      _NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);
	  for (i = 0; i < cset->ncoll_syms; ++i)
	    {
	      const unsigned char *coll_sym = extra + cset->coll_syms[i];
	      /* Compare the length of input collating element and
		 the length of current collating element.  */
	      if (*coll_sym != elem_len)
		continue;
	      /* Compare each bytes.  */
	      for (j = 0; j < *coll_sym; j++)
		if (pin[j] != coll_sym[1 + j])
		  break;
	      if (j == *coll_sym)
		{
		  /* Match if every bytes is equal.  */
		  match_len = j;
		  goto check_node_accept_bytes_match;
		}
	    }

	  if (cset->nranges)
	    {
	      if (elem_len <= char_len)
		{
		  collseqwc = _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQWC);
		  in_collseq = __collseq_table_lookup (collseqwc, wc);
		}
	      else
		in_collseq = find_collation_sequence_value (pin, elem_len);
	    }
	  /* match with range expression?  */
	  for (i = 0; i < cset->nranges; ++i)
	    if (cset->range_starts[i] <= in_collseq
		&& in_collseq <= cset->range_ends[i])
	      {
		match_len = elem_len;
		goto check_node_accept_bytes_match;
	      }

	  /* match with equivalence_class?  */
	  if (cset->nequiv_classes)
	    {
	      const unsigned char *cp = pin;
	      table = (const int32_t *)
		_NL_CURRENT (LC_COLLATE, _NL_COLLATE_TABLEMB);
	      weights = (const unsigned char *)
		_NL_CURRENT (LC_COLLATE, _NL_COLLATE_WEIGHTMB);
	      extra = (const unsigned char *)
		_NL_CURRENT (LC_COLLATE, _NL_COLLATE_EXTRAMB);
	      indirect = (const int32_t *)
		_NL_CURRENT (LC_COLLATE, _NL_COLLATE_INDIRECTMB);
	      idx = findidx (&cp);
	      if (idx > 0)
		for (i = 0; i < cset->nequiv_classes; ++i)
		  {
		    int32_t equiv_class_idx = cset->equiv_classes[i];
		    size_t weight_len = weights[idx];
		    if (weight_len == weights[equiv_class_idx])
		      {
			int cnt = 0;
			while (cnt <= weight_len
			       && (weights[equiv_class_idx + 1 + cnt]
				   == weights[idx + 1 + cnt]))
			  ++cnt;
			if (cnt > weight_len)
			  {
			    match_len = elem_len;
			    goto check_node_accept_bytes_match;
			  }
		      }
		  }
	    }
	}
      else
# endif /* _LIBC */
	{
	  /* match with range expression?  */
#if __GNUC__ >= 2
	  wchar_t cmp_buf[] = {L'\0', L'\0', wc, L'\0', L'\0', L'\0'};
#else
	  wchar_t cmp_buf[] = {L'\0', L'\0', L'\0', L'\0', L'\0', L'\0'};
	  cmp_buf[2] = wc;
#endif
	  for (i = 0; i < cset->nranges; ++i)
	    {
	      cmp_buf[0] = cset->range_starts[i];
	      cmp_buf[4] = cset->range_ends[i];
	      if (wcscoll (cmp_buf, cmp_buf + 2) <= 0
		  && wcscoll (cmp_buf + 2, cmp_buf + 4) <= 0)
		{
		  match_len = char_len;
		  goto check_node_accept_bytes_match;
		}
	    }
	}
    check_node_accept_bytes_match:
      if (!cset->non_match)
	return match_len;
      else
	{
	  if (match_len > 0)
	    return 0;
	  else
	    return (elem_len > char_len) ? elem_len : char_len;
	}
    }
  return 0;
}

# ifdef _LIBC
static unsigned int
find_collation_sequence_value (mbs, mbs_len)
    const unsigned char *mbs;
    size_t mbs_len;
{
  uint32_t nrules = _NL_CURRENT_WORD (LC_COLLATE, _NL_COLLATE_NRULES);
  if (nrules == 0)
    {
      if (mbs_len == 1)
	{
	  /* No valid character.  Match it as a single byte character.  */
	  const unsigned char *collseq = (const unsigned char *)
	    _NL_CURRENT (LC_COLLATE, _NL_COLLATE_COLLSEQMB);
	  return collseq[mbs[0]];
	}
      return UINT_MAX;
    }
  else
    {
      int32_t idx;
      const unsigned char *extra = (const unsigned char *)
	_NL_CURRENT (LC_COLLATE, _NL_COLLATE_SYMB_EXTRAMB);

      for (idx = 0; ;)
	{
	  int mbs_cnt, found = 0;
	  int32_t elem_mbs_len;
	  /* Skip the name of collating element name.  */
	  idx = idx + extra[idx] + 1;
	  elem_mbs_len = extra[idx++];
	  if (mbs_len == elem_mbs_len)
	    {
	      for (mbs_cnt = 0; mbs_cnt < elem_mbs_len; ++mbs_cnt)
		if (extra[idx + mbs_cnt] != mbs[mbs_cnt])
		  break;
	      if (mbs_cnt == elem_mbs_len)
		/* Found the entry.  */
		found = 1;
	    }
	  /* Skip the byte sequence of the collating element.  */
	  idx += elem_mbs_len;
	  /* Adjust for the alignment.  */
	  idx = (idx + 3) & ~3;
	  /* Skip the collation sequence value.  */
	  idx += sizeof (uint32_t);
	  /* Skip the wide char sequence of the collating element.  */
	  idx = idx + sizeof (uint32_t) * (extra[idx] + 1);
	  /* If we found the entry, return the sequence value.  */
	  if (found)
	    return *(uint32_t *) (extra + idx);
	  /* Skip the collation sequence value.  */
	  idx += sizeof (uint32_t);
	}
    }
}
# endif /* _LIBC */
#endif /* RE_ENABLE_I18N */

/* Check whether the node accepts the byte which is IDX-th
   byte of the INPUT.  */

static int
check_node_accept (preg, node, mctx, idx)
    const regex_t *preg;
    const re_token_t *node;
    const re_match_context_t *mctx;
    int idx;
{
  unsigned char ch;
  if (node->constraint)
    {
      /* The node has constraints.  Check whether the current context
	 satisfies the constraints.  */
      unsigned int context = re_string_context_at (mctx->input, idx,
						   mctx->eflags,
						   preg->newline_anchor);
      if (NOT_SATISFY_NEXT_CONSTRAINT (node->constraint, context))
	return 0;
    }
  ch = re_string_byte_at (mctx->input, idx);
  switch (node->type)
    {
    case CHARACTER:
      return node->opr.c == ch;
    case SIMPLE_BRACKET:
      return bitset_contain (node->opr.sbcset, ch);
    case OP_UTF8_PERIOD:
      if (ch >= 0x80)
        return 0;
      /* FALLTHROUGH */
    case OP_PERIOD:
      return !((ch == '\n' && !(preg->syntax & RE_DOT_NEWLINE))
	       || (ch == '\0' && (preg->syntax & RE_DOT_NOT_NULL)));
    default:
      return 0;
    }
}

/* Extend the buffers, if the buffers have run out.  */

static reg_errcode_t
extend_buffers (mctx)
     re_match_context_t *mctx;
{
  reg_errcode_t ret;
  re_string_t *pstr = mctx->input;

  /* Double the lengthes of the buffers.  */
  ret = re_string_realloc_buffers (pstr, pstr->bufs_len * 2);
  if (BE (ret != REG_NOERROR, 0))
    return ret;

  if (mctx->state_log != NULL)
    {
      /* And double the length of state_log.  */
      /* XXX We have no indication of the size of this buffer.  If this
	 allocation fail we have no indication that the state_log array
	 does not have the right size.  */
      re_dfastate_t **new_array = re_realloc (mctx->state_log, re_dfastate_t *,
					      pstr->bufs_len + 1);
      if (BE (new_array == NULL, 0))
	return REG_ESPACE;
      mctx->state_log = new_array;
    }

  /* Then reconstruct the buffers.  */
  if (pstr->icase)
    {
#ifdef RE_ENABLE_I18N
      if (pstr->mb_cur_max > 1)
	{
	  ret = build_wcs_upper_buffer (pstr);
	  if (BE (ret != REG_NOERROR, 0))
	    return ret;
	}
      else
#endif /* RE_ENABLE_I18N  */
	build_upper_buffer (pstr);
    }
  else
    {
#ifdef RE_ENABLE_I18N
      if (pstr->mb_cur_max > 1)
	build_wcs_buffer (pstr);
      else
#endif /* RE_ENABLE_I18N  */
	{
	  if (pstr->trans != NULL)
	    re_string_translate_buffer (pstr);
	}
    }
  return REG_NOERROR;
}


/* Functions for matching context.  */

/* Initialize MCTX.  */

static reg_errcode_t
match_ctx_init (mctx, eflags, input, n)
    re_match_context_t *mctx;
    int eflags, n;
    re_string_t *input;
{
  mctx->eflags = eflags;
  mctx->input = input;
  mctx->match_last = -1;
  if (n > 0)
    {
      mctx->bkref_ents = re_malloc (struct re_backref_cache_entry, n);
      mctx->sub_tops = re_malloc (re_sub_match_top_t *, n);
      if (BE (mctx->bkref_ents == NULL || mctx->sub_tops == NULL, 0))
	return REG_ESPACE;
    }
  else
    mctx->bkref_ents = NULL;
  mctx->nbkref_ents = 0;
  mctx->abkref_ents = n;
  mctx->max_mb_elem_len = 1;
  mctx->nsub_tops = 0;
  mctx->asub_tops = n;
  return REG_NOERROR;
}

/* Clean the entries which depend on the current input in MCTX.
   This function must be invoked when the matcher changes the start index
   of the input, or changes the input string.  */

static void
match_ctx_clean (mctx)
    re_match_context_t *mctx;
{
  match_ctx_free_subtops (mctx);
  mctx->nsub_tops = 0;
  mctx->nbkref_ents = 0;
}

/* Free all the memory associated with MCTX.  */

static void
match_ctx_free (mctx)
    re_match_context_t *mctx;
{
  match_ctx_free_subtops (mctx);
  re_free (mctx->sub_tops);
  re_free (mctx->bkref_ents);
}

/* Free all the memory associated with MCTX->SUB_TOPS.  */

static void
match_ctx_free_subtops (mctx)
     re_match_context_t *mctx;
{
  int st_idx;
  for (st_idx = 0; st_idx < mctx->nsub_tops; ++st_idx)
    {
      int sl_idx;
      re_sub_match_top_t *top = mctx->sub_tops[st_idx];
      for (sl_idx = 0; sl_idx < top->nlasts; ++sl_idx)
	{
	  re_sub_match_last_t *last = top->lasts[sl_idx];
	  re_free (last->path.array);
	  re_free (last);
	}
      re_free (top->lasts);
      if (top->path)
	{
	  re_free (top->path->array);
	  re_free (top->path);
	}
      free (top);
    }
}

/* Add a new backreference entry to MCTX.
   Note that we assume that caller never call this function with duplicate
   entry, and call with STR_IDX which isn't smaller than any existing entry.
*/

static reg_errcode_t
match_ctx_add_entry (mctx, node, str_idx, from, to)
     re_match_context_t *mctx;
     int node, str_idx, from, to;
{
  if (mctx->nbkref_ents >= mctx->abkref_ents)
    {
      struct re_backref_cache_entry* new_entry;
      new_entry = re_realloc (mctx->bkref_ents, struct re_backref_cache_entry,
			      mctx->abkref_ents * 2);
      if (BE (new_entry == NULL, 0))
	{
	  re_free (mctx->bkref_ents);
	  return REG_ESPACE;
	}
      mctx->bkref_ents = new_entry;
      memset (mctx->bkref_ents + mctx->nbkref_ents, '\0',
	      sizeof (struct re_backref_cache_entry) * mctx->abkref_ents);
      mctx->abkref_ents *= 2;
    }
  mctx->bkref_ents[mctx->nbkref_ents].node = node;
  mctx->bkref_ents[mctx->nbkref_ents].str_idx = str_idx;
  mctx->bkref_ents[mctx->nbkref_ents].subexp_from = from;
  mctx->bkref_ents[mctx->nbkref_ents].subexp_to = to;
  mctx->bkref_ents[mctx->nbkref_ents++].flag = 0;
  if (mctx->max_mb_elem_len < to - from)
    mctx->max_mb_elem_len = to - from;
  return REG_NOERROR;
}

/* Search for the first entry which has the same str_idx.
   Note that MCTX->BKREF_ENTS is already sorted by MCTX->STR_IDX.  */

static int
search_cur_bkref_entry (mctx, str_idx)
     re_match_context_t *mctx;
     int str_idx;
{
  int left, right, mid;
  right = mctx->nbkref_ents;
  for (left = 0; left < right;)
    {
      mid = (left + right) / 2;
      if (mctx->bkref_ents[mid].str_idx < str_idx)
	left = mid + 1;
      else
	right = mid;
    }
  return left;
}

static void
match_ctx_clear_flag (mctx)
     re_match_context_t *mctx;
{
  int i;
  for (i = 0; i < mctx->nbkref_ents; ++i)
    mctx->bkref_ents[i].flag = 0;
}

/* Register the node NODE, whose type is OP_OPEN_SUBEXP, and which matches
   at STR_IDX.  */

static reg_errcode_t
match_ctx_add_subtop (mctx, node, str_idx)
     re_match_context_t *mctx;
     int node, str_idx;
{
#ifdef DEBUG
  assert (mctx->sub_tops != NULL);
  assert (mctx->asub_tops > 0);
#endif
  if (BE (mctx->nsub_tops == mctx->asub_tops, 0))
    {
      int new_asub_tops = mctx->asub_tops * 2;
      re_sub_match_top_t **new_array = re_realloc (mctx->sub_tops,
						   re_sub_match_top_t *,
						   new_asub_tops);
      if (BE (new_array == NULL, 0))
	return REG_ESPACE;
      mctx->sub_tops = new_array;
      mctx->asub_tops = new_asub_tops;
    }
  mctx->sub_tops[mctx->nsub_tops] = calloc (1, sizeof (re_sub_match_top_t));
  if (BE (mctx->sub_tops[mctx->nsub_tops] == NULL, 0))
    return REG_ESPACE;
  mctx->sub_tops[mctx->nsub_tops]->node = node;
  mctx->sub_tops[mctx->nsub_tops++]->str_idx = str_idx;
  return REG_NOERROR;
}

/* Register the node NODE, whose type is OP_CLOSE_SUBEXP, and which matches
   at STR_IDX, whose corresponding OP_OPEN_SUBEXP is SUB_TOP.  */

static re_sub_match_last_t *
match_ctx_add_sublast (subtop, node, str_idx)
     re_sub_match_top_t *subtop;
     int node, str_idx;
{
  re_sub_match_last_t *new_entry;
  if (BE (subtop->nlasts == subtop->alasts, 0))
    {
      int new_alasts = 2 * subtop->alasts + 1;
      re_sub_match_last_t **new_array = re_realloc (subtop->lasts,
						    re_sub_match_last_t *,
						    new_alasts);
      if (BE (new_array == NULL, 0))
	return NULL;
      subtop->lasts = new_array;
      subtop->alasts = new_alasts;
    }
  new_entry = calloc (1, sizeof (re_sub_match_last_t));
  if (BE (new_entry != NULL, 1))
    {
      subtop->lasts[subtop->nlasts] = new_entry;
      new_entry->node = node;
      new_entry->str_idx = str_idx;
      ++subtop->nlasts;
    }
  return new_entry;
}

static void
sift_ctx_init (sctx, sifted_sts, limited_sts, last_node, last_str_idx,
	       check_subexp)
    re_sift_context_t *sctx;
    re_dfastate_t **sifted_sts, **limited_sts;
    int last_node, last_str_idx, check_subexp;
{
  sctx->sifted_states = sifted_sts;
  sctx->limited_states = limited_sts;
  sctx->last_node = last_node;
  sctx->last_str_idx = last_str_idx;
  sctx->check_subexp = check_subexp;
  sctx->cur_bkref = -1;
  sctx->cls_subexp_idx = -1;
  re_node_set_init_empty (&sctx->limits);
}

int
Rregexec (const regex_t *__restrict preg, const char *__restrict string, 
	  size_t nmatch, regmatch_t pmatch[], int eflags, int offset)
{
  reg_errcode_t err;
  int length = strlen (string);
  if (preg->no_sub)
    err = re_search_internal (preg, string, length, offset, length-offset, 
			      length, 0, NULL, eflags);
  else
    err = re_search_internal (preg, string, length, offset, length-offset, 
			      length, nmatch, pmatch, eflags);
  return err != REG_NOERROR;
}
#endif /* not USE_SYSTEM_REGEX */
