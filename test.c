#include "htrees.h"
#include <stdio.h>

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

	create_tree("test", 2);
	elem = "test[x][y]";
	tree_insert(elem, (foint)"864");
	query_tree(elem, &result);
	puts(result.s);
	elem = "test[x][z]";
	tree_insert(elem, (foint)"229");
	query_tree("test[y][z]", &result);
	puts(result.s);
	query_tree(elem, &result);
	puts(result.s);

	elem = "example[sub]";
	tree_insert(elem, (foint)"hello");
	query_tree(elem, &result);
	puts(result.s);

	elem = "another[1]";
	query_tree(elem, &result);
	puts(result.s);

	tree_insert(elem, (foint)"1");
	query_tree(elem, &result);
	puts(result.s);

	create_tree("new", 2);
	for (int i=0; i<10; i++)
	{
		for (int j=0; j<10; j++)
		{
			char gen[12];
			sprintf(gen, "new[%i][%i]", i, j);
			char value[4];
			sprintf(value, "%i", i+j);
			tree_insert(gen, (foint)value);
		}
	}

	while(!tree_iter_done("new", false))
	{
		const char* i = tree_next("new");
		char next[8];
		sprintf(next, "new[%s]", i);

		while(!tree_iter_done(next, false))
		{
			const char* ret = tree_next(next);
			puts(ret);
		}
	}

	return 0;
}
