cliente: cliente.c
	gcc cliente.c -o cliente
	./cliente.exe

servidor: servidor.c
	gcc servidor.c -o servidor
	./servidor.exe

clean:
	rm servidor.exe cliente.exe