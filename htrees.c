#include "htrees.h"

// HTrees, found by their name in a gawk program, are contained here
BINTREE* trees = NULL; 

static HTREE* create_tree(char* name, const int depth) 
{
	HTREE* array = HTreeAlloc(depth, TREE_ALLOC_ARGS);
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

static int parse_subscripts(char* subs_str, foint* subscripts, const char* split)
{
	char* token = strtok(subs_str, split);
	int num_subscripts = 1;
	subscripts[0].s = token;

	while(token)
	{ 
		token = strtok(NULL, split);
		subscripts[num_subscripts++].s = token;
	}

	if (--num_subscripts > MAX_SUBSCRIPTS)
	{
		warning(ext_id, "Too many subscripts given");
	}

	return num_subscripts;
}

static awk_value_t* do_tree_insert(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_split, awk_name, awk_subscripts, awk_value;
	char* split = ","; 
	char* name;
	char* subscripts;
	foint value;

	if (nargs > 3 && get_argument(3, AWK_STRING, &awk_split))
	{
		split = awk_split.str_value.str; 
	}

	// handle value separately to discern between string or double
	get_argument(2, AWK_STRING, &awk_value);

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
    
	if (get_argument(0, AWK_STRING, &awk_name) &&
			get_argument(1, AWK_STRING, &awk_subscripts))
	{
		foint _tree, key;
		HTREE* tree;
        
		if (BinTreeLookup(trees, key, &_tree)) 
		{
			tree = (HTREE*) _tree.v;
			foint keys[tree->depth];
			parse_subscripts(subscripts, keys, split);
			HTreeInsert(tree, keys, value);
		}
		else 
		{
			foint keys[MAX_SUBSCRIPTS];
			int depth = parse_subscripts(subscripts, keys, split);
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
		found = HTreeLookup(tree, keys, result);
		
		if (!found)
		{
			memset(result->s, '\0', 1);
			HTreeInsert(tree, keys, *result);
		}
	}
	else
	{
		HTREE* tree = create_tree(name, num_subscripts);
		_tree.v = tree;
		BinTreeInsert(trees, _name, _tree);
		memset(result->s, '\0', 1);
		HTreeInsert(tree, keys, *result); 
	}

	return found;
}

static awk_value_t* do_query_tree(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

 	awk_value_t awk_name, awk_subscripts, awk_split;   
	char* name;
	char* subs_str;
	foint _subscripts[MAX_SUBSCRIPTS];
	int num_subscripts;
	char* split;
	foint data;

	if (nargs > 2 && get_argument(2, AWK_STRING, &awk_split))
	{
		split = awk_split.str_value.str;
	}

	if (get_argument(0, AWK_STRING, &awk_name)
		&& get_argument(1, AWK_STRING, &awk_subscripts))
	{
		name = awk_name.str_value.str;
		subs_str = awk_subscripts.str_value.str;
		num_subscripts = parse_subscripts(subs_str, _subscripts, split);
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

	query_tree(name, subscripts, num_subscripts, &data);
	return make_number(data.f, result);
	// return make_malloced_string(data.s, strlen(data.s), result);
	// TODO: No way to tell if number or string found yet, if void* is found, return nothing
}

static awk_bool_t init_trees()
{
	trees = BinTreeAlloc(TREE_ALLOC_ARGS); 
	return trees != NULL;
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 4, 3, awk_false, NULL },
	{ "query_tree",  do_query_tree, 3, 2, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
