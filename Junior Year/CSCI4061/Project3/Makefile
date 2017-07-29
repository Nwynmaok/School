all: 
	cc -c test.c mini_filesystem.c
	ar -cvq libctest.a test.o mini_filesystem.o
	cc -o test test.c libctest.a
	rm test.o
	rm mini_filesystem.o
	rm libctest.a

clean:
	rm -r output_dir
	rm test
	rm log.txt
	rm summary.txt
	rm filesystem_content.html

	
