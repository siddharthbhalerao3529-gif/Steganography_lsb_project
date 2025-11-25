a.out : test_encode.o encode.o decode.o
	gcc -o a.out test_encode.o encode.o decode.o
test_encode.o : test_encode.c
	gcc -c test_encode.c
encode.o : encode.c
	gcc -c encode.c
decode.o : decode.c
	gcc -c decode.c
clean :
	rm *.o *.out
