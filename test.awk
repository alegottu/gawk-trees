@load "htrees"

BEGIN {
	# test basics
	create_tree("pred", 2)
	elem = "pred[a][b]"
	tree_insert(elem, 1)
	print query_tree(elem)
	print query_tree(elem) # test querying multiple times for the same value
	elem = "pred[x][y]"
	tree_insert(elem, "hello")
	print query_tree(elem)

	# test 2D tree
	create_tree("test", 2)
	tree_insert("test[x][y]", 864)
	print query_tree("test[x][y]")
	tree_insert("test[x][z]", 229)
	print query_tree("test[y][z]")
	print query_tree("test[x][z]")
	# print query_tree("test[x][y][z]") # produces fatal error
	# print query_tree("test[x]") # produces fatal error
	
	# test inserting into a tree / value that does not yet exist
	tree_insert("example[sub]", "hello")
	print query_tree("example[sub]")
	# print query_tree("example[sub][new]") # produces fatal error

	# test querying into a tree that doesn't yet exist with number keys
	print query_tree("another[1]")

	# test inserting for the same key
	tree_insert("another[1]", 1)
	print query_tree("another[1]")

	# test inserting many values
	create_tree("new", 2)
	for (i=0; i<10; i++)
	{
		for (j=0; j<10; j++)
		{
			elem="new["i"]["j"]"
			tree_insert(elem, i + j)
		}
	}

	# test iteration
	do
	{
		print get_tree_next("new")
	}
	while (is_current_tree_done() == 0)
}
