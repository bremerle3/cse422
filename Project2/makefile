binaries=Deadlock Room Example1 Example2 Example3 Example4 Example5 Wiki modWiki semWiki

Deadlock:
	g++ -g diningDeadlock.cpp -o Deadlock.o

Room:
	g++ -g diningRoom.cpp -o Room.o

Example1: 
	g++ -g -pthread pthreadExample1.cpp -o Example1.o

Example2: 
	g++ -g -pthread pthreadExample2.cpp -o Example2.o

Example3: 
	g++ -g -pthread pthreadExample3.cpp -o Example3.o

Example4: 
	g++ -g -pthread pthreadExample4.cpp -o Example4.o

Example5: 
	g++ -g -pthread pthreadExample5.cpp -o Example5.o

Wiki: 
	g++ -g -pthread Wiki.cpp -o Wiki.o

modWiki: 
	g++ -g -pthread modWiki.cpp -o modWiki.o

semWiki: 
	g++ -g -pthread semWiki.cpp -o semWiki.o

.PHONY: clean

clean:
	rm -f $(binaries) *.o
