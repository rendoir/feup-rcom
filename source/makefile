nonCanonical: sender.c
	gcc receiver.c dataLink.c serialPort.c -Wall -o receiver
	gcc sender.c dataLink.c serialPort.c -Wall -o sender
clean:
	rm -rf *.o
	rm -rf receiver
	rm -rf sender
