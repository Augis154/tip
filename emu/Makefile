LDFLAGS:=-Llib -lcpu
PHONY:=clean run

HEADERS:=-I$(realpath lib/include)
LIBRARIES:=-lcpu 
LIBPATH:=-L$(realpath lib)
LD_LIBRARY_PATH:=$(realpath lib)
export HEADERS LDFLAGS LIBRARIES LIBPATH LD_LIBRARY_PATH

frontend: libcpu.so
	$(MAKE) -C frontend 

libcpu.so:
	$(MAKE) -C lib

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C frontend clean
	rm -f *.so *.o

run: frontend
	./frontend/frontend

debug: frontend
	gdb --args env DEBUG=1 ./frontend/frontend
