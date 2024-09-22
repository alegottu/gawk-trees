#include "htrees.h"

#include <string.h>

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */

// HTrees, found by their name in a gawk program, are contained here
static TREETYPE* trees = NULL; 

static awk_bool_t init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	awk_atexit((void*)do_at_exit, NULL);

	return trees != NULL;
}

static foint copy_str(foint info)
{
	foint ret;
	ret.s = gawk_malloc((strlen(info.s) + 1) * sizeof(char));
	strcpy(ret.s, info.s);
	return ret;
}

static HTREE* create_tree(const foint name, const int depth) 
{
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)copy_str, (pFointFreeFcn)api->api_free);
	foint data; data.v = array;
	TreeInsert(trees, name, data);

	return array;
}

static awk_value_t* do_create_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name, awk_depth;
	bool ret;

	if (get_argument(0, AWK_STRING, &awk_name) && get_argument(1, AWK_NUMBER, &awk_depth))
	{
		char* name = awk_name.str_value.str;
		int depth = (int)awk_depth.num_value;
		ret = create_tree((foint){.s=name}, depth) != NULL;
	}
	else
	{
		fatal(ext_id, "create_tree: Invalid arguments");
	}

	// no make_bool function in gawk api 3.0
	return make_number((double)ret, result);
}

static int parse_subscripts(char* subs_str, foint* subscripts)
{
	// assuming the name of the tree has already been extracted
	char* token = strtok(subs_str, "][");
	int num_subscripts = 0;

	while(token)
	{ 
		subscripts[num_subscripts++].s = token;
		token = strtok(NULL, "][");
	}

	if (num_subscripts > MAX_SUBSCRIPTS)
	{
		warning(ext_id, "Too many subscripts given");
	}

	return num_subscripts;
}

static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_query, awk_value;
	foint value;

	// handle value separately to discern between string or double
	get_argument(1, AWK_STRING, &awk_value);

	switch (awk_value.val_type)
	{
		case AWK_STRING:
			value.s = awk_value.str_value.str;
			break;
		case AWK_NUMBER:
			value.f = awk_value.num_value;
			break;
		case AWK_ARRAY:
			// value.v = awk_value.array_cookie;
			// TODO: turn array cookie into HTREE*
			fatal(ext_id, "tree_insert: Array insert not yet implemented");
			break;
		default:
			fatal(ext_id, "tree_insert: Invalid value type given");
	}
    
	if (get_argument(0, AWK_STRING, &awk_query))
	{
		foint _tree, key;
		char* name = strdup(awk_query.str_value.str);
		name = strtok(name, "[");
		key.s = name;
		char* subscripts = strtok(NULL, "\0");
		HTREE* tree;
        
		if (TreeLookup(trees, key, &_tree)) 
		{
			tree = (HTREE*)_tree.v;
			foint keys[tree->depth];
			parse_subscripts(subscripts, keys);
			HTreeInsert(tree, keys, value);
		}
		else 
		{
			foint keys[MAX_SUBSCRIPTS];
			int depth = parse_subscripts(subscripts, keys);
			tree = create_tree(key, depth);
			HTreeInsert(tree, keys, value);
		}

		free(name);
	}
	else
	{
		fatal(ext_id, "tree_insert: Invalid arguments");
	}

	return make_number(1, result); // assume success if we get to this point
}

static bool query_tree(char* name, foint subscripts[], const int num_subscripts, foint* result)
{
	foint _tree, _name; _name.s = name;
	bool found = false;

	if (TreeLookup(trees, _name, &_tree))
	{
		HTREE* tree = _tree.v;

		if (num_subscripts != tree->depth)
		{
			fatal(ext_id, "query_tree: incorrect number of subcripts given for tree depth; returning arrays not yet implemented");
		}

		found = HTreeLookup(tree, subscripts, result);
		
		if (!found)
		{
			result->s = "";
			HTreeInsert(tree, subscripts, *result);
		}
	}
	else
	{
		found = false;
		HTREE* tree = create_tree(_name, num_subscripts);
		result->s = "";
		HTreeInsert(tree, subscripts, *result); 
	}

	return found;
}

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

 	awk_value_t awk_query;   
	char* name;
	char* subs_str;
	// TODO: start this array at size 1 and dynamically expand ?
	foint subscripts[MAX_SUBSCRIPTS];
	unsigned char num_subscripts;
	foint data;

	if (get_argument(0, AWK_STRING, &awk_query))
	{
		name = strdup(awk_query.str_value.str);
		name = strtok(name, "[");
		subs_str = strtok(NULL, "\0");
		num_subscripts = parse_subscripts(subs_str, subscripts);
	}
	else 
	{
		fatal(ext_id, "query_tree: Invalid arguments");
	}

	query_tree(name, subscripts, num_subscripts, &data);
	free(name);
	return make_const_string(data.s, strlen(data.s), result);
	
	// return make_number(data.f, result);
	// TODO: No way to tell if number or string found yet, if void* is found, return nothing
}

/*
static awk_array_t current_array;
static unsigned char current_depth;
static unsigned char current_max_depth;

static Boolean htree_traverse_final(foint key, foint data)
{
	awk_value_t awk_key, awk_data;
	// for now use make_const_string, can possibly just manually change awk_value_t vars
	make_const_string(key.s, strlen(key.s), &awk_key);
	make_const_string(data.s, strlen(data.s), &awk_data);

	if (!set_array_element(current_array, &awk_key, &awk_data))
	{
		fatal(ext_id, "get_tree_iter: Could not initialize iterator element");
	}

	current_depth = 0;
	// true = continue to make sure we get all the elements on the final depth
	return true;
}

static Boolean htree_traverse_lower(foint key, foint data)
{
	TREETYPE* tree = data.v;

	if (current_depth++ == current_max_depth - 1)
	{
		return TreeTraverse(tree, (pFointTraverseFcn)htree_traverse_final);
	}
	else
	{
		return TreeTraverse(tree, (pFointTraverseFcn)htree_traverse_lower);
	}
}

static awk_value_t* do_get_tree_iter(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	foint name, _tree;
	HTREE* tree;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		name.s = awk_name.str_value.str;
	}
	else 
	{
		fatal(ext_id, "get_tree_iter: Invalid arguments");
	}

	current_array = create_array();
	result->val_type = AWK_ARRAY;
	result->array_cookie = current_array;

	// update gawk symbol table so that we don't have to use release_array manually
	if (!sym_update(name.s, result))
	{
		fatal(ext_id, "get_tree_iter: Could not update symbol table");
	}

	current_array = result->array_cookie;
	
	if (TreeLookup(trees, name, &_tree))
	{
		tree = _tree.v;
	}
	else
	{
		fatal(ext_id, "get_tree_iter: Tree not found");
	}

	current_max_depth = tree->depth;
	current_depth = 0;
	TreeTraverse(tree->tree, (pFointTraverseFcn)htree_traverse_lower);

	return result;
}
*/

static char* current_array_name = NULL;
static char** current_array;
static unsigned char current_length;
static unsigned char current_element;
static unsigned char current_depth;
static unsigned char current_max_depth;

static Boolean htree_traverse_final(foint key, foint data)
{
	// we never get here currently
	current_array[current_element++] = data.s;	
	current_depth = 0;
	// true = continue to make sure we get all the elements on the final depth
	return true;
}

static Boolean htree_traverse_lower(foint key, foint data)
{
	TREETYPE* tree = data.v;

	if (current_depth++ == current_max_depth - 1)
	{
		if (current_array == NULL)
		{
			current_length = tree->n;
			current_array = malloc(current_length * sizeof(char*));
		}
		else
		{
			current_length += tree->n;
			current_array = realloc(current_array, current_length * sizeof(char*));
		}

		return TreeTraverse(tree, (pFointTraverseFcn)htree_traverse_final);
	}
	else
	{
		return TreeTraverse(tree, (pFointTraverseFcn)htree_traverse_lower);
	}
}

static awk_value_t* do_get_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	foint name, _tree;
	char* ret;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		name.s = awk_name.str_value.str;
	}
	else 
	{
		fatal(ext_id, "get_tree_iter: Invalid arguments");
	}
	
	if (current_array_name == NULL || strcmp(name.s, current_array_name) != 0)
	{
		if (current_array_name != NULL)
		{
			free(current_array_name);
			free(current_array);
		}

		current_array_name = strdup(name.s);	
		HTREE* tree;

		if (TreeLookup(trees, name, &_tree))
		{
			tree = _tree.v;
		}
		else
		{
			fatal(ext_id, "get_tree_iter: Tree not found");
		}

		// flatten the HTREE into an array of char*
		current_array = NULL;
		current_max_depth = tree->depth;
		current_element = 0;
		current_depth = 0;
		current_length = 0;
		TreeTraverse(tree->tree, (pFointTraverseFcn)htree_traverse_lower);
		current_element = 0;
	}

	if (current_element >= current_length)
	{
		ret = "__htree_end__";
		current_element = 0;
	}
	else
	{
		ret = current_array[current_element];
		current_element++;
	}

	return make_const_string(ret, strlen(ret), result);
}

static void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

static void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 2, 2, awk_false, NULL },
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL },
	// { "get_tree_iter", do_get_tree_iter, 1, 1, awk_false, NULL }
	{ "get_tree_next", do_get_tree_next, 1, 1, awk_false, NULL}
};
dl_load_func(func_table, htrees, "");
