CC = gcc
CFLAGS = -fopenmp
TARGET = hex_game
TEST_TARGET = test_ia
SOURCES = main.c hex_game.c minimax.c alphabeta.c
TEST_SOURCES = test_ia.c hex_game.c minimax.c alphabeta.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)