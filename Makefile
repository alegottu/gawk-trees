LW_PATH = libwayne
GAWK_PATH = ../gawk

all:
	gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include -Iinclude src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/bintree.c $(LW_PATH)/src/misc.c
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

debug:
	gcc -g -fPIC -shared -DHAVE_CONFIG_H -c -O0 -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include -Iinclude src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/bintree.c $(LW_PATH)/src/misc.c
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

