simpsh:main.c
	gcc -lrt -pthread -o addtest main.c
	gcc -lrt -pthread -o sltest dbmain.c
clean:
	rm sltest
	rm addtest
check:
	./sltest
	./addtest
dist:
	tar -cvzf lab4-shao.tar.gz main.c dbmain.c Makefile answer.txt sltest addtest SortedList.h

