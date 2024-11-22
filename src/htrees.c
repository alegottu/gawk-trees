#include "htrees.h"

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */


// HTrees, found by their name in a gawk program, are contained here
TREETYPE* trees = NULL; 

// keys = tree or sub-tree name, value = node queue (LL*)
TREETYPE* current_iterators = NULL; 

void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

static foint copy_str(foint info)
{
	foint ret;
	ret.s = malloc((strlen(info.s) + 1) * sizeof(char));
	strcpy(ret.s, info.s);
	return ret;
}

bool init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree); 
	current_iterators = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)LinkedListFree); 

	on_exit((void*)do_at_exit, NULL); // possible to use 2nd arg instead of global trees

	return trees != NULL;
}

HTREE* create_tree(char* name, const int depth) 
{
	// possibly need free foint fcn
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)copy_str, (pFointFreeFcn)free);
	TreeInsert(trees, (foint){.s=name}, (foint){.v=array});

	return array;
}

const bool delete_tree(char* name)
{
	return TreeDelete(trees, (foint){.s=name});
}

static int parse_subscripts(foint* subscripts)
{
	// assuming the name of the tree was recently extracted
	char* token = strtok(NULL, "][");
	int num_subscripts = 0;

	while(token)
	{ 
		subscripts[num_subscripts++].s = token;
		token = strtok(NULL, "][");
	}

	if (num_subscripts > MAX_SUBSCRIPTS)
	{
		fputs("Too many subscripts given, the rest will be ignored", stderr);
	}

	return num_subscripts;
}

void tree_insert(const char* query, const foint value)
{
	foint _htree, key;
	char* _query = strdup(query);
	char* name = strtok(_query, "[");
	key.s = name;
	HTREE* htree;
	
	if (TreeLookup(trees, key, &_htree)) 
	{
		htree = _htree.v;
		foint keys[htree->depth];
		parse_subscripts(keys);
		HTreeInsert(htree, keys, value);
	}
	else 
	{
		foint keys[MAX_SUBSCRIPTS];
		int depth = parse_subscripts(keys);
		htree = create_tree(name, depth);
		HTreeInsert(htree, keys, value);
	}

	free(_query);
}

const bool query_tree(const char* query, foint* result)
{
	char* _query = strdup(query);
	char* name = strtok(_query, "[");
	foint subscripts[MAX_SUBSCRIPTS]; // potentially expand this dynamically
	unsigned short num_subscripts = parse_subscripts(subscripts);
	foint _htree, _name; _name.s = name;
	bool found = false;

	if (TreeLookup(trees, _name, &_htree))
	{
		HTREE* htree = _htree.v;

		if (num_subscripts != htree->depth)
		{
			fputs("query_tree: Incorrect number of subcripts given for tree depth; returning arrays not yet implemented", stderr);
			exit(1);
			// May have to changes instances of fputs -> exit to a special return value checked by funcs in gawk_ext.c to then use fatal()
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
		HTREE* htree = create_tree(name, num_subscripts);
		result->s = "";
		HTreeInsert(htree, subscripts, *result); 
	}

	free(_query);
	return found;
}

const bool tree_elem_exists(char* tree, char** subscripts)
{
	foint _htree;
	TreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	unsigned char depth = htree->depth;
	foint _subscripts[depth];

	for (unsigned char i = 0; i < depth; ++i)
	{
		_subscripts[i].s = subscripts[i];
	}

	foint result;
	return HTreeLookup(htree, _subscripts, &result);
}

const bool tree_remove(char* tree, char** subscripts)
{
	foint _htree;
	TreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	unsigned char depth = htree->depth;
	foint _subscripts[depth];

	for (unsigned char i = 0; i < depth; ++i)
	{
		_subscripts[i].s = subscripts[i];
	}

	return HTreeLookDel(htree, _subscripts, (foint*)1);
}

const unsigned short is_tree(const char* query)
{
	char* _query = strdup(query);
	char* name = strtok(_query, "[");
	foint subscripts[MAX_SUBSCRIPTS]; // same decision to dynamically expand
	unsigned short num_subscripts = parse_subscripts(subscripts);
	foint _htree;

	if (!TreeLookup(trees, (foint){.s=name}, &_htree))
	{
		free(_query);
		fputs("Invalid tree name", stderr);
		exit(1);
	}

	free(_query);
	HTREE* htree = _htree.v;

	// could possibly use depth - num_subscripts instead
	if (num_subscripts < htree->depth)
		return 1;
	else if (num_subscripts == htree->depth)
		return 0;

	fputs("Excess subscripts given for this tree's depth", stderr);
	exit(1);
}

// Creates an iterator if one is not found for the given name, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const foint query)
{
	foint iterator, _htree;
	LINKED_LIST* result;
	char* _query = strdup(query.s);
	char* name = strtok(_query, "[");

	if (!TreeLookup(trees, (foint){.s=name}, &_htree))
	{
		fputs("No tree found for an iterator", stderr);
		exit(1);
	}

	HTREE* htree = _htree.v;
	foint subscripts[MAX_SUBSCRIPTS]; // same optimization to potentially expand dynamically
	unsigned char num_subscripts = parse_subscripts(subscripts);

	if (num_subscripts == htree->depth)
	{
		fputs("Attempt to iterate through a scalar value", stderr);
		exit(1);
	}
		
	if (!TreeLookup(current_iterators, query, &iterator))
	{
		result = LinkedListAlloc(NULL, false);

		if (htree->tree->root == NULL)
		{
			free(_query);
			return NULL;
		}
		else
		{
			LinkedListAppend(result, (foint){.v=htree->tree->root});
			iterator.v = result;
			TreeInsert(current_iterators, query, iterator);

			free(_query);
			return result;
		}
	}
	else
	{
		free(_query);
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

// Returns the next index, not the next element
const char* tree_next(char* query)
{
	LINKED_LIST* node_queue = get_iterator((foint){.s=query});
	foint result;

	if (node_queue != NULL)
	{
		if (visit_next_node(node_queue, &result))
		{
			return result.s;
		}
		else
		{
			fputs("get_tree_next: Attempt to infinitely iterate through the given tree", stderr);
			exit(1);
		}
	}
	else
		fputs("get_tree_next: No items in tree", stderr); 

	return "ERROR";
}

const bool tree_iter_done(char* query, const bool force)
{
	foint _query = {.s=query};
	LINKED_LIST* node_queue = get_iterator(_query);

	if (node_queue == NULL) // No elements found to iterate
		return true;

	if (force)
	{
		TreeDelete(current_iterators, _query);
		return true;
	}

	if (LinkedListSize(node_queue) == 0)
	{
		TreeDelete(current_iterators, _query);
		return true;
	}

	return false;
}

void do_at_exit(void* data, int exit_status)
{
	TreeFree(trees);
	TreeFree(current_iterators);
}
