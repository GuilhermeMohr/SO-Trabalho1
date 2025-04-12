cliente: cliente.c
	gcc cliente.c -o cliente
	./cliente

servidor: servidor.c
	gcc servidor.c -o servidor -lpthread
	./servidor

clean:
	rm servidor cliente sendfifo receivefifo