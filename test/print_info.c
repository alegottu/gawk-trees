#include "print_info.h"
#include "htrees.h"

static Boolean print_node(foint _, foint key, foint data)
{
	printf("Key: %s\n", key.s);
	printf("Data: %s\n\n", data.s);
	return true;
}

static void print_htree_helper(NODETYPE* node, const unsigned char max_depth, const unsigned char depth)
{
	if (depth == max_depth - 1)
	{
		TREETYPE* tree = node->info.v;
		TreeTraverse((foint)0, tree, (pFointTraverseFcn)print_node);
	}
	else
	{
		TREETYPE* tree = node->info.v;
		print_htree_helper(tree->root, max_depth, depth + 1);
	}

	if (node->left != NULL)
		print_htree_helper(node->left, max_depth, depth);
	if (node->right != NULL)
		print_htree_helper(node->right, max_depth, depth);
}

void print_htree(HTREE* htree)
{
	print_htree_helper(htree->tree->root, htree->depth - 1, 0);
}
