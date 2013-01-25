#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void
usage(char *progname)
{
	printf("usage: %s filename [address]\n", progname);
}

void
prepare_filebuf(char **fb, long size)
{
	*fb = (char*) malloc(size);
}

int
main(int argc, char *argv[])
{
	int fd; 
	struct stat statbuf;
	char *filebuf;

	if (argc <= 1) {
		usage(argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_RDONLY)) == -1)  {
		perror("Error openinig file");
		exit(2);
	}

	fstat(fd, &statbuf);

	if (argc == 3)
		filebuf = (char*) strtol(argv[2], NULL, 16);
	else 
		prepare_filebuf(&filebuf, statbuf.st_size);

	printf("file is %ld bytes long, will load at %p\n", (long) statbuf.st_size, filebuf);

	if (read(fd, filebuf, statbuf.st_size) == -1) {
		perror("Error reading file");
		exit(3);
	}
}

