
all: tcp_app upd_app mixed_app

tcp_app: tcp_client tcp_server

upd_app: upd_client upd_server

mixed_app: mixed_client mixed_server

tcp_client: tcpclient.c
	gcc -o tcpclient.out tcpclient.c

upd_client: udpclient.c
	gcc -o udpclient.out udpclient.c

tcp_server: tcpserver.c
	gcc -o tcpserver.out tcpserver.c

upd_server: udpserver.c
	gcc -o udpserver.out udpserver.c

mixed_client: mixedclient.c
	gcc -o mixedclient.out mixedclient.c

mixed_server: mixedserver.c
	gcc -o mixedserver.out mixedserver.c

clean:
	rm *.out *.o -f
 