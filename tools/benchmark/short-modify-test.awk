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
						tree_modify("test", a, b, c, d, e, "+2*x")
					}
				}
			}
		}
	}
}
