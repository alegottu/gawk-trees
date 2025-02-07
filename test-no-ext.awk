BEGIN {
	print "testing basics"
	pred[a][b] = 1
	print pred[a][b]
	print pred[a][b]
	pred[x][y] = "hello"
	print pred[x][y]
	# for each in statement might need to expect only one level down
	print pred[x][b] in pred == 0
	print pred[x][y] in pred == 1
	print isarray(pred[x]) == 1
	print isarray(pred[a][b]) == 0
	delete pred[x][y]
	delete pred[x][y]
	print pred[x][y] in pred == 0
	delete pred

	print "testing 2D trees"
	test[x][y] = 864
	print test[x][y]
	test[x][z] = 229
	print test[y][z]
	print test[x][z]
	# print test[x] # also produces error
	
	print "testing inserting into a tree / value that does not yet exist"
	example["sub"] = "hello"
	print example["sub"]

	print "testing querying into a tree that doesn't yet exist with number keys"
	print another[1]

	print "testing inserting for the same key"
	another[1] = 1
	print another[1]

	print "testing removing non-final elements"
	delete test[x]
	print test[x][y] in test == 0
	print test[x][z] in test == 0
}

{
	print "testing inserting many values"

	if (NF == 2)
	{
		for (i=0; i<$1+0; i++)
		{
			for (j=0; j<$2+0; j++)
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
	else
	{
		for (i=0; i<$1+0; i++)
		{
			new[i] = i/2.0
		}

		print "testing iteration"
		for (i in new)
		{
			print new[i]
		}
	}

	print "testing force break"
	for (i in new)
	{
		if (i == "3")
		{
			break
		}
	}

	for (i in new)
	{
		print i != "3"
		break
	}
}
