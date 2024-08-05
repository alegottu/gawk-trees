It's possible to compile htrees.so using the libwayne.a library like the following:
    gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -I../gawk -Ilibwayne/include htrees.c
    gcc -Wl,--no-undefined -shared -o htrees.so htrees.o -Llibwayne -lwayne
but libwayne.a needs to be recompiled with -fPIC flags enabled in order to contribute to a shared object.

Otherwise, in order to compile htrees.so with just source and header files, do the following:
1. Copy the files htree.c, bintree.c, and misc.c from the libwayne repository into the src directory
2. Copy their matching header files into the include directory
3. Ensure htrees.c and htrees.h are also in those correct directories for header and source files
4. Use "git clone https://github.com/waynebhayes/libwayne.git"
5. Use "git clone https://git.savannah.gnu.org/r/gawk.git", then "cd gawk",
    then "git checkout gawk-3.1-stable". This is to ensure the API version used to build the shared
    library is the same version as the one in the gawk binary already installed on your system, which
    is usually version 3, while the latest version from the repository is version 4
6. Finally, use "make", or "make debug" to compile with debug flags
