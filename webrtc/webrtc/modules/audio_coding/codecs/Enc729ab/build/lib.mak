SRC_DIR = ../src

# 如果有多个源文件, 用空格格开 
ENCSRCS:= $(SRC_DIR)/acelp_ca.c\
 $(SRC_DIR)/basic_op.c $(SRC_DIR)/bits.c\
 $(SRC_DIR)/calcexc.c $(SRC_DIR)/cod_ld8a.c $(SRC_DIR)/cor_func.c\
 $(SRC_DIR)/dec_sid.c $(SRC_DIR)/dspfunc.c $(SRC_DIR)/dtx.c\
 $(SRC_DIR)/filter.c\
 $(SRC_DIR)/gainpred.c\
 $(SRC_DIR)/lpc.c $(SRC_DIR)/lpcfunc.c $(SRC_DIR)/lspgetq.c\
 $(SRC_DIR)/oper_32b.c\
 $(SRC_DIR)/p_parity.c $(SRC_DIR)/pitch_a.c $(SRC_DIR)/pre_proc.c $(SRC_DIR)/pred_lt3.c\
 $(SRC_DIR)/qsidgain.c $(SRC_DIR)/qsidlsf.c $(SRC_DIR)/qua_gain.c $(SRC_DIR)/qua_lsp.c\
 $(SRC_DIR)/tab_ld8a.c $(SRC_DIR)/tab_dtx.c $(SRC_DIR)/taming.c\
 $(SRC_DIR)/util.c\
 $(SRC_DIR)/vad.c

ENCOBJS = $(ENCSRCS:.c=.o)

C_OPTIONS:=
S_OPTIONS:= -O4 -P -xassembler-with-cpp

#
# Target 
#
ENCLIB =../lib/libenc729.a
TARGETHD=../include
TARGETH =../include/Enc729.h

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


