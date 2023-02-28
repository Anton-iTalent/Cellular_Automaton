MF=	Makefile

CC=	mpicc -cc=icc
CFLAGS=	-O3 -Wall

LFLAGS= $(CFLAGS)

EXE=	automation2d

INC= \
	automation2d.h

SRC= \
	automation2d.c \
	unirand.c \
	automation2d_lib.c \
	cellio.c
#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(EXE)

$(OBJ):	$(INC)

$(EXE):	$(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ)

$(OBJ):	$(MF)

clean:
	rm -f $(EXE) $(OBJ) core *.out *.err
