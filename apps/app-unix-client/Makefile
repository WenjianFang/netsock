DIR=.

BIN=$(DIR)/app-unix-sock-client

SHARE=--share

CFLAG=-g -Wall -I../../lib

LFLAG=-L../../lib -lnetsock -lpthread

CC=gcc -o

AR=ar -cr

CO=$(CC)

SRC=$(wildcard $(DIR)/*.c)
OBJ=$(patsubst %.c, %.o, $(SRC))

all:$(BIN)
$(BIN):$(OBJ)
	$(CO) $@ $^ $(LFLAG)

$(DIR)/%.o:$(DIR)/%.c
	$(CC) $@ -c $< $(CFLAG)

.PHONY:clean
clean:
	rm $(OBJ) $(BIN)