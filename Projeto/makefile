PROTOC_DIR=/usr/local/
LDFLAGS = ${PROTOC_DIR}lib/libprotobuf-c.a

all: data.o entry.o tree.o sdmessage.pb-c.c sdmessage.pb-c.o message.o tree-client.o client_stub.o network-client.o client-lib.o tree_skel.o tree-server.o network_server.o tree-client tree-server 

data.o:
	gcc -Wall -I include -g -o object/data.o -c source/data.c

entry.o: include/data.h
	gcc -Wall -g -I include -c source/entry.c -o ./object/entry.o

tree.o:
	gcc -Wall -I include -g -o object/tree.o -c source/tree.c
	
client_stub.o: 
	gcc -Wall -I include -g -o object/client_stub.o -c source/client_stub.c

network-client.o:
	gcc -I include -g -o object/network-client.o -c source/network-client.c
	
sdmessage.pb-c.c:
	protoc --c_out=source include/sdmessage.proto
	mv source/include/sdmessage.pb-c.c source
	cp source/include/sdmessage.pb-c.h include/sdmessage.pb-c.h
	
sdmessage.pb-c.o:
	gcc -Wall -c source/sdmessage.pb-c.c -o object/sdmessage.pb-c.o	

message.o: source/include/sdmessage.pb-c.h include/message-private.h
	gcc -Wall -I include -g -o object/message.o -c source/message.c
	
client-lib.o: 
	ld -r object/message.o object/client_stub.o object/network-client.o object/data.o object/entry.o object/sdmessage.pb-c.o $(LDFLAGS) -o lib/client-lib.o

tree-client.o: include/client_stub.h include/client_stub-private.h
	gcc -I include -c source/tree-client.c -g -o object/tree-client.o

tree-client: lib/client-lib.o object/tree-client.o
	gcc lib/client-lib.o object/tree-client.o -g $(LDFLAGS) -o binary/tree-client -lzookeeper_mt -DTHREADED
	
network_server.o: include/tree_skel.h include/message-private.h include/sdmessage.pb-c.h
	gcc -I include -g -o object/network_server.o -c source/network_server.c
	
tree_skel.o: source/include/sdmessage.pb-c.h include/tree.h include/message-private.h
	gcc -I include -g -o object/tree_skel.o -c source/tree_skel.c

tree-server.o: include/client_stub.h include/client_stub-private.h include/tree_skel.h include/tree_skel-private.h
	gcc -I include -c source/tree-server.c -g -o object/tree-server.o
	
tree-server: object/network_server.o object/tree-server.o object/tree_skel.o object/message.o object/data.o object/entry.o  object/tree.o
	gcc -pthread object/data.o object/entry.o  object/tree.o object/network_server.o object/tree_skel.o object/message.o object/sdmessage.pb-c.o object/client_stub.o object/network-client.o object/tree-server.o -g $(LDFLAGS) -o binary/tree-server -lzookeeper_mt -DTHREADED
	
	
clean:
	rm -f object/data.o
	rm -f object/entry.o
	rm -f object/tree.o
	rm -f lib/client-lib.o
	rm -f object/client_stub.o
	rm -f object/message.o
	rm -f object/network-client.o
	rm -f object/network_server.o
	rm -f object/sdmessage.pb-c.o
	rm -f object/tree-client.o
	rm -f object/tree-server.o
	rm -f object/tree_skel.o
	rm -f binary/tree-client
	rm -f binary/tree-server
	
