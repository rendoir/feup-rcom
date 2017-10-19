nonCanonical: sender.c
	gcc receiver.c dataLink.c state_machine.c serialPort.c string.c -std=c11 -Wall -o receiver
	gcc sender.c dataLink.c state_machine.c  serialPort.c string.c -std=c11 -Wall -o sender
clean:
	rm -rf *.o
	rm -rf receiver
	rm -rf sender
