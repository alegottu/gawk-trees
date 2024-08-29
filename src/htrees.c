#include "htrees.h"

#include <gawkapi.h>
#include <string.h>

// HTrees, found by their name in a gawk program, are contained here
BINTREE* trees = NULL; 

static awk_bool_t init_trees()
{
	awk_atexit((void*)do_at_exit, NULL);
	trees = BinTreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	return trees != NULL;
}

static HTREE* create_tree(char* name, const int depth) 
{
	// change comp, cpy, and free to work with foints, not just strs
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free);
	foint data; data.v = array;
	foint _name; _name.s = name;
	BinTreeInsert(trees, _name, data);

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
		int depth = (int) awk_depth.num_value;
		ret = create_tree(name, depth) != NULL;
	}
	else
	{
		fatal(ext_id, "create_tree: Invalid arguments");
	}

	return make_null_string(result);
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
	char* name;
	char* subscripts;
	char* _value;
	foint value;

	// handle value separately to discern between string or double
	get_argument(1, AWK_STRING, &awk_value);

	switch (awk_value.val_type)
	{
		case AWK_STRING:
			_value = calloc(strlen(awk_value.str_value.str) + 1, sizeof(char));
			strcpy(_value, awk_value.str_value.str);
			value.s = _value;
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
		// TODO: fix free for name
		name = calloc(strlen(awk_query.str_value.str) + 1, sizeof(char));
		strcpy(name, awk_query.str_value.str);
		name = strtok(name, "[");
		key.s = name;
		subscripts = strtok(NULL, "\0");
		strcat(subscripts, "\0");
		HTREE* tree;
        
		if (BinTreeLookup(trees, key, &_tree)) 
		{
			tree = (HTREE*) _tree.v;
			foint keys[tree->depth];
			parse_subscripts(subscripts, keys);
			HTreeInsert(tree, keys, value);
		}
		else 
		{
			foint keys[MAX_SUBSCRIPTS];
			int depth = parse_subscripts(subscripts, keys);
			tree = create_tree(name, depth);
			HTreeInsert(tree, keys, value);
		}
	}
	else
	{
		fatal(ext_id, "tree_insert: Invalid arguments");
	}

	return make_null_string(result); // No way to discern failure from void HTreeInsert
}

static bool query_tree(char* name, char* subscripts[], const int num_subscripts, foint* result)
{
	foint _name, _tree; _name.s = name; 
	bool found = false;
	foint keys[num_subscripts]; 

	for (int i = 0; i < num_subscripts; ++i)
	{
		keys[i].s = subscripts[i];
	}

	if (BinTreeLookup(trees, _name, &_tree))
	{
		HTREE* tree = _tree.v;

		if (num_subscripts != tree->depth)
		{
			fatal(ext_id, "query_tree: incorrect number of subcripts given for tree depth; returning arrays not yet implemented");
		}

		found = HTreeLookup(tree, keys, result);
		
		if (!found)
		{
			result->s = "";
			HTreeInsert(tree, keys, *result);
		}
	}
	else
	{
		found = false;
		HTREE* tree = create_tree(name, num_subscripts);
		_tree.v = tree;
		BinTreeInsert(trees, _name, _tree);
		result->s = "";
		HTreeInsert(tree, keys, *result); 
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
	foint _subscripts[MAX_SUBSCRIPTS];
	int num_subscripts;
	foint data;


	if (get_argument(0, AWK_STRING, &awk_query))
	{
		name = calloc(strlen(awk_query.str_value.str) + 1, sizeof(char));
		strcpy(name, awk_query.str_value.str);
		name = strtok(name, "[");
		subs_str = strtok(NULL, "\0");
		strcat(subs_str, "\0");
		num_subscripts = parse_subscripts(subs_str, _subscripts);
	}
	else 
	{
		fatal(ext_id, "query_tree: Invalid arguments");
	}

	char* subscripts[num_subscripts];

	for (int i = 0; i < num_subscripts; ++i)
	{
		subscripts[i] = _subscripts[i].s;
	}

	if (query_tree(name, subscripts, num_subscripts, &data))
	{
		free(name);
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
	BinTreeFree(trees);
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 2, 2, awk_false, NULL },
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
