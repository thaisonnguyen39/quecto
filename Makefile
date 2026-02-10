main: src/*.c include/*.h
	gcc -I include src/*.c -o main

tests:
