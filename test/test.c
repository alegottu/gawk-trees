#include "htrees.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	init_trees();

	create_tree("pred", 2);
	char* subscripts[2] = {"a","b"};
	tree_insert("pred", subscripts, (foint)"1", 2);
	assert(strcmp(query_tree("pred", subscripts, 2), "1") == 0);

	query_tree("pred", subscripts, 2);
	subscripts[0] = "x"; subscripts[1] = "y";
	tree_insert("pred", subscripts, (foint)"hello", 2);
	assert(strcmp(query_tree("pred", subscripts, 2), "hello") == 0);
	subscripts[1] = "b";
	assert(tree_elem_exists("pred", subscripts, 2) == 0);
	subscripts[1] = "y";
	assert(tree_elem_exists("pred", subscripts, 2) == 1);
	assert(is_tree("pred", subscripts, 1) == 1);
	subscripts[0] = "a"; subscripts[1] = "b";
	assert(is_tree("pred", subscripts, 2) == 0);

	subscripts[0] = "x"; subscripts[1] = "y";
	tree_remove("pred", subscripts, 2);
	tree_remove("pred", subscripts, 2);
	assert(tree_elem_exists("pred", subscripts, 2) == 0);
	delete_tree("pred");

	create_tree("test", 2);
	tree_insert("test", subscripts, (foint)"864", 2);
	assert(strcmp(query_tree("test", subscripts, 2), "864") == 0);

	subscripts[1] = "z";
	tree_insert("test", subscripts, (foint)"229", 2);
	subscripts[0] = "y";
	assert(strcmp(query_tree("test", subscripts, 2), "") == 0);
	subscripts[0] = "x";
	assert(strcmp(query_tree("test", subscripts, 2), "229") == 0);
	assert(tree_length("test") == 3);

	char* subs[2] = {"sub", ""};
	tree_insert("example", subs, (foint)"hello", 1);
	assert(strcmp(query_tree("example", subs, 1), "hello") == 0);

	subs[0] = "1";
	assert(strcmp(query_tree("another", subs, 1), "") == 0);
	tree_insert("another", subs, (foint)"1", 1);
	assert(strcmp(query_tree("another", subs, 1), "1") == 0);

	tree_remove("test", subscripts, 1);
	subscripts[1] = "y";
	assert(tree_elem_exists("test", subscripts, 2) == 0);
	subscripts[1] = "z";
	assert(tree_elem_exists("test", subscripts, 2) == 0);

	char* iargs[4] = {"1","2","3","1"};
	tree_increment("create", iargs, 4);
	assert(strcmp(query_tree("create", iargs, 3), "1") == 0);

	tree_increment("create", iargs, 4);
	assert(strcmp(query_tree("create", iargs, 3), "2") == 0);

	subs[0] = "sub";
	tree_increment("example", subs, 1);
	assert(strcmp(query_tree("example", subs, 1), "1") == 0);

	subscripts[0] = "y";
	tree_decrement("test", subscripts, 2);
	assert(strcmp(query_tree("test", subscripts, 2), "-1") == 0);

	subscripts[0] = "1";
	tree_modify("another", subscripts, 1, "+2");
	assert(strcmp(query_tree("another", subscripts, 1), "3") == 0);

	tree_modify("another", subscripts, 1, "*x");
	assert(strcmp(query_tree("another", subscripts, 1), "9") == 0);

	create_tree("new", 2);
	unsigned int dimensions[2] = { atoi(argv[1]), argc == 2 ? atoi(argv[1]) : atoi(argv[2]) };

	for (unsigned int i=0; i<dimensions[0]; i++)
	{
		for (unsigned int j=0; j<dimensions[1]; j++)
		{
			char gen0[dimensions[0]+1]; char gen1[dimensions[1]+1];
			char* gen[2] = {gen0, gen1};
			sprintf(gen0, "%u", i);
			sprintf(gen1, "%u", j);
			char value[11];
			sprintf(value, "%u", i+j);
			tree_insert("new", gen, (foint){.s=value}, 2);
		}
	}

	while(tree_iters_remaining("new", NULL, 0) > 0)
	{
		char* idx[2] = {tree_next("new", NULL, 0), ""};

		while(tree_iters_remaining("new", idx, 1) > 0)
		{
			idx[1] = tree_next("new", idx, 1);
			char buf[11];
			sprintf(buf, "%u", atoi(idx[0]) + atoi(idx[1]));
			char value[strlen(buf)];
			strcpy(value, buf);
			assert(strcmp(query_tree("new", idx, 2), value) == 0);
		}
	}

	unsigned int n = 0;
	char* i;

	while(tree_iters_remaining("new", NULL, 0) > 0)
	{
		i = tree_next("new", NULL, 0);
		n++;

		if (n == 3)
		{
			tree_iter_break();
			break;
		}
	}

	assert(tree_next("new", NULL, 0) != i);
	return 0;
}
