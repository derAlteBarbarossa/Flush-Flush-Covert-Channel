SRC = main.c sender.c receiver.c
OBJ = $(addsuffix .o, $(basename $(SRC)))
TARGET = $(basename $(OBJ))
CC = gcc
CCFLAGS = 

all: $(TARGET)

main: main.o util.o ECC.o
	${CC} ${CCFLAGS} -o $@ main.o util.o ECC.o

sender:	sender.o util.o ECC.o
	${CC} ${CCFLAGS} -o $@ sender.o util.o ECC.o

receiver: receiver.o util.o ECC.o
	${CC} ${CCFLAGS} -o $@ receiver.o util.o ECC.o

util.o: util.c util.h
	${CC} ${CCFLAGS} -c $<

ECC.o: ECC.c ECC.h
	${CC} ${CCFLAGS} -c $<

.c.o:
	${CC} ${CCFLAGS} -c $<

clean:
	rm -f *.o $(TARGET) 
