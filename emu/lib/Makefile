lib_files:=src/cpu.c
all_files:=$(lib_files) include/cpu.h
LDFLAGS:=-I./include
PHONY:=clean

libcpu.so: $(lib_files)
	$(CC) -c $(lib_files) $(LDFLAGS) -fPIC -o lib.o
	$(CC) -shared lib.o -o libcpu.so

clean:
	rm -f *.so *.o