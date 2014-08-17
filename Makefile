
INCLUDE += -I .
INCLUDE += -I ../../exec_once
INCLUDE += -I ../../voba_str
INCLUDE += -I ../../../vhash
INCLUDE += -I ../../voba_value/src
INCLUDE += -I ~/d/other-working/GC/bdwgc/include
LIBS += -L ~/d/other-working/GC/bdwgc/mybuild

CFLAGS   += $(INCLUDE)
CXXFLAGS += $(INCLUDE)
LDFLAGS  += $(LIBS)

FLAGS += -Wall -Werror


CFLAGS += -ggdb -O0
CFLAGS += -std=c99
CFLAGS += $(FLAGS)

CXXFLAGS += -std=c++11
CXXFLAGS += $(FLAGS)
export INCLUDE
export LIBS
export CFLAGS
export CXXFLAGS
export LDFLAGS

all: src test

.PHONY: src test

src:
	make -C $@
test:
	make -C $@

clean:
	make -C src clean; make -C test clean
