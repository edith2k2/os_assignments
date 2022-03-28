all: demo1 demo2
	./demo1
	./demo2
demo1: demo1.c memlab.c
	gcc -o demo1 demo1.c memlab.c
demo2: demo2.c memlab.c
	gcc -o demo2 demo2.c memlab.c
clean:
	rm demo1
