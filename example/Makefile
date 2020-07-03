LIB_DIR = ../src

LIB_SRC = $(LIB_DIR)/ctlseq.c \
          $(LIB_DIR)/emsh.c \
          $(LIB_DIR)/numcast10.c

LIB_INC = $(LIB_DIR)/ascii.h \
          $(LIB_DIR)/bytearray.h \
          $(LIB_DIR)/ctlseq.h \
          $(LIB_DIR)/emsh.h \
          $(LIB_DIR)/list.h \
          $(LIB_DIR)/numcast10.h

CC ?= clang
CFLAGS += -std=c99 -Wall -W -Wextra -Wpedantic -Werror -O3 -fomit-frame-pointer -ftree-vectorize -I$(LIB_DIR)

console: console.c $(LIB_SRC) $(LIB_INC)
	$(CC) $(CFLAGS) $(filter %.c,$^) $(LDFLAGS) -o $@

clean:
	$(RM) -r console
