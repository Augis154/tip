src:=src/main.c
PHONY:=clean

frontend: $(src)
	$(CC) $(src) -o frontend $(HEADERS) $(LIBPATH) $(LIBRARIES)

clean:
	rm frontend