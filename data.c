#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "data.h"

int get_file_size(char *file){

	struct stat buf;

	if ((stat(file, &buf)) == -1){
		perror("stat");
		exit(1);
	}

	return buf.st_size;
}