CFLAGS += -I../src
TEST_FILES := $(wildcard test_*.c)
TESTS := $(patsubst %.c,%, $(TEST_FILES))
TEST_INPUTS :=$(patsubst %.c,%.in, $(TEST_FILES))
TEST_OUTPUTS :=$(patsubst %.c,%.out, $(TEST_FILES))

LDFLAGS += -L../src 
LDFLAGS += -Wl,-rpath,../src -lvoba_value

all: $(TESTS) $(TEST_OUTPUTS)

%.out: %.in ../src/libvoba_value.so %
	./$(patsubst %.out,% , $@) < $(patsubst %.out,%.in, $@) > $@

%.out: %

%: %.c

clean:
	rm $(TESTS) $(TEST_OUTPUTS)

.PHONY: clean



