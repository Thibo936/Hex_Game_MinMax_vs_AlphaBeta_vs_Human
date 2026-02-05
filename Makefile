CC = gcc
CFLAGS = -fopenmp
LDFLAGS = -lm
TARGET = hex_game
TEST_TARGET = test_ia
TEST2_TARGET = test2
SOURCES = main.c hex_game.c minimax.c alphabeta.c mcts.c
TEST_SOURCES = test_ia.c hex_game.c minimax.c alphabeta.c mcts.c
TEST2_SOURCES = test2.c hex_game.c minimax.c alphabeta.c mcts.c

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

test: $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET) $(LDFLAGS)

test2: $(TEST2_SOURCES)
	$(CC) $(CFLAGS) $(TEST2_SOURCES) -o $(TEST2_TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(TEST2_TARGET)
