LW_PATH = libwayne
GAWK_PATH = gawk
TE_PATH = tinyexpr
BASE_FLAGS = -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -fPIC -shared -DHAVE_CONFIG_H -c -DTE_NAT_LOG -DTE_POW_FROM_RIGHT
INCLUDES = -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include -I$(TE_PATH)
SOURCE = src/*.c $(TE_PATH)/tinyexpr.c
OBJ_FLAGS = -Wl,--no-undefined -shared
SO_DEFAULT = *.o -o bin/htrees.so -lm
LIBS = -lm -L$(LW_PATH) -l:libwayne.a
LIBS_DEBUG = -lm -L$(LW_PATH) -l:libwayne-g.a
DEBUG_FLAGS = -g3 -Og
RELEASE_FLAGS = -Os
BUILD=bin

all: setup
	gcc $(RELEASE_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	gcc $(RELEASE_FLAGS) $(OBJ_FLAGS) $(SO_DEFAULT) $(LIBS)
	rm *.o

release: all

debug: setup
	gcc $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) $(SO_DEFAULT) $(LIBS_DEBUG)
	rm *.o

verbose: setup
	gcc -DVERBOSE $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	$(MAKE) -C libwayne clean
	$(MAKE) -C libwayne libwayne 'OPT=-fPIC -DVERBOSE' 'GDB=-ggdb' 'DEBUG=-DDEBUG=1' 'LIBOUT=libwayne-v.a'
	$(MAKE) -C libwayne raw_clean
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) *.o -o bin/vhtrees.so -lm -L$(LW_PATH) -l:libwayne-v.a
	rm *.o

test: setup
	gcc $(DEBUG_FLAGS) -Wno-discarded-qualifiers -Wno-incompatible-pointer-types $(INCLUDES) $(SOURCE) tools/print_info.c tools/test.c -o bin/test $(LIBS_DEBUG)

bintree:
	gcc -DHTREE_USES_AVL=0 $(DEBUG_FLAGS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	$(MAKE) -C libwayne clean
	$(MAKE) -C libwayne libwayne 'OPT=-fPIC -DHTREE_USES_AVL=0' 'GDB=-ggdb' 'DEBUG=-DDEBUG=1' 'LIBOUT=libwayne-bh.a'
	$(MAKE) -C libwayne raw_clean
	gcc $(DEBUG_FLAGS) $(OBJ_FLAGS) *.o -o bin/binhtrees.so -lm -L$(LW_PATH) -l:libwayne-bh.a
	rm *.o

setup:
	if [ ! -d $(BUILD) ]; then \
		mkdir $(BUILD); \
		git submodule update --init --remote --recursive; \
	fi
