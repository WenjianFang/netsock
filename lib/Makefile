DIR=.

BIN=$(DIR)/libnetsock.so

SHARE=--share

CFLAG=-g -fPIC -lpthread

LFLAG=$(SHARE)

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

install:
	cp libnetsock.so /usr/lib32/

.PHONY:clean
clean:
	rm $(OBJ) $(BIN)