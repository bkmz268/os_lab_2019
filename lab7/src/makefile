
all: tcp udp

tcp: tcpclient tcpserver

udp: udpclient udpserver

tcpserver: tcpserver.c
	gcc tcpserver.c  -o tcpserver

tcpclient: tcpclient.c
	gcc tcpclient.c -o tcpclient

udpserver: udpserver.c
	gcc udpserver.c -o udpserver

udpclient: udpclient.c
	gcc udpclient.c -o udpclient

clean:
	rm tcpserver tcpclient udpserver udpclient
