all:
	make clean
	make oblivious
	make mixed
	make morton
	make morton_opt
	make kernel

oblivious:
	make clean
	gcc -Wall -o oblivious.x oblivious.c -O2
	./oblivious.x 4 1000
	./oblivious.x 8 1000
	./oblivious.x 16 1000
	./oblivious.x 32 1000
	./oblivious.x 64 1000
	./oblivious.x 128 1000
	./oblivious.x 256 1000
	./oblivious.x 512 1000

mixed:
	make clean
	gcc -Wall -o mixed.x mixed.c -O2
	./mixed.x 4 1000
	./mixed.x 8 1000
	./mixed.x 16 1000
	./mixed.x 32 1000
	./mixed.x 64 1000
	./mixed.x 128 1000
	./mixed.x 256 1000
	./mixed.x 512 1000

morton:
	make clean
	gcc -Wall -o morton.x morton.c -O2
	./morton.x 4 1000
	./morton.x 8 1000
	./morton.x 16 1000
	./morton.x 32 1000
	./morton.x 64 1000
	./morton.x 128 1000
	./morton.x 256 1000
	./morton.x 512 1000

morton_opt:
	make clean
	gcc -Wall -o morton_optimized.x morton_optimized.c -O2
	./morton_optimized.x 4 1000
	./morton_optimized.x 8 1000
	./morton_optimized.x 16 1000
	./morton_optimized.x 32 1000
	./morton_optimized.x 64 1000
	./morton_optimized.x 128 1000
	./morton_optimized.x 256 1000
	./morton_optimized.x 512 1000

kernel:
	make clean
	g++ -o kernel.x kernel.c -mavx -mfma -O1
	./kernel.x 1 1000
	./kernel.x 2 1000
	./kernel.x 4 1000
	./kernel.x 8 1000
	./kernel.x 16 1000
	./kernel.x 32 1000
	./kernel.x 64 1000
	./kernel.x 128 1000
	./kernel.x 256 1000
	./kernel.x 512 1000
	./kernel.x 1024 1000
	./kernel.x 2048 1000
	./kernel.x 4096 1000
	./kernel.x 8192 1000
	
clean:
	rm -rf *.x
