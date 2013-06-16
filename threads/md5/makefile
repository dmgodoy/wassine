all: md5

md5: md5main.o md5.o
	gcc $^ -o md5

clean:
	rm -f *.o md5
