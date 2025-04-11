LW_PATH = libwayne
GAWK_PATH = $(LW_PATH)/gawk
LIB_DIR = -L$(LW_PATH)
LIB_FILES = -l:libwayne.a -lm
BASE_FLAGS = -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -fPIC -shared -DHAVE_CONFIG_H -c
INCLUDES = -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include
SOURCE = src/*.c 
OBJ_FLAGS = -Wl,--no-undefined -shared
SO_DEFAULT = *.o -o bin/htrees.so
DEBUG_FLAGS = -g -Og
RELEASE_FLAGS = -Os

# release
all:
	gcc $(RELEASE_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	gcc $(RELEASE_FLAGS) $(OBJ_FLAGS) $(LIB_DIR) $(SO_DEFAULT) $(LIB_FILES)
	rm *.o

debug:
	gcc $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) $(LIB_DIR) $(SO_DEFAULT) $(LIB_FILES)
	rm *.o

test:
	gcc $(DEBUG_FLAGS) -Wno-discarded-qualifiers -Wno-incompatible-pointer-types $(LIB_DIR) $(INCLUDES) $(SOURCE) tools/print_info.c tools/test.c -o bin/test $(LIB_FILES)

bintree:
	gcc -DHTREE_USES_AVL=0 $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) $(LIB_DIR) *.o -o bin/binhtrees.so -lm $(LIB_FILES)
	rm *.o

