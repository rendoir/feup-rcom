nonCanonical: sender.c
	gcc receiver.c dataLink.c serialPort.c -std=c11 -Wall -o receiver
	gcc sender.c dataLink.c serialPort.c -std=c11 -Wall -o sender
clean:
	rm -rf *.o
	rm -rf receiver
	rm -rf sender
