lib_files:=$(wildcard src/*.c)
lib_obj_files:=$(lib_files:.c=.o)
header_files:=$(wildcard include/*.h)
all_files:=$(lib_files) $(header_files)
PHONY:=clean
CPPFLAGS:=-fPIC -I./include -Wall -Wextra -Werror -O2

libcpu.so: $(lib_obj_files)
	$(CC) -shared $(lib_obj_files) -o libcpu.so

clean:
	rm -f *.so src/*.o
