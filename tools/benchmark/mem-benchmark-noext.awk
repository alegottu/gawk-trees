BEGIN {
    for (a=0; a<1000000; a++)
    {
         test[a]=rand()
    }
}
BEGIN {
    for (a=0; a<1000000; a++)
    {
        print test[a]
    }
}