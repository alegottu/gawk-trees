BEGIN {
    for (a=0; a<50000; a++)
    {
        for (b=0; b<20; b++)
        {
             test[a][b]=rand()
        }
    }
}
BEGIN {
    for (a=0; a<50000; a++)
    {
        for (b=0; b<20; b++)
        {
            print test[a][b]
        }
    }
}