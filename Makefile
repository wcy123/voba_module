PREFIX  = ../build
INCLUDE += -I .
INCLUDE += -I ../exec_once
INCLUDE += -I ../voba_str
INCLUDE += -I ../../vhash
INCLUDE += -I ~/d/other-working/GC/bdwgc/include
INCLUDE += -I $(PREFIX)

GC_PATH := /home/chunywan/d/other-working/GC/bdwgc/mybuild

CFLAGS   += $(INCLUDE)
CXXFLAGS += $(INCLUDE)

FLAGS += -Wall -Werror
FLAGS += -fPIC

CFLAGS += -ggdb -O0
CFLAGS += -std=c99
CFLAGS += $(FLAGS)

CXXFLAGS += -std=c++11
CXXFLAGS += $(FLAGS)

LDFLAGS  += -L $(GC_PATH) -Wl,-rpath,$(GC_PATH) -lgcmt-dll

CFLAGS += -fPIC
CFLAGS += -D_BSD_SOURCE # otherwise realpath is not defined.

all: install

install: libvoba_module.so
	install libvoba_module.so $(PREFIX)/voba/lib/
	install module.h $(PREFIX)/voba/include/module.h
	install module_end.h $(PREFIX)/voba/include/module_end.h

libvoba_module.so: module.o module_cpp.o
	$(CXX) -shared -Wl,-soname,$@  -o $@ $+

module.o: module.c module.h
module_cpp.o: module_cpp.cc module.h

clean:
	rm *.o *.so

.PHONY: all clean
