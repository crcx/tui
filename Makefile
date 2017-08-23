CC = clang
CFLAGS = -Wall -Wextra

default:
	$(CC) $(CFLAGS) tui.c -o tui
