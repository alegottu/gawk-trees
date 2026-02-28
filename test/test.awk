@load "ghtrees"

function test (result, expected, message)
{
	if (result != expected)
	{
		printf("assertion failed: %s != %s (%s)\n",
			   result, expected, message) > "/dev/stderr"
        exit 1
	}
}

BEGIN {
	create_tree("pred",2)
	tree_insert("pred","a","b",1)
	test(query_tree("pred","a","b"), 1, "check numeric value after insertion")

	query_tree("pred","a","b") # test querying multiple times for the same value
	tree_insert("pred","x","y","hello")
	test(query_tree("pred","x","y"), "hello", "check string value after insertion")
	test(tree_elem_exists("pred","x","b"), 0, "elem_exists for non-existent element")
	test(tree_elem_exists("pred","x","y"), 1, "elem_exists for existing element")
	test(is_tree("pred", "x"), 1, "is_tree for subtree in 2D tree")
	test(is_tree("pred", "a", "b"), 0, "is_tree for final element")
	test(is_tree("pred", "dne"), 0, "is_tree for non-existent element")

	tree_remove("pred","x","y")
	tree_remove("pred","x","y") # should do nothing
	test(tree_elem_exists("pred","x","y"), 0, "elem_exists after removal")
	delete_tree("pred")

	create_tree("test",2)
	tree_insert("test","x","y",864)
	test(query_tree("test","x","y"), 864, "inserting into 2D tree")

	tree_insert("test","x","z", 229)
	test(query_tree("test","y","z"), "", "querying 2D tree for non-existent element")
	test(query_tree("test","x","z"), 229, "querying 2D tree for element in an existing subtree")
	test(tree_length("test"), 3, "length after insertion")
	# query_tree("test","x","y","z") # produces fatal error (no variable depth trees, using a scalar as an array)
	# query_tree("test","x") # produces fatal error (using an array as a scalar)
	
	tree_insert("example","sub","hello")
	test(query_tree("example","sub"), "hello", "inserting into a tree/value that don't yet exist")
	# query_tree("example","sub","new") # produces fatal error (no variable depth trees, using a scalar as an array)

	test(query_tree("another",1), "", "querying a tree that doesn't yet exist with number keys")
	tree_insert("another",1,1)
	test(query_tree("another",1), 1, "did not replace existing element")

	tree_remove("test","x")
	test(tree_elem_exists("test","x","y"), 0, "unsucessfully removed all elements of subtree")
	test(tree_elem_exists("test","x","z"), 0, "unsucessfully removed all elements of subtree")
	test(tree_length("test"), 1, "length after removing subtree")

	tree_increment("create",1,2,3,1) # first on a tree that doesn't exist
	test(query_tree("create",1,2,3), 1, "1st increment; create[1][2][3] != 1")

	tree_increment("another",1)
	test(query_tree("another",1), 2, "2nd increment; create[1][2][3] != 2")

	tree_increment("example","sub") # try on non-number element
	test(query_tree("example","sub"), 1, "1st decrement; example['sub'] != 1")

	tree_decrement("test","y","z")
	test(query_tree("test","y","z"), -1, "2nd decrement; test['y']['z'] != -1")

	tree_modify("another",1,"+2") # try with no "x"
	test(query_tree("another",1), 4, "1st modify; another[1] != 4")

	tree_modify("another",1,"*x") # try with element as a part of expression
	test(query_tree("another",1), 16, "2nd modify; another[1] != 16")
}

{
	if (NF == 2)
	{
		create_tree("new",2)
		for (i=0; i<$1+0; i++)
		{
			for (j=0; j<$2+0; j++)
			{
				tree_insert("new",i,j,i+j)
			}
		}

		pass=1
		while(tree_iters_remaining("new") > 0)
		{
			i=tree_next("new")
			while(tree_iters_remaining("new",i) > 0)
			{
				j=tree_next("new", i)
				if (query_tree("new", i, j) != i+j) pass=0
			}
		}
		test(pass, 1, "inserting many values/iteration")
	}
	else
	{
		create_tree("new", 1)
		for (i=0; i<$1+0; i++)
		{
			tree_insert("new",i,i/2.0)
		}

		pass=1
		while(tree_iters_remaining("new") > 0)
		{
			i = tree_next("new")
			if (query_tree("new",i) != i/2.0) pass=0
		}
		test(pass, 1, "inserting many values/iteration")
	}

	n=0
	i=-1
	while(tree_iters_remaining("new") > 0)
	{
		n++
		i=tree_next("new")
		if(n==3)
		{
			tree_iter_break("new")
			break
		}
	}
	v=tree_next("new")
	if (v==i) print "F: force break"
}
