/*	NOTE: convention for an extension function in awk named 'foo' is to be bound to a function
	in C named 'do_foo'	*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

#include "gawkapi.h"
#include "htree.h"

#define MAX_SUBSCRIPTS 8 
#define TREE_ALLOC_ARGS (pCmpFcn) strcmp, (pFointCopyFcn) strdup, (pFointFreeFcn) free, NULL, NULL

static const gawk_api_t* api;	// for convenience macros to work
static awk_ext_id_t ext_id;
static const char* ext_version = "bintree extension: version 1.0";
static awk_bool_t init_trees(void);
static awk_bool_t (*init_func)(void) = init_trees;

int plugin_is_GPL_compatible;

//	usage: create_tree(name, depth)
static awk_value_t* do_create_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);

/* 	usage: tree_insert(name+subscripts, value)
	the first parameter is the name of the tree concatenated with the desired
		subscripts, e.g. "name[x][y]"
	creates a tree called "name" if one doesn't yet exist
	creates a default value under the subscripts if one doesn't exit yet	*/
static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _);

/* 	usage: query_tree(name+subscripts);
	as with tree_insert, the first and only parameter is the name of the tree
		concatenated with the desired subscripts, e.g. "name[x][y]"
 	instantiates the value for the subscript(s) if there isn't one already
 	instantiates the entire tree as well if there isn't one named that already	*/
static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _);