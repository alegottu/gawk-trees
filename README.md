It's possible to compile htrees.so using the libwayne.a library like the following:
```
    gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -g -Igawk -Ilibwayne/include -Iinclude src/htrees.c
    gcc -Wl,--no-undefined -shared -o htrees.so htrees.o -Llibwayne -lwayne
```
but libwayne.a needs to be recompiled with -fPIC flags enabled in order to contribute to a shared object.

Otherwise, in order to compile htrees.so with just source and header files, do the following:
1. Clone this repository
2. Use "git submodule update --init"
3. Use "make" or "make debug"
