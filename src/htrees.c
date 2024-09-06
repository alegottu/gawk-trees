#include "htrees.h"

#include <gawkapi.h>
#include <stdio.h>
#include <string.h>

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */

// HTrees, found by their name in a gawk program, are contained here
TREETYPE* trees = NULL; 

static awk_bool_t init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	awk_atexit((void*)do_at_exit, NULL);

	return trees != NULL;
}

static foint copy_str(foint info)
{
	foint ret;
	ret.s = malloc((strlen(info.s) + 1) * sizeof(char));
	strcpy(ret.s, info.s);
	return ret;
}

static HTREE* create_tree(const foint name, const int depth) 
{
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)strdup, (pFointFreeFcn)free);
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

	return result; // No way to discern failure from void HTreeInsert
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
		_tree.v = tree;
		TreeInsert(trees, _name, _tree);
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

	if (query_tree(name, subscripts, num_subscripts, &data))
	{
		free(name);
		// return make_malloced_string(data.s, strlen(data.s), result);
		return make_const_string(data.s, strlen(data.s), result);
	}
	else 
	{
		free(name);
		return make_null_string(result);
	}
	// return make_number(data.f, result);
	// TODO: No way to tell if number or string found yet, if void* is found, return nothing
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
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
