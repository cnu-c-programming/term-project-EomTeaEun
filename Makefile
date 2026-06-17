CC     = gcc
CFLAGS = -Wall -Wextra -std=c11

# TODO: Add all your .c source files here (e.g., student.c file_io.c command.c)
SRCS    = main.c student.c file_io.c command.c

all: admin client

.PHONY: all admin client clean

# Admin Program: ADMIN_MODE 플래그로 모든 명령어 포함
admin:
	$(CC) $(CFLAGS) -DADMIN_MODE $(SRCS) -o admin_shell

# Client Program: CLIENT_MODE 플래그로 조회 명령어만 포함
client:
	$(CC) $(CFLAGS) -DCLIENT_MODE $(SRCS) -o client_shell

clean:
	rm -f admin_shell client_shell *.o
