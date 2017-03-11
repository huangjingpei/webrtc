SRC_DIR = ../src

# 如果有多个源文件, 用空格格开 
ENCSRCS:= $(SRC_DIR)/basic_op.o $(SRC_DIR)/bits.o\
 $(SRC_DIR)/calcexc.o\
 $(SRC_DIR)/de_acelp.o $(SRC_DIR)/dec_gain.o $(SRC_DIR)/dec_lag3.o $(SRC_DIR)/dec_ld8a.o\
 $(SRC_DIR)/dspfunc.o $(SRC_DIR)/dec_sid.o\
 $(SRC_DIR)/filter.o\
 $(SRC_DIR)/gainpred.o\
 $(SRC_DIR)/lpcfunc.o $(SRC_DIR)/lspdec.o $(SRC_DIR)/lspgetq.o\
 $(SRC_DIR)/oper_32b.o\
 $(SRC_DIR)/p_parity.o $(SRC_DIR)/post_pro.o $(SRC_DIR)/pred_lt3.o $(SRC_DIR)/postfilt.o\
 $(SRC_DIR)/qsidgain.o\
 $(SRC_DIR)/tab_dtx.o $(SRC_DIR)/tab_ld8a.o $(SRC_DIR)/taming.o\
 $(SRC_DIR)/util.o

ENCOBJS = $(ENCSRCS:.c=.o)

C_OPTIONS:=
S_OPTIONS:= -O4 -P -xassembler-with-cpp

#
# Target 
#
ENCLIB =../lib/libdec729.a
TARGETHD=../include
TARGETH =../include/Dec729.h

CC = gcc
AR = ar

INC_DIR = -I$(SRC_DIR)

ifeq ($(DEBUG),0)
  C_OPTIONS += -O2
else 
 ifeq ($(DEBUG), 1)
  C_OPTIONS += -pg 
  S_OPTIONS += -pg
 else
  C_OPTIONS += -g -O0
 endif
endif

#;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

# CFLAGS
CFLAGS += -mno-cygwin $(INC_DIR) -Wall -DNULL=0 

%.o: %.c
	$(CC) $(CFLAGS) $(C_OPTIONS) -c $< -o $@

all: $(ENCLIB)

$(ENCLIB):$(ENCOBJS)
	rm -f $(ENCLIB) $(TARGETH)
	@$(shell mkdir ../lib)
	@$(shell mkdir ../include)
	$(AR) cr $(ENCLIB) $(ENCOBJS)
	rm -f $(ENCOBJS)
	@$(foreach item,$(TARGETH),cp $(subst $(TARGETHD),$(SRC_DIR),$(item)) $(TARGETHD);)
	
clean:
	rm -f *.trace $(ENCLIB) $(ENCOBJS)
	rm -rf $(TARGETHD) 


