LW_PATH = ..
GAWK_PATH = gawk
TE_PATH = tinyexpr
BASE_FLAGS = -Wno-discarded-qualifiers -Wno-incompatible-pointer-types -fPIC -shared -DHAVE_CONFIG_H -c -DTE_NAT_LOG -DTE_POW_FROM_RIGHT
INCLUDES = -Iinclude -I$(GAWK_PATH) -I$(GAWK_PATH)/pc -I$(LW_PATH)/include -I$(TE_PATH)
SOURCE = src/*.c $(TE_PATH)/tinyexpr.c
OBJ_FLAGS = -Wl,--no-undefined -shared
BUILD = bin
OUT = htrees.so
LW_NAME = libwayne.a
DEBUG_FLAGS = -g3 -Og
RELEASE_FLAGS = -Os
OPTS = $(RELEASE_FLAGS)
LW_OPTS = shared-opt

all: setup
	$(MAKE) htrees

release: all

debug: setup
	$(MAKE) OPTS='$(DEBUG_FLAGS)'\
		LW_OPTS='shared-debug'\
		OUT='ghtrees.so'\
		LW_NAME='libwayne-g.a'\
		htrees

verbose: setup
	$(MAKE) OPTS='-DVERBOSE $(DEBUG_FLAGS)'\
		LW_OPTS="'OPT=-fPIC -DVERBOSE' 'GDB=-ggdb' 'DEBUG=-DDEBUG=1' 'LIBOUT=libwayne-v.a' libwayne"\
		OUT='vhtrees.so'\
		LW_NAME='libwayne-v.a'\
		htrees

bintree: setup
	$(MAKE) OPTS='-DHTREE_USES_AVL=0 $(DEBUG_FLAGS)'\
		LW_OPTS="'OPT=-fPIC -DHTREE_USES_AVL=0' 'GDB=-ggdb' 'DEBUG=-DDEBUG=1' 'LIBOUT=libwayne-bh.a' libwayne"\
		OUT='binhtrees.so'\
		LW_NAME='libwayne-bh.a'\
		htrees

htrees:
	gcc $(OPTS) $(BASE_FLAGS) $(INCLUDES) $(SOURCE)
	$(MAKE) -C $(LW_PATH) $(LW_OPTS)
	gcc $(OPTS) $(OBJ_FLAGS) *.o -o $(BUILD)/$(OUT) -lm -L$(LW_PATH) -l:$(LW_NAME)
	rm *.o
	$(MAKE) -C $(LW_PATH) raw_clean

# Made to be entirely seperate from gawk source to test memory leaks
test: setup
	$(MAKE) -C $(LW_PATH) shared-debug
	gcc $(DEBUG_FLAGS) -Wno-discarded-qualifiers -Wno-incompatible-pointer-types $(INCLUDES) -Itest $(SOURCE) test/print_info.c test/test.c -o test/test -lm -L$(LW_PATH) -l:libwayne-g.a
	$(MAKE) -C $(LW_PATH) raw_clean

setup:
	if [ ! -d $(BUILD) ]; then \
		mkdir $(BUILD); \
		git submodule update --init --remote --recursive; \
	fi
	$(MAKE) -C $(LW_PATH) clean
