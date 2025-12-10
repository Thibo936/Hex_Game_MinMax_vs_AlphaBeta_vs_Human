CC = gcc
CFLAGS = -fopenmp
LDFLAGS = -lm
TARGET = hex_final
SOURCES = main.c hex_game.c minimax.c alphabeta.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)