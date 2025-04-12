cliente: cliente.c
	gcc cliente.c -o cliente
	./cliente

servidor: servidor.c
	gcc servidor.c -o servidor
	./servidor

clean:
	rm servidor cliente