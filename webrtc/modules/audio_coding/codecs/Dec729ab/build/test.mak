APP_DIR = ../test

CC = gcc
LD = gcc
PP = gprof

BIN:=$(APP_DIR)/decoder.exe


# 如果有多个源文件, 用空格格开 
CSRCS:= $(APP_DIR)/decoder.c
COBJS = $(CSRCS:.c=.o)

SSRCS:=
SOBJS = $(SSRCS:.s=.o)

C_OPTIONS:=
S_OPTIONS:= -O2 -P -xassembler-with-cpp

INC_DIR = -I../include
LIB_DIR = -L../lib

# CFLAGS
CFLAGS = -mno-cygwin $(INC_DIR) -Wall -DNULL=0 -DWord16=short -DWord32=long

# LDFLAGS
LDFLAGS = -mno-cygwin $(LIB_DIR)

DECLIB = -ldec729

ifeq ($(DEBUG),1)
  C_OPTIONS += -pg
  LDFLAGS += -pg
  TARGET = profile
else 
  C_OPTIONS += -g -O0
  TARGET = $(BIN)
endif


OBJS = $(COBJS) $(SOBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(C_OPTIONS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) $(S_OPTIONS) -c $< -o $@

all: $(TARGET)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJS) $(DECLIB) 
	rm -f $(OBJS)
	
profile :$(BIN)
	cd $(APP_DIR) && $(BIN) && $(PP) $(P_OPTIONS) $(BIN) > profile.txt
		
clean:
	rm -f $(BIN) $(OBJS) $(TARGET) profile.txt *.out

