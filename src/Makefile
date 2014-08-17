CFLAGS += -fPIC
CFLAGS += -D_BSD_SOURCE # otherwise realpath is not defined.
all: libvoba_module.so

libvoba_module.so: voba_module.o
	$(CXX) -shared -Wl,-soname,$@  -o $@ $<

voba_module.o: voba_module.c voba_module.h voba_module_lex.c


voba_module_lex.c: voba_module_lex.l
	flex voba_module_lex.l
clean:
	rm *.o *.so

.PHONY: all clean
