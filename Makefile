
INCLUDE += -I .
INCLUDE += -I ../exec_once
INCLUDE += -I ../voba_str
INCLUDE += -I ../../vhash
INCLUDE += -I ../voba_value
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



CFLAGS += -fPIC
CFLAGS += -D_BSD_SOURCE # otherwise realpath is not defined.
all: libvoba_module.so

libvoba_module.so: voba_module.o
	$(CXX) -shared -Wl,-soname,$@  -o $@ $<

voba_module.o: voba_module.c voba_module.h voba_module_lex.c ../voba_value/voba_value.h ../voba_value/data_type_imp.h


voba_module_lex.c: voba_module_lex.l
	flex voba_module_lex.l
clean:
	rm *.o *.so

.PHONY: all clean
