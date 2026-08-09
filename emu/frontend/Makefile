src:=$(wildcard src/*.c)
PHONY:=clean
ifeq ($(DEBUG), 1)
	DBGFLAGS:=-DDEBUG -g -O0
endif

frontend: $(src)
	$(CC) $(DBGFLAGS) $(src) -o frontend $(HEADERS) $(LIBPATH) $(LIBRARIES)

clean:
	rm frontend
