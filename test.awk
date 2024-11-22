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
}

{
	print "testing inserting many values"
	create_tree("new", 1)
	for (i=0; i<$1+0; i++)
	{
		elem="new["i"]"
		tree_insert(elem, 100-i)
	}

	print "testing iteration"
	while(!tree_iter_done("new"))
	{
		i=tree_next("new")
		print i
	}
}
