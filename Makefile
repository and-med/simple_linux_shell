CC=gcc
OBJECTS=build/main.o build/utilities.o build/jobarray.o build/jobmanager.o

build-dir :
	mkdir -p build

build/%.o : %.c build-dir
	$(CC) -c $< -o $@ -g

update: $(OBJECTS)

build: $(OBJECTS)
	$(CC) -o prog $(OBJECTS) -g

clean :
	rm -rf build