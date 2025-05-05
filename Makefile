all:
	g++ -Wall -O2 -o server.out server.cpp
	g++ -Wall -O2 -o client.out client.cpp
clean:
	rm server.cpp client.cpp