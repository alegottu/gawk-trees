#include "htrees.h"
#include "stack.h"

#include <tinyexpr.h>
#include <stdlib.h>
#include <float.h>

/* NOTE from gawk API documentation:
 * All pointers filled in by gawk point to memory managed by gawk and should be treated by the extension as read-only.

Memory for all strings passed into gawk from the extension must come from calling one of gawk_malloc(), gawk_calloc(), or gawk_realloc(), and is managed by gawk from then on */

// HTrees, found by their name in a gawk program, are contained here
TREETYPE* trees = NULL;

// Stack of node queues (LL*)
STACK* current_iterators = NULL;

void free_htree(foint tree)
{
	HTreeFree((HTREE*)tree.v);
}

bool init_trees()
{
	trees = TreeAlloc((pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, NULL, (pFointFreeFcn)free_htree);
	current_iterators = StackAlloc(4);
	// NOTE: 4 seems like a reasonable maximum (before the stack has to resize) of for-in loops going at once

	on_exit((void*)do_at_exit, NULL);
	return trees != NULL;
}

// NOTE: Ignoring "discards qualifiers" when it's only because it's being put into a foint;
// e.g. TreeInsert creates a copy of the string/key, so the char* in foint may as well be const char*
HTREE* create_tree(const char* name, const int depth)
{
	HTREE* array = HTreeAlloc(depth, (pCmpFcn)strcmp, (pFointCopyFcn)strdup, (pFointFreeFcn)free, (pFointCopyFcn)strdup, (pFointFreeFcn)free);
	TreeInsert(trees, (foint){.s=name}, (foint){.v=array});

	return array;
}

void delete_tree(const char* name)
{
	TreeDelete(trees, (foint){.s=name});
}

const double tree_length(const char* name)
{
	foint htree;

	if (TreeLookup(trees, (foint){.s=name}, &htree))
	{
		return ((HTREE*)htree.v)->n;
	}
	else
	{
		return strlen(name);
	}
}

static void fill_foints(const char** strs, foint* result, const unsigned char count)
{
	for (unsigned char i = 0; i < count; ++i)
	{
		result[i].s = strs[i];
	}
}

void tree_insert(const char* tree, const char** subscripts, const foint value, const unsigned char depth)
{
	foint _htree;
	HTREE* htree;
	
	if (TreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		htree = _htree.v;
		foint keys[depth];
		fill_foints(subscripts, keys, depth);
		HTreeInsert(htree, keys, value);
	}
	else
	{
		foint keys[depth];
		fill_foints(subscripts, keys, depth);
		htree = create_tree(tree, depth);
		HTreeInsert(htree, keys, value);
	}
}

static foint get_element(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	foint keys[depth];
	fill_foints(subscripts, keys, depth);
	foint result;

	if (TreeLookup(trees, (foint){.s=tree}, &_htree))
	{
		HTREE* htree = _htree.v;

		if (depth != htree->depth)
		{
			fatal(ext_id, "query_tree: Incorrect number of subscripts given for tree depth; treating array as a scalar value");
		}

		if (!HTreeLookup(htree, keys, &result))
		{
			HTreeInsert(htree, keys, (foint){.s=""});
			HTreeLookup(htree, keys, &result);
		}
	}
	else
	{
		HTREE* htree = create_tree(tree, depth);
		HTreeInsert(htree, keys, (foint){.s=""});
		HTreeLookup(htree, keys, &result);
	}

	return result;
}

const char* query_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	return get_element(tree, subscripts, depth).s;
}

static unsigned int remove_trailing_zeroes(char* num)
{
	unsigned char first_last_zero_idx = 0;
	const char* from_point = strchr(num, '.');
	
	for (unsigned char i = strlen(from_point)-1; i > 0; --i)
	{
		if (from_point[i] == '0') first_last_zero_idx = i;
		else break;
	}

	unsigned char end_pos;
	if (first_last_zero_idx == 1)
		end_pos = from_point - num;
	else
	 	end_pos = from_point + first_last_zero_idx - num;
	num[end_pos] = '\0';
		
	return end_pos + 1;
}

const double tree_modify(const char* tree, const char** subscripts, const unsigned char depth, const char* expr)
{
	foint result = get_element(tree, subscripts, depth);
	double x = atof(result.s);

	// TODO: compare speed / mem if we do branch into just te_interp
	// if (strchr(expr, 'x') == NULL)
	
	te_variable vars[] = { {"x", &x} };
	int err;
	char* _expr = malloc((strlen(result.s) + strlen(expr) + 1) * sizeof(char));
	strcpy(_expr, result.s);
	strcat(_expr, expr);
	te_expr* te = te_compile(_expr, vars, 1, &err);

	if (te != NULL)
	{
		x = te_eval(te);
		te_free(te);
		free(_expr);

		char buf[DBL_DECIMAL_DIG];
		sprintf(buf, "%f", x);
		char value[remove_trailing_zeroes(buf)];
		strcpy(value, buf);
		tree_insert(tree, subscripts, (foint){.s=value}, depth);

		return x;
	}
	else
	{
		fatal(ext_id, "tree_modify: Invalid expression given; parse error");
	}
}

// NOTE: mult is always -1 or 1
const double increment(const char* tree, const char** args, const unsigned char argc, const char mult)
{
	HTREE* htree;
	unsigned char depth;
	foint result;
	double amount = 1;
	unsigned char amount_digits = 1;

	if (TreeLookup(trees, (foint){.s=tree}, &result))
	{
		htree = result.v;

		if (argc == htree->depth)
			depth = argc;
		else
		{
			amount = atof(args[argc-1]);
			amount_digits = strlen(args[argc-1]);
			depth = argc-1;
		}

		if (depth != htree->depth)
		{
			fatal(ext_id, "tree_increment: Incorrect number of subscripts given for tree depth; treating array as a scalar value");
		}
	}
	else
	{
		amount = atof(args[argc-1]);
		amount_digits = strlen(args[argc-1]);
		depth = argc-1;

		if (amount == 0)
		{
			amount = 1;
			amount_digits = 1;
			depth = argc;
		}

		htree = create_tree(tree, depth);
	}

	foint keys[depth];
	fill_foints(args, keys, depth);
	double num = 0;

	if (HTreeLookup(htree, keys, &result))
	{
		num = atof(result.s);
	}

	unsigned int len = (num == 0 ? 1 : strlen(result.s)) + amount_digits + 8;
	// + 8 = 1 for '.' and \0, 6 for default precision of %f
	num += amount * mult;
	char buf[len];
	sprintf(buf, "%f", num);
	char value[remove_trailing_zeroes(buf)];
	strcpy(value, buf);
	HTreeInsert(htree, keys, (foint){.s=value});

	return num;
}

const double tree_increment(const char* tree, const char** args, const unsigned char argc)
{
	return increment(tree, args, argc, 1);
}

const double tree_decrement(const char* tree, const char** args, const unsigned char argc)
{
	return increment(tree, args, argc, -1);
}

const bool tree_elem_exists(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	TreeLookup(trees, (foint){.s=tree}, &_htree);
	HTREE* htree = _htree.v;
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);

	unsigned char actual = htree->depth;
	htree->depth = depth;
	const bool result = HTreeLookup(htree, _subscripts, NULL);
	htree->depth = actual;
	return result;
}

void tree_remove(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;
	bool result;
	bool found = TreeLookup(trees, (foint){.s=tree}, &_htree);

	if (found)
	{
		HTREE* htree = _htree.v;
		foint _subscripts[depth];
		fill_foints(subscripts, _subscripts, depth);

		if (depth < htree->depth)
		{
			unsigned char actual = htree->depth;
			htree->depth = depth; foint result;
			HTreeLookup(htree, _subscripts, &result);
			TREETYPE* tree = result.v;
			htree->n -= tree->n - 1;
			TreeFree(tree);
			HTreeDelete(htree, _subscripts);
			htree->depth = actual;
		}
		else
		{
			HTreeDelete(htree, _subscripts);
		}
	}
}

const unsigned short is_tree(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _subscripts[depth];
	fill_foints(subscripts, _subscripts, depth);
	foint htree;

	if (!TreeLookup(trees, (foint){.s=tree}, &htree))
		return 0;

	return depth < ((HTREE*)htree.v)->depth;
}

typedef struct _iterator
{
	unsigned int hash; // Needed to validate if a certain iterator already exists or not
	LINKED_LIST* current;
} ITERATOR;

static const unsigned int hash_query(const char* tree, const char** subscripts, const unsigned char depth)
{
	unsigned int length = strlen(tree);
	char* query = malloc((length + 1) * sizeof(char));
	strcpy(query, tree);

	for (unsigned char i = 0; i < depth; ++i)
	{
		length += strlen(subscripts[i]) + 1;
		query = realloc(query, (length + 1) * sizeof(char)); // TODO: analyze this function and see if one alloc is better (maybe with sprintf or stpcpy)
		strcat(query, " "); // Delimiter needed to prevent mismatches
		strcat(query, subscripts[i]);
	}

	const unsigned char p = 67;
	const unsigned int m = 1e9 + 9;
	unsigned int result = 0;
	unsigned int p_raised_i = 1;

	for (unsigned int i = 0; i < length; ++i)
	{
		result = (result + query[i] * p_raised_i) % m;
		p_raised_i = (p_raised_i * p) % m;
	}

	free(query);
	return result;
}

// Creates an iterator if one is not found for the given query, and the HTREE of that name has valid elements to create one
static LINKED_LIST* get_iterator(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint _htree;

	if (!TreeLookup(trees, (foint){.s=tree}, &_htree))
		return NULL;

	HTREE* htree = _htree.v;
	const unsigned char htree_depth = htree->depth;
	
	if (depth == htree_depth)
	{
		fatal(ext_id, "Attempt to iterate through a scalar value");
	}

	ITERATOR* last;
	unsigned int hash;

	if (StackSize(current_iterators) == 0 ||
	(hash = hash_query(tree, subscripts, depth)) != (last = StackTop(current_iterators).v)->hash)
	{
		if (htree->tree->root == NULL)
		{
			return NULL;
		}
		else
		{
			ITERATOR* new = malloc(sizeof(ITERATOR));
			new->current = LinkedListAlloc(NULL, false);
			new->hash = hash;
			foint root_node;

			if (depth == 0)
				root_node.v = htree->tree->root;
			else
			{
				foint _subscripts[depth]; foint result;
				fill_foints(subscripts, _subscripts, depth);
				unsigned char actual = htree->depth;
				htree->depth = depth;
				HTreeLookup(htree, _subscripts, &result);
				htree->depth = actual;
				root_node.v = ((TREETYPE*)result.v)->root;
			}

			LinkedListAppend(new->current, root_node);
			foint iterator = {.v=new};
			StackPush(current_iterators, iterator);
			return new->current;
		}
	}
	else
	{
		return last->current;
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
const char* tree_next(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint result;
	LINKED_LIST* node_queue = get_iterator(tree, subscripts, depth);

	if (node_queue != NULL)
	{
		if (visit_next_node(node_queue, &result))
		{
			return result.s;
		}
		else
		{
			fatal(ext_id, "get_tree_next: Attempt to infinitely iterate through the given tree");
		}
	}

	fatal(ext_id, "get_tree_next: No items in tree");
	return "";
}

static void iterators_pop()
{
	ITERATOR* item = StackPop(current_iterators).v;
	LinkedListFree(item->current);
	free(item);
}

// NOTE: Might have to change to larger return type
const unsigned int tree_iters_remaining(const char* tree, const char** subscripts, const unsigned char depth)
{
	foint result;
	LINKED_LIST* node_queue = get_iterator(tree, subscripts, depth);
	
	if (node_queue == NULL) // No elements found to iterate
		return 0;

	const int remaining = LinkedListSize(node_queue);

	if (remaining == 0)
		iterators_pop();

	return remaining;
}

void tree_iter_break()
{
	if (StackSize(current_iterators) == 0)
	{
		fatal(ext_id, "tree_iter_break: Cannot break if the corresponding iterator does not exist");
	}

	iterators_pop();
}

void do_at_exit(void* data, int exit_status)
{
	while(StackSize(current_iterators) > 0)
		iterators_pop();

	TreeFree(trees);
	StackFree(current_iterators);
}
