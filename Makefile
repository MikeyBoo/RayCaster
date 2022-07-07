BIN = bin

build:
	gcc -std=c99 ./src/*.c  -lm -ldl -lSDL2 -o $(BIN)/raycast;

run:
	$(BIN)/raycast;

clean:
	@rm $(BIN)/*;