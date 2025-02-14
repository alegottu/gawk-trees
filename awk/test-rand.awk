@load "htrees"

BEGIN { create_tree("test", 2) }

{
	tree_insert("test", $1, $2, $3)
	print query_tree("test", $1, $2), $3

	if (FNR % 1 == 0)
	{
		get=query_tree("test", $1, $2);
		if (get != $3)
			printf "value should be \"%s\", got \"%s\"\n", $3, get; 
	}
}

