CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = flappy_bird
SOURCE = flappy_bird.c

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

install:
	@echo "Dependências do sistema necessárias:"
	@echo "- gcc (compilador C)"
	@echo "- make"
	@echo "- terminal com suporte a ANSI escape codes"
	@echo ""
	@echo "Para macOS:"
	@echo "  xcode-select --install"
	@echo ""
	@echo "Para Ubuntu/Debian:"
	@echo "  sudo apt-get install build-essential"
	@echo ""
	@echo "Para CentOS/RHEL:"
	@echo "  sudo yum groupinstall 'Development Tools'"

.PHONY: clean run debug install