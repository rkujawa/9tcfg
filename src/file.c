/* Ninetails configuration program */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <exec/types.h>

#include "file.h"

/* get file size */
ULONG
file_size(BYTE *path)
{
	int fd;
	ULONG filesize;
	struct stat statbuf;

	if ((fd = open(path, O_RDONLY)) == -1)  {	
		perror("Error openinig file");
		return 0;
	}

	fstat(fd, &statbuf);
	filesize = statbuf.st_size;

	close(fd);

	return filesize;
}

/* load file to memory buffer */
BOOL
file_load(BYTE *path, BYTE *filebuf, ULONG filesize)
{
	int fd;

	if ((fd = open(path, O_RDONLY)) == -1)  {	
		perror("Error openinig file");
		return 0;
	}

#ifdef DEBUG
	printf("DEBUG: loading %lx bytes long file at %p\n", (ULONG) filesize, (void*) filebuf);
#endif /* DEBUG */

	if (read(fd, filebuf, filesize) == -1) {
		perror("Error reading file");
		return 0;
	}

	close(fd);

	return 1;
}

