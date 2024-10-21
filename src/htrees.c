#include "htrees.h"
#include "linked-list.h"

#include <gawkapi.h>
#include <string.h>

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */

// HTrees, found by their name in a gawk program, are contained here
static TREETYPE* trees = NULL; 

// keys = tree or sub-tree name, value = node queue (LL*)
static TREETYPE* current_iterators = NULL; 

static awk_bool_t init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	current_iterators = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)LinkedListFree); 

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
		fatal(ext_id, "create_tree: Invalid arguments");

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

	if (get_argument(0, AWK_STRING, &awk_query) && get_argument(1, AWK_STRING, &awk_value))
	{
		switch (awk_value.val_type)
		{
			case AWK_STRING:
				value.s = awk_value.str_value.str;
				break;
			case AWK_ARRAY:
				fatal(ext_id, "tree_insert: Attempt to use array value as a scalar");
				break;
			default:
				fatal(ext_id, "tree_insert: Invalid value type given");
		}

		foint _htree, key;
		char* name = strdup(awk_query.str_value.str);
		name = strtok(name, "[");
		key.s = name;
		char* subscripts = strtok(NULL, "\0");
		HTREE* htree;
        
		if (TreeLookup(trees, key, &_htree)) 
		{
			htree = _htree.v;
			foint keys[htree->depth];
			parse_subscripts(subscripts, keys);
			HTreeInsert(htree, keys, value);
		}
		else 
		{
			foint keys[MAX_SUBSCRIPTS];
			int depth = parse_subscripts(subscripts, keys);
			htree = create_tree(key, depth);
			HTreeInsert(htree, keys, value);
		}

		free(name);
	}
	else
		fatal(ext_id, "tree_insert: Invalid arguments");

	return make_number(1, result); // assume success if we get to this point
}

static bool query_tree(char* name, foint subscripts[], const int num_subscripts, foint* result)
{
	foint _htree, _name; _name.s = name;
	bool found = false;

	if (TreeLookup(trees, _name, &_htree))
	{
		HTREE* htree = _htree.v;

		if (num_subscripts != htree->depth)
			fatal(ext_id, "query_tree: Incorrect number of subcripts given for tree depth; returning arrays not yet implemented");

		found = HTreeLookup(htree, subscripts, result);
		
		if (!found)
		{
			result->s = "";
			HTreeInsert(htree, subscripts, *result);
		}
	}
	else
	{
		found = false;
		HTREE* htree = create_tree(_name, num_subscripts);
		result->s = "";
		HTreeInsert(htree, subscripts, *result); 
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
		fatal(ext_id, "query_tree: Invalid arguments");

	query_tree(name, subscripts, num_subscripts, &data);
	free(name);
	return make_const_string(data.s, strlen(data.s), result);
}

// Creates an iterator if one is not found for the given name, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const foint name)
{
	foint iterator, _htree;
	LINKED_LIST* result;
	
	// change this to look for root name (no "[")
	if (!TreeLookup(trees, name, &_htree))
		fatal(ext_id, "No tree found for an iterator");
	
	// here check for amount of "[" against max depth, then determine if final element or array

	if (!TreeLookup(current_iterators, name, &iterator))
	{
		result = LinkedListAlloc(NULL, false);
		HTREE* htree = _htree.v;

		if (htree->tree->root == NULL)
			return NULL;
		else
		{
			LinkedListAppend(result, (foint){.v=htree->tree->root});
			iterator.v = result;
			TreeInsert(current_iterators, name, iterator);

			return result;
		}
	}
	else
	{
		return iterator.v;
	}
}

static Boolean visit_next_node(LINKED_LIST* iterator, foint* result)
{
	if (LinkedListSize(iterator) == 0)
		return false;

	NODETYPE* current_node = LinkedListPop(iterator).v;

	if (current_node->left != NULL)
		LinkedListAppend(iterator, (foint){.v=current_node->left});
	if (current_node->right != NULL)
		LinkedListAppend(iterator, (foint){.v=current_node->right});

	*result = current_node->key;
	return true;
}

// Returns the next indice, not the next element
static awk_value_t* do_get_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	foint name, _htree, _result;

	if (get_argument(0, AWK_STRING, &awk_name))
		name.s = awk_name.str_value.str;
	else
		fatal(ext_id, "get_tree_next: Invalid arguments");

	LINKED_LIST* node_queue = get_iterator(name);
	char* ret;

	if (node_queue != NULL)
	{
		if (visit_next_node(node_queue, &_result))
		{
			// check here if array or not, how to tell what depth?
			ret = _result.s;
		}
		else
		{
			// finished
			// find out when to free / restart here?
		}
	}
	else
		fatal(ext_id, "get_tree_next: No items in tree"); 

	return make_const_string(ret, strlen(ret), result);
}

static awk_value_t* do_tree_iter_done(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name, awk_force;
	LINKED_LIST* node_queue;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		foint name;
		name.s = awk_name.str_value.str;
		node_queue = get_iterator(name);

		if (node_queue == NULL) // No elements found to iterate
			return make_number(1, result);

		if (nargs > 1) // Anything for 2nd arg = force exit
		{
			TreeDelete(current_iterators, name);
			return make_number(1, result);
		}
	}
	else
		fatal(ext_id, "tree_iter_done: Invalid args");

	bool finished = LinkedListSize(node_queue) == 0;
	return make_number((double)finished, result);
}

static void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

static void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);
	TreeFree(current_iterators);
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 2, 2, awk_false, NULL },
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL },
	{ "get_tree_next", do_get_tree_next, 1, 1, awk_false, NULL},
	{ "tree_iter_done",  do_tree_iter_done, 1, 2, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
