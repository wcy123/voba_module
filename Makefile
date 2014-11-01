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


CFLAGS += -std=c99
CFLAGS += $(FLAGS)

CXXFLAGS += -std=c++11
CXXFLAGS += $(FLAGS)

LDFLAGS  += -L $(GC_PATH)  -lgcmt-dll
#-Wl,-rpath,$(GC_PATH)
CFLAGS += -fPIC
CFLAGS += -D_BSD_SOURCE # otherwise realpath is not defined.

all: install

C_SRCS += module.c
OBJS += $(patsubst %.c,%.o,$(C_SRCS))
libvoba_module.so: $(OBJS)
	$(CXX) -shared -Wl,-soname,$@  -o $@ $+ $(LDFLAGS)

module.o: module.c module.h


clean:
	rm *.o *.so

.PHONY: all clean


INSTALL_FILES += $(PREFIX)/voba/lib/libvoba_module.so
INSTALL_FILES += $(PREFIX)/voba/include/module.h
INSTALL_FILES += $(PREFIX)/voba/include/module_end.h

install: $(INSTALL_FILES)

$(PREFIX)/voba/lib/libvoba_module.so:  libvoba_module.so
	install libvoba_module.so $(PREFIX)/voba/lib/
$(PREFIX)/voba/include/module.h: module.h
	install module.h $(PREFIX)/voba/include/module.h
$(PREFIX)/voba/include/module_end.h: module_end.h
	install module_end.h $(PREFIX)/voba/include/module_end.h

.PHONY: depend
depend: 
	for i in $(C_SRCS); do $(CC) -MM $(CFLAGS) $$i; done > $@.inc
-include depend.inc
