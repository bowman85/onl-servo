all: onl-servo

one: test

CC=gcc

test:	servo.o	functions.o	
	$(CC) servo.o functions.o  -o test

servo.o:	servo.c
	$(CC) -c servo.c

functions.o: functions.c
	$(CC) -c functions.c
	

onl-servo: functions.c servo.c
	$(CC) -o onl-servo functions.c servo.c 

	
retro: retranspare.c
	$(CC) -o retro retranspare.c	

