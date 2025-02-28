@load "htrees"
BEGIN {
    for (a=0; a<50000; a++)
    {
        for (b=0; b<20; b++)
        {
            tree_insert("test", a, b, rand())
        }
    }
}
BEGIN {
    while(tree_iters_remaining("test") > 0)
    {
        a=tree_next("test")
        while (tree_iters_remaining("test", a) > 0)
        {
            b=tree_next("test", a)
            print query_tree("test", a, b)
        }
    }
}