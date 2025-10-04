Usage: mem-benchmark.py [-i|--iteration] [-v|--verbose] [-m|--massif] [-b|--bintree] [-p|--print n] dimensions

"dimensions" should be formated as x-y for each tree structure, e.g. mem-benchmark.py 100-100 250
to test two tree structures equivalent to an array of [100][100] and 250 elements respectively;
you can also use the short hand AxB, e.g. 100-100x5 to mean 5 tree structures equivalent to
an array of [100][100], as well as the shorthand A^B, e.g. 100^3 to mean a tree structure
equivalent to an array of [100][100][100], and you may combine these shorthands together

--iteration: also test iterating through each element of each tree structure
--verbose: also print info about the number of steps needed to traverse AVL trees during key operations;
    make sure you've built the "verbose" target if you want to use this
--massif: use valgrind --tool=massif to profile instead of time -v; outputs *.massif, see below
--bintree: test the AVL tree version of htrees against the regular binary tree version""");
    make sure you've built the "bintree" target if you want to use this
--print: just print the generated script according to the provided argument n,
    1 being the extension version, 2 being the regular version

By default, benchmarks will use the time command and put the Maximum resident sizes,
as well as the wall-clock times elapsed by both tests (with and without the extension,
or with two different versions of the extensions in the case of --bintree) into a
".data" file, where the prefix is whatever dimensions you supplied for the test.
If you use the --massif flag, two massif files "ext.massif" and "normal.massif" will be
created, both under a directory in the logs folder named after the dimensions supplied.

If you'd like to see exactly what the tests are doing, check out the hidden files
".mem-benchmark.awk" and ".mem-benchmark-no-ext.awk" for the code that the script
generates then runs. When --bintree is used, the first script is used for both tests,
being run with different arguments to load a different library file for the extension.

make_graph.py is an internal tool to make graphs using matplotlib from the data output by make-table.sh,
to use it make sure to create a virtual environment with matplotlib installed.
