#include "htrees.h"

int main(int argc, char *argv[])
{
	// TODO: change all puts() to assert()
	init_trees();

	puts("testing basics");
	create_tree("pred", 2);
	char* subscripts[2] = {"a","b"};
	tree_insert("pred", subscripts, (foint)"1", 2); // for testing, key / value must always be as string, gawk API normally does this for us
	foint result;
	result = query_tree("pred", subscripts, 2);
	puts(result.s);
	result = query_tree("pred", subscripts, 2);
	puts(result.s);
	subscripts[0] = "x"; subscripts[1] = "y";
	tree_insert("pred", subscripts, (foint)"hello", 2);
	result = query_tree("pred", subscripts, 2);
	puts(result.s);
	subscripts[1] = "b";
	result.i = tree_elem_exists("pred", subscripts, 2);
	unsigned short test = result.i == 0;
	printf("%i\n", test);
	subscripts[1] = "y";
	result.i = tree_elem_exists("pred", subscripts, 2);
	test = result.i == 1;
	printf("%i\n", test);
	result.i = is_tree("pred", subscripts, 1);
	test = result.i == 1;
	printf("%i\n", test);
	subscripts[0] = "a"; subscripts[1] = "b";
	result.i = is_tree("pred", subscripts, 2);
	test = result.i == 0;
	printf("%i\n", test);
	subscripts[0] = "d"; // element dne
	result.i = is_tree("pred", subscripts, 1);
	test = result.i == 0;
	printf("%i\n", test);
	subscripts[0] = "x"; subscripts[1] = "y";
	tree_remove("pred", subscripts, 2);
	tree_remove("pred", subscripts, 2);
	result.i = tree_elem_exists("pred", subscripts, 2);
	test = result.i == 0;
	printf("%i\n", test);
	delete_tree("pred");


	puts("testing 2D trees");
	create_tree("test", 2);
	tree_insert("test", subscripts, (foint)"864", 2);
	result = query_tree("test", subscripts, 2);
	puts(result.s);
	subscripts[1] = "z";
	tree_insert("test", subscripts, (foint)"229", 2);
	subscripts[0] = "y";
	result = query_tree("test", subscripts, 2);
	puts(result.s);
	subscripts[0] = "x";
	result = query_tree("test", subscripts, 2);
	puts(result.s);


	puts("testing inserting into a tree / value that does not yet exist");
	char* subs[2] = {"sub", ""};
	tree_insert("example", subs, (foint)"hello", 1);
	result = query_tree("example", subs, 1);
	puts(result.s);


	puts("testing querying into a tree that doesn't yet exist with number keys");
	subs[0] = "1"; subs[1] = "\0";
	result = query_tree("another", subs, 1);
	puts(result.s);


	puts("testing inserting for the same key");
	tree_insert("another", subs, (foint)"1", 1);
	result = query_tree("another", subs, 1);
	puts(result.s);


	puts("testing removing non-final elements");
	tree_remove("test", subscripts, 1);
	subscripts[1] = "y";
	result.i = tree_elem_exists("test", subscripts, 2);
	test = result.i == 0;
	printf("%i\n", test);
	subscripts[1] = "z";
	result.i = tree_elem_exists("test", subscripts, 2);
	test = result.i == 0;
	printf("%i\n", test);


	puts("testing increment/decrement");
	char* iargs[4] = {"1","2","3","1"};
	tree_increment("create", iargs, 4);
	result = query_tree("create", iargs, 3);
	test = strcmp(result.s, "1") == 0;
	printf("%i\n", test);
	tree_increment("another", iargs, 1);
	result = query_tree("another", iargs, 1);
	test = strcmp(result.s, "2") == 0;
	printf("%i\n", test);
	subs[0] = "sub";
	tree_increment("example", subs, 1);
	result = query_tree("example", subs, 1);
	test = strcmp(result.s, "1") == 0;
	printf("%i\n", test);
	subscripts[0] = "y";
	tree_decrement("test", subscripts, 2);
	result = query_tree("test", subscripts, 2);
	test = strcmp(result.s, "-1") == 0;
	printf("%i\n", test);


	puts("testing inserting many values");
	create_tree("new", 2);
	unsigned int dimensions[2] = { atoi(argv[1]), atoi(argv[2]) };

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
			tree_insert("new", gen, (foint)value, 2);
		}
	}

	puts("testing iteration");
	while(tree_iters_remaining("new", NULL, 0) > 0)
	{
		const char* i = tree_next("new", NULL, 0);
		const char* got[1] = {i};

		while(tree_iters_remaining("new", got, 1) > 0)
		{
			const char* ret = tree_next("new", got, 1);
			puts(ret);
		}
	}

	puts("testing force break");
	while(tree_iters_remaining("new", NULL, 0) > 0)
	{
		const char* i = tree_next("new", NULL, 0);

		if (strcmp(i, "3") == 0)
		{
			tree_iter_break();
			break;
		}
	}

	const char* i = tree_next("new", NULL, 0);
	test = strcmp(i, "3") != 0; // rough test for this tree, might just want to create a seperate one
	printf("%i\n", test);

	return 0;
}
