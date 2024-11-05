#include "htrees.h"

int main(int argc, char *argv[])
{
	init_trees();

	create_tree("pred", 2);
	char* elem = "pred[a][b]";
	tree_insert(elem, (foint)"1"); // for testing, key / value must always be as string, gawk API normally does this for us
	foint result;
	query_tree(elem, &result);
	puts(result.s);
	query_tree(elem, &result);
	puts(result.s);
	elem = "pred[x][y]";
	tree_insert(elem, (foint)"hello");
	query_tree(elem, &result);
	puts(result.s);

	return 0;
}
