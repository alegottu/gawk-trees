LW_PATH = libwayne
GAWK_PATH = gawk

all:
	gcc -fPIC -shared -DHAVE_CONFIG_H -c -O -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/bintree.c $(LW_PATH)/src/misc.c
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

debug:
	gcc -g -Og -fPIC -shared -DHAVE_CONFIG_H -c -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/bintree.c $(LW_PATH)/src/misc.c
	gcc -g -Og -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

