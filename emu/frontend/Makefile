src:=$(wildcard src/*.c)
obj_files:=$(src:.c=.o)

header_files:=$(wildcard include/*.h)
PHONY:=clean
CPPFLAGS:=-I./include -Wall -Wextra -Werror -O0 $(DBGFLAGS) -g $(HEADERS) $(LIBPATH) $(LIBRARIES)

ifeq ($(DEBUG), 1)
	DBGFLAGS:=-DDEBUG -g -O0
endif

frontend: $(obj_files)
	$(CC) $(src) -o frontend $(CPPFLAGS)

clean:
	rm src/*.o
	rm frontend
