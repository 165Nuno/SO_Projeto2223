all: folders server client

server: bin/monitor

client: bin/tracer

folders:
	@mkdir -p src obj bin tmp

bin/monitor: obj/monitor.o obj/execucao.o obj/ListaLigadaExec.o
	gcc -g obj/monitor.o obj/execucao.o obj/ListaLigadaExec.o -o bin/monitor

obj/monitor.o: src/server/monitor.c
	gcc -Wall -g -c src/server/monitor.c -o obj/monitor.o

obj/execucao.o: src/server/execucao.c
	gcc -Wall -g -c src/server/execucao.c -o obj/execucao.o

obj/ListaLigadaExec.o: src/server/ListaLigadaExec.c
	gcc -Wall -g -c src/server/ListaLigadaExec.c -o obj/ListaLigadaExec.o

bin/tracer: obj/tracer.o obj/ProgExec.o
	gcc -g obj/tracer.o obj/ProgExec.o -o bin/tracer

obj/tracer.o: src/client/tracer.c
	gcc -Wall -g -c src/client/tracer.c -o obj/tracer.o

obj/ProgExec.o: src/client/ProgExec.c
	gcc -Wall -g -c src/client/ProgExec.c -o obj/ProgExec.o

clean:
	rm -f obj/* tmp/* bin/*