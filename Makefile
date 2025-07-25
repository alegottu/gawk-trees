# TODO: use libwayne.a like awk_value_t branch

LW_PATH = libwayne
GAWK_PATH = gawk
TE_PATH = tinyexpr
BASE_FLAGS = -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -fPIC -shared -DHAVE_CONFIG_H -c -DTE_NAT_LOG -DTE_POW_FROM_RIGHT
INCLUDES = -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include -I$(TE_PATH)
SOURCE = src/*.c $(LW_PATH)/src/misc.c $(LW_PATH)/src/htree.c $(LW_PATH)/src/linked-list.c $(LW_PATH)/src/stack.c $(LW_PATH)/src/mem-debug.c $(TE_PATH)/tinyexpr.c
OBJ_FLAGS = -Wl,--no-undefined -shared
SO_DEFAULT = *.o -o bin/htrees.so -lm
DEBUG_FLAGS = -g3 -Og
RELEASE_FLAGS = -Os
BUILD=bin

all: setup
	gcc $(RELEASE_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE) $(LW_PATH)/src/avltree.c
	gcc $(RELEASE_FLAGS) $(OBJ_FLAGS) $(SO_DEFAULT)
	rm *.o

release: all

debug: setup
	gcc $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE) $(LW_PATH)/src/avltree.c
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) $(SO_DEFAULT)
	rm *.o

verbose: setup
	gcc -DVERBOSE $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE) $(LW_PATH)/src/avltree.c
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) *.o -o bin/vhtrees.so -lm
	rm *.o

test: setup
	gcc $(DEBUG_FLAGS) -Wno-discarded-qualifiers -Wno-incompatible-pointer-types $(INCLUDES) $(SOURCE) $(LW_PATH)/src/avltree.c tools/print_info.c tools/test.c -o bin/test -lm

# TODO: out of order until bintree query / insert also return foint*
# bintree:
# 	gcc -DHTREE_USES_AVL=0 $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE) $(LW_PATH)/src/bintree.c
# 	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) *.o -o bin/binhtrees.so -lm
# 	rm *.o

setup:
	if [ ! -d $(BUILD) ]; then \
		mkdir $(BUILD); \
		git submodule update --init --remote --recursive; \
	fi
