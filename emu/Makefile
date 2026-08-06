LDFLAGS:=-Llib -lcpu
PHONY:=clean run

HEADERS:=-I$(realpath lib/include)
LIBRARIES:=-lcpu 
LIBPATH:=-L$(realpath lib)
LD_LIBRARY_PATH:=$(realpath lib)
export HEADERS LDFLAGS LIBRARIES LIBPATH LD_LIBRARY_PATH

frontend: libcpu.so
	$(MAKE) -C frontend 
	cp frontend/frontend build/frontend
	cp lib/libcpu.so build/libcpu.so

libcpu.so:
	$(MAKE) -C lib

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C frontend clean
	rm -f *.so *.o

run: frontend
	./frontend/frontend