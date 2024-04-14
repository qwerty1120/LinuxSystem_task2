CC=gcc
OPTIONS=-g -lcrypto

OBJ = repo.c add.o commit.o remove.o log.o revert.o status.o help.o

EXECUTABLES = add commit remove log revert status help repo

all: $(EXECUTABLES)

$(EXECUTABLES): %: %.c repo_header.h
	$(CC) -o $@ $^ $(OPTIONS)

clean:
	rm -f *.o $(EXECUTABLES)
