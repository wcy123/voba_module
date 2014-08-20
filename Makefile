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
	install voba_module.h $(PREFIX)/voba/include/module.h
	install voba_module_end.h $(PREFIX)/voba/include/module_end.h

libvoba_module.so: voba_module.o
	$(CXX) -shared -Wl,-soname,$@  -o $@ $<

voba_module.o: voba_module.c voba_module.h voba_module_lex.c


voba_module_lex.c: voba_module_lex.l
	flex voba_module_lex.l
clean:
	rm *.o *.so

.PHONY: all clean
