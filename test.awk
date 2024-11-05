@load "htrees"

BEGIN {
	print "testing basics"
	create_tree("pred", 2)
	elem="pred[a][b]"
	tree_insert(elem, 1)
	print query_tree(elem)
	print query_tree(elem) # test querying multiple times for the same value
	elem="pred[x][y]"
	tree_insert(elem, "hello")
	print query_tree(elem)

	print "testing 2D trees"
	create_tree("test", 2)
	tree_insert("test[x][y]", 864)
	print query_tree("test[x][y]")
	tree_insert("test[x][z]", 229)
	print query_tree("test[y][z]")
	print query_tree("test[x][z]")
	# print query_tree("test[x][y][z]") # produces fatal error
	# print query_tree("test[x]") # produces fatal error
	
	print "testing inserting into a tree / value that does not yet exist"
	tree_insert("example[sub]", "hello")
	print query_tree("example[sub]")
	# print query_tree("example[sub][new]") # produces fatal error

	print "testing querying into a tree that doesn't yet exist with number keys"
	print query_tree("another[1]")

	print "testing inserting for the same key"
	tree_insert("another[1]", 1)
	print query_tree("another[1]")

	print "testing inserting many values"
	create_tree("new", 2)
	for (i=0; i<10; i++)
	{
		for (j=0; j<10; j++)
		{
			elem="new["i"]["j"]"
			tree_insert(elem, i + j)
		}
	}

	print "testing iteration"
	while(!tree_iter_done("new"))
	{
		i=tree_next("new")
		elem="new["i"]"

		while(!tree_iter_done(elem))
		{
			print tree_next(elem)
		}
	}
}
