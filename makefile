All:
	gcc seeWhat.c restart.c -o seeWhat -lm -w
	gcc timerServer.c restart.c -o timerServer -lm -w
	gcc showResult.c restart.c -o showResult

clean:
	rm *.o seeWhat timerServer showResult

