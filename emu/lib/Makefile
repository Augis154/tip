lib_files:=$(wildcard src/*.c)
lib_obj_files:=$(lib_files:.c=.o)
header_files:=$(wildcard include/*.h)
all_files:=$(lib_files) $(header_files)
PHONY:=clean

ifeq ($(DEBUG), 1)
	DBGFLAGS:=-DDEBUG -g -O0
endif

CPPFLAGS:=-fPIC -I./include -Wall -Wextra -Werror -O2 $(DBGFLAGS) -g -O0

libcpu.so: $(lib_obj_files)
	$(CC) -shared $(DBGFLAGS) $(lib_obj_files) -o libcpu.so

clean:
	rm -f *.so src/*.o
