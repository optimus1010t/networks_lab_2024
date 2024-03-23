libmsocket.a: msocket.o
	ar rcs libmsocket.a msocket.o

msocket.o: msocket.c msocket.h
	gcc -c msocket.c

initmsocket: initmsocket.c libmsocket.a
	gcc -o initmsocket initmsocket.c -L. -lmsocket

user1: user1.c libmsocket.a
	gcc -o user1 user1.c -L. -lmsocket

user2: user2.c libmsocket.a
	gcc -o user2 user2.c -L. -lmsocket

clean:
	rm -f *.o *.a initmsocket user1 user2 a.out

# libmsocket.a: msocket.o
# 	ar rcs libmsocket.a msocket.o

# msocket.o: msocket.c msocket.h
# 	gcc -c  -DDEBUG msocket.c

# initmsocket: initmsocket.c libmsocket.a
# 	clear
# 	gcc  -DDEBUG -o initmsocket initmsocket.c -L. -lmsocket

# user1: user1.c libmsocket.a
# 	clear
# 	gcc  -DDEBUG -o user1 user1.c -L. -lmsocket

# user2: user2.c libmsocket.a
# 	clear
# 	gcc  -DDEBUG -o user2 user2.c -L. -lmsocket

# clean:
# 	rm -f *.o *.a initmsocket user1 user2 a.out

# libmsocket.a: msocket.o
# 	ar rcs libmsocket.a msocket.o

# msocket.o: msocket.c msocket.h
# 	gcc -c -g -fsanitize=address -DDEBUG msocket.c

# initmsocket: initmsocket.c libmsocket.a
# 	clear
# 	gcc -g -fsanitize=address -DDEBUG -o initmsocket initmsocket.c -L. -lmsocket

# user1: user1.c libmsocket.a
# 	clear
# 	gcc -g -fsanitize=address -DDEBUG -o user1 user1.c -L. -lmsocket

# user2: user2.c libmsocket.a
# 	clear
# 	gcc -g -fsanitize=address -DDEBUG -o user2 user2.c -L. -lmsocket

# clean:
# 	rm -f *.o *.a initmsocket user1 user2 a.out

