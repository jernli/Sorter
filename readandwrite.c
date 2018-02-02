#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

ssize_t writen(int fd, const void *wptr, size_t wnbyte){

	size_t wnleft;
	ssize_t wrote;
	const char *ptr;
	ptr = wptr;
	wnleft = wnbyte;

	while (wnleft > 0){
		if ((wrote = write(fd, ptr, wnleft)) <= 0){
			if (errno == EINTR){
				wrote = 0;
			} else
				return (-1);
		}

		wnleft -= wrote;
		ptr += wrote;
	}

	return wnbyte;
}

void Writen(int fd, void *Wptr, size_t nbytes){

	if (writen(fd, Wptr, nbytes) != nbytes){
		perror("writen error");
	}
}

ssize_t readn(int fd, void *rptr, size_t rnbyte){

	size_t rnleft;
	ssize_t rnread;
	char *ptr;
	ptr = rptr;
	rnleft = rnbyte;

	while (rnleft > 0){
		if ((rnread = read(fd, ptr, rnleft)) < 0){
			if (errno == EINTR){
				rnread = 0;
			} else 
				return -1;
		} else if (rnread == 0)
				break;

		rnleft -= rnread;
		ptr += rnread;
	}

	return (rnbyte - rnleft);
}

ssize_t Readn(int fd, void *Rptr, size_t nbytes){

	ssize_t n;

	if ((n = readn(fd, Rptr, nbytes)) < 0){
		perror("readn error");
	}
		return n;
}
