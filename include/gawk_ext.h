/*	NOTE: For all the extension functions below, you can find their usage in README.md
/*	NOTE: convention for an extension function in gawk named 'foo' is to be bound to a function
	in C named 'do_foo'	*/

#ifndef GAWK_EXT_H
#define GAWK_EXT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gawkapi.h"

static const gawk_api_t* api;	// for convenience macros to work
static awk_ext_id_t ext_id;
static const char* ext_version = "htree extension: version 1.0";
static awk_bool_t do_at_init(void);
static awk_bool_t (*init_func)(void) = do_at_init;

int plugin_is_GPL_compatible;

typedef struct query
{
	char* name;
	char** subscripts;
	unsigned char num_subs;
} query_t;

static awk_value_t* do_create_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_delete_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_modify(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_increment(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_decrement(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_remove(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_elem_exists(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_is_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_iters_remaining(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static awk_value_t* do_tree_iter_break(const int nargs, awk_value_t* result, struct awk_ext_func* _);

#endif // !GAWK_EXT_H
