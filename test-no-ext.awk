BEGIN {
	print "testing basics"
	pred[a][b] = 1
	print pred[a][b]
	print pred[a][b]
	pred[x][y] = "hello"
	print pred[x][y]

	print "testing 2D trees"
	test[x][y] = 864
	print test[x][y]
	test[x][z] = 229
	print test[y][z]
	print test[x][z]
	
	print "testing inserting into a tree / value that does not yet exist"
	example["sub"] = "hello"
	print example["sub"]

	print "testing querying into a tree that doesn't yet exist with number keys"
	print another[1]

	print "testing inserting for the same key"
	another[1] = 1
	print another[1]

	print "testing inserting many values"
	for (i=0; i<2000; i++)
	{
		for (j=0; j<2000; j++)
		{
			new[i][j] = i+j
		}
	}

	print "testing iteration"
	for (i in new)
	{
		for (j in new[i])
		{
			print j 
		}
	}	
}
