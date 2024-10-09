/*	NOTE: convention for an extension function in gawk named 'foo' is to be bound to a function
	in C named 'do_foo'	*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HTREES_H
#define HTREES_H

// needed for other included files below
#include <string.h>
#include <stdio.h>

#include <stdbool.h>
#include <sys/stat.h>

#include "gawkapi.h"
#include "htree.h"

#define MAX_SUBSCRIPTS 8 

static const gawk_api_t* api;	// for convenience macros to work
static awk_ext_id_t ext_id;
static const char* ext_version = "bintree extension: version 1.0";
static awk_bool_t init_trees(void);
static awk_bool_t (*init_func)(void) = init_trees;

int plugin_is_GPL_compatible;

static foint copy_str(foint info);

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

// usage: get_tree_next(name), use in a loop to iterate through all elements
// once this function returns the final element, it will loop back to the first one
// each HTREE is iterated through using a series of queues for a BFS of each level of the HTREE;
// at the moment, only one HTREE iterator like this is supported in memory, so requesting to
// iterate through a different tree while another one is in progress will reset the iteration
// through that other tree.
static awk_value_t* do_get_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _);

// usage: is_current_tree_done(), returns 0 if the current tree being iterated on with
// "get_tree_next" has more elements, or 1 if the final element was returned with the
// most recent call of "get_tree_next"
// if no tree is currently being iterated on, this will always return false
static awk_value_t* do_is_current_tree_done(const int nargs, awk_value_t* result, struct awk_ext_func* _);

static void free_htree(foint tree);
static void do_at_exit(void* data, int exit_status);

#endif // !HTREES_H
