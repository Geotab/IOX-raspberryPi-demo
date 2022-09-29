CFLAGS =  -Wall -lang-c-c++-comments  -Wall -g -O -c

LDFLAGS = -lm
LIBB = -L ./

CC = gcc

TARGET = iox_can_player

OBJ = main.o can.o CANRxManager.o CANTxManager.o InfoTable.o IOControl.o MessageBuilder.o NVM.o PRNG.o startup.o statemachine.o ticktimer.o MimePassthrough.o

all : $(TARGET)

$(TARGET) : $(OBJ)
	$(CC) $(LIBB) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -rf *.[oa] *.gdb *~ core $(TARGET)

O = o

%.$(O) : %.c
	$(CC) $(CFLAGS) -c $< -o $@
