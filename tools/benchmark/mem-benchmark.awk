BEGIN {
    for (a=0; a<100000000; a++)
    {
        tree_insert("test", a, rand())
    }
}
BEGIN {
    while(tree_iters_remaining("test") > 0)
    {
        a=tree_next("test")
        print query_tree("test", a)
    }
}