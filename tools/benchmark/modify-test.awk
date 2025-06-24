@load "htrees"

BEGIN {
	for (a=0; a<100; a++)
	{
		for (b=0; b<100; b++)
		{
			for (c=0; c<100; c++)
			{
				for (d=0; d<100; d++)
				{
					for (e=0; e<100; e++)
					{
						x=query_tree("test", a, b, c, d, e)
						tree_insert("test", a, b, c, d, e, x + 2*x)
					}
				}
			}
		}
	}
}
