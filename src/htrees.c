#include "htrees.h"
#include "linked-list.h"

#include <gawkapi.h>
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

	if (get_argument(0, AWK_STRING, &awk_query) && get_argument(1, AWK_STRING, &awk_value))
	{
		switch (awk_value.val_type)
		{
			case AWK_STRING:
				value.s = awk_value.str_value.str;
				break;
			/* // no way to work with this for now
			case AWK_NUMBER:
				value.f = awk_value.num_value;
				break;
			*/
			case AWK_ARRAY:
				// value.v = awk_value.array_cookie;
				// TODO: turn array cookie into HTREE*
				fatal(ext_id, "tree_insert: Array insert not yet implemented");
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
	{
		fatal(ext_id, "tree_insert: Invalid arguments");
	}

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
		{
			fatal(ext_id, "query_tree: Incorrect number of subcripts given for tree depth; returning arrays not yet implemented");
		}

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
	{
		fatal(ext_id, "query_tree: Invalid arguments");
	}

	query_tree(name, subscripts, num_subscripts, &data);
	free(name);
	return make_const_string(data.s, strlen(data.s), result);
	
	// return make_number(data.f, result);
	// TODO: No way to tell if number or string found yet, if void* is found, return nothing
}

// these values are global because they have to persist over time through calls of get_tree_next
static char* current_array_name = NULL;
static LINKED_LIST** node_queues; // one queue for each depth level of a given htree
static unsigned char current_max_depth;
static bool just_finished = false;

static Boolean visit_next_node(const unsigned char current_depth, foint* result)
{
	LINKED_LIST* current_queue = node_queues[current_depth];

	if (LinkedListSize(current_queue) == 0)
		return false;

	NODETYPE* current_node = LinkedListPop(current_queue).v;

	if (current_node->left != NULL)
		LinkedListAppend(current_queue, (foint){.v=current_node->left});
	if (current_node->right != NULL)
		LinkedListAppend(current_queue, (foint){.v=current_node->right});

	*result = current_node->info;
	return true;
}

static void fill_queues(unsigned char current_depth, foint init_info)
{
	LinkedListAppend(node_queues[current_depth], init_info);

	while (current_depth < current_max_depth - 1)
	{
		foint result;
		visit_next_node(current_depth, &result);
		TREETYPE* next_tree = result.v;
		LinkedListAppend(node_queues[++current_depth], (foint){.v=next_tree->root});
	}
}

static void free_queues()
{
	for (unsigned char i = 0; i < current_max_depth; ++i)
	{
		LinkedListFree(node_queues[i]);
	}

	free(node_queues);
}

static awk_value_t* do_get_tree_next(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	awk_value_t awk_name;
	foint name, _htree, _result;

	if (get_argument(0, AWK_STRING, &awk_name))
	{
		name.s = awk_name.str_value.str;
	}
	else 
	{
		fatal(ext_id, "get_tree_next: Invalid arguments");
	}
	
	if (current_array_name == NULL || strcmp(name.s, current_array_name) != 0)
	{
		if (current_array_name != NULL)
		{
			free(current_array_name);
			free_queues();
		}

		HTREE* htree;

		if (TreeLookup(trees, name, &_htree))
		{
			htree = _htree.v;
		}
		else
		{
			fatal(ext_id, "get_tree_next: Tree not found");
		}

		if (htree->tree->root == NULL)
		{
			fatal(ext_id, "get_tree_next: No items in tree"); 
		}

		current_array_name = strdup(name.s);
		current_max_depth = htree->depth;
		node_queues = malloc(current_max_depth * sizeof(LINKED_LIST*));

		for (unsigned char i = 0; i < current_max_depth; ++i)
		{
			node_queues[i] = LinkedListAlloc(NULL, false); // setting this to true would cause a double free
		}

		fill_queues(0, (foint){.v=htree->tree->root});
	}

	const unsigned char target_depth = current_max_depth - 1;
	unsigned char current_depth = target_depth;

	if (LinkedListSize(node_queues[current_depth]) == 0)
	{
		while(current_depth > 0)
		{
			if (visit_next_node(--current_depth, &_result))
			{
				TREETYPE* next_tree = _result.v;
				fill_queues(++current_depth, (foint){.v=next_tree->root});
				current_depth = target_depth;
				break;
			}
		}
	}

	visit_next_node(current_depth, &_result);
	const char* ret = _result.s;

	if (LinkedListSize(node_queues[current_depth]) == 0)
	{
		for (unsigned char i = 0; i < current_depth; ++i)
		{
			if (LinkedListSize(node_queues[i]) != 0)
			{
				just_finished = false;
				break;
			}
			else
			{
				just_finished = true;
			}
		}

		if (just_finished)
		{
			free_queues();
			free(current_array_name);
			current_array_name = NULL;
		}
	}

	return make_const_string(ret, strlen(ret), result);
}

static awk_value_t* do_is_current_tree_done(const int nargs, awk_value_t* result, struct awk_ext_func* _)
{
	assert(result != NULL);

	return make_number((double)just_finished, result);
}

static void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

static void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);

	if (current_array_name != NULL)
	{
		free_queues();
		free(current_array_name);
	}
}

static awk_ext_func_t func_table[] = 
{
	{ "create_tree", do_create_tree, 2, 2, awk_false, NULL },
	{ "tree_insert",  do_tree_insert, 2, 2, awk_false, NULL },
	{ "query_tree",  do_query_tree, 1, 1, awk_false, NULL },
	{ "get_tree_next", do_get_tree_next, 1, 1, awk_false, NULL},
	{ "is_current_tree_done",  do_is_current_tree_done, 0, 0, awk_false, NULL }
};
dl_load_func(func_table, htrees, "");
