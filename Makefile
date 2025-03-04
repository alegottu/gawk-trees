LW_PATH = libwayne
GAWK_PATH = gawk

all:
	gcc -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -fPIC -shared -DHAVE_CONFIG_H -c -Os -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/avltree.c $(LW_PATH)/src/misc.c $(LW_PATH)/src/linked-list.c $(LW_PATH)/src/mem-debug.c
	gcc -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

debug:
	gcc -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -g -Og -fPIC -shared -DHAVE_CONFIG_H -c -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/avltree.c $(LW_PATH)/src/misc.c $(LW_PATH)/src/linked-list.c $(LW_PATH)/src/mem-debug.c
	gcc -g -Og -Wl,--no-undefined -shared -o htrees.so *.o -lm
	rm *.o

test:
	gcc -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -g -Og -Iinclude -Itools -I$(GAWK_PATH) -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/avltree.c $(LW_PATH)/src/misc.c $(LW_PATH)/src/linked-list.c $(LW_PATH)/src/mem-debug.c tools/print_info.c tools/test.c -o test -lm

test-release:
	gcc -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -Os -Iinclude -Itools -I$(GAWK_PATH) -I$(LW_PATH)/include src/*.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/avltree.c $(LW_PATH)/src/misc.c $(LW_PATH)/src/linked-list.c $(LW_PATH)/src/mem-debug.c tools/print_info.c tools/test.c -o test -lm
