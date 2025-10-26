.PHONY: run default clean
TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -l -a "Timmy H.,123 Sheshire Ln.,120"
	./$(TARGET) -f ./mynewdb.db -l -a "John D.,42 Hollywood Blv.,100"
	./$(TARGET) -f ./mynewdb.db -l -u "John D.,42 Hollywood Blv.,180"
	./$(TARGET) -f ./mynewdb.db -l -r "Timmy H."

default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET): $(OBJ)
	gcc -o $@ $? -Wall -pedantic

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude -Wall -pedantic
