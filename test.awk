@load "htrees"

# TODO: update test-no-ext.awk and test.c with testing for removal, element existence, delete array, is array

BEGIN {
	print "testing basics"
	create_tree("pred",2)
	tree_insert("pred","a","b",1)
	print query_tree("pred","a","b")
	print query_tree("pred","a","b") # test querying multiple times for the same value
	tree_insert("pred","x","y","hello")
	print query_tree("pred","x","y")
	print tree_elem_exists("pred","x","b") == 0
	print tree_elem_exists("pred","x","y") == 1
	print is_tree("pred", "x") == 1
	print is_tree("pred", "a", "b") == 0
	tree_remove("pred","x","y")
	tree_remove("pred","x","y") # should do nothing
	print tree_elem_exists("pred","x","y") == 0
	delete_tree("pred") # TODO: tree_exists function?

	print "testing 2D trees"
	create_tree("test",2)
	tree_insert("test","x","y",864)
	print query_tree("test","x","y")
	tree_insert("test","x","z", 229)
	print query_tree("test","y","z")
	print query_tree("test","x","z")
	# print query_tree("test","x","y","z") # produces fatal error
	# print query_tree("test","x") # produces fatal error
	
	print "testing inserting into a tree / value that does not yet exist"
	tree_insert("example","sub","hello")
	print query_tree("example","sub")
	# print query_tree("example","sub","new") # produces fatal error

	print "testing querying into a tree that doesn't yet exist with number keys"
	print query_tree("another",1)

	print "testing inserting for the same key"
	tree_insert("another",1,1)
	print query_tree("another",1)

	print "testing removing non-final elements"
	tree_remove("test","x")
	print tree_elem_exists("test","x","y") == 0
	print tree_elem_exists("test","x","z") == 0
}

{
	print "testing inserting many values"
	create_tree("new",2)
	for (i=0; i<$1+0; i++)
	{
		for (j=0; j<$1+0; j++)
		{
			tree_insert("new",i,j,i+j)
		}
	}

	print "testing iteration"
	while(!tree_iter_done("new"))
	{
		i=tree_next("new")
		while(!tree_iter_done("new",i))
		{
			print tree_next("new",i)
		}
	}
}
