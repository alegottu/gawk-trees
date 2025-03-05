BEGIN {
    for (a=0; a<1000; a++)
    {
        for (b=0; b<1000; b++)
        {
             test[a][b]=rand()
        }
    }
}
BEGIN {
    for (a=0; a<1000; a++)
    {
        for (b=0; b<1000; b++)
        {
            print test[a][b]
        }
    }
}