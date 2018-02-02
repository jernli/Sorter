#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "data.h"
#include "io.h"

struct pipefd{
	int fdes[2];
};

int get_file_size(char *file);
void child_sort(int start, int end, char *infile, int fd);
int compare_rank(const void *ranrec1, const void *ranrec2);
void merge(struct pipefd *pptr, int n, char *outfile);

/* Takes in 3 arguments:
	-n <number of processes>; number of child to be forked
	-f <input file name>; has to be binary file
	-o <output file name>
*/
int main(int argc, char *argv[]){

	extern char *optarg;
	int arg, nkids, nrecords, start, end, status;
	int i, j, pid;
	char *infile, *outfile;
	struct pipefd *pfds;
	struct timeval stime, etime;
	double timediff;

	if ((gettimeofday(&stime, NULL)) == -1){
		perror("gettimeofday");
		exit(1);
	}

	while((arg = getopt(argc, argv, "n:f:o:")) != -1){
		switch(arg){
			case 'n':
				nkids = atoi(optarg);
				break;
			case 'f':
				infile = optarg;
				break;
			case 'o':
				outfile = optarg;
				break;

			default: fprintf(stderr, "Usage: sortmain -n <number of processes> "
					"-f <file name> -o <output file name\n");
			exit(1);
		}
	}

	pfds = (struct pipefd *)malloc(sizeof(struct pipefd)*nkids);

	nrecords = get_file_size(infile) / sizeof(struct ranrec);

	for (i=0; i<nkids; i++){
		start = i * nrecords / nkids;
		end = (i+1) * nrecords / nkids;
		end = end > nrecords ? nrecords : end;

		if ((pipe(pfds[i].fdes)) == -1){
			perror("pipe");
		}

		pid = fork();
		if (pid == 0){
			for (j=0; j<i; j++){
				close(pfds[j].fdes[0]);
			}
			close(pfds[i].fdes[0]);
			child_sort(start, end, infile, pfds[i].fdes[1]);
		} else if (pid == -1){
			perror("fork");
			exit(1);
		} else {
			close(pfds[i].fdes[1]);
		}
	}

	merge(pfds, nkids, outfile);

	for (i=0; i<nkids; i++){
		if ((wait(&status)) > 0){
			if ((WEXITSTATUS(status)) != 0){
				fprintf(stderr, "Child %d terminated with error\n", i);
			}
		} else{
			perror("wait");
		}
	}

	if ((gettimeofday(&etime, NULL)) == -1){
		perror("gettimeofday");
		exit(1);
	}

	timediff = (etime.tv_sec - stime.tv_sec) +
				(etime.tv_usec - stime.tv_usec) / 1000000.0;
	fprintf(stdout, "%.4f\n", timediff);

	return 0;
}



void child_sort(int start, int end, char *infile, int fd){

	struct ranrec *rec;
	FILE *fp;
	int nread, i;

	rec = (struct ranrec *)malloc(sizeof(struct ranrec) * (end-start));

	if ((fp = fopen(infile, "r")) == NULL){
		fprintf(stderr, "Could not open %s\n", infile);
		exit(1);
	}

	if ((fseek(fp, start*sizeof(struct ranrec), SEEK_SET)) == -1){
		perror("fseek");
		exit(1);
	}

	nread = fread(rec, sizeof(struct ranrec), end-start, fp);
	fclose(fp);

	if (nread != end-start){
		fprintf(stderr, "Read error, %d records read\n", nread);
	}

	qsort(rec, nread, sizeof(struct ranrec), compare_rank);

	for(i=0; i<nread; i++){
		Writen(fd, &rec[i], sizeof(struct ranrec));
	}
	close(fd);
	exit(0);
}

int compare_rank(const void *ranrec1, const void *ranrec2){

	struct ranrec *rr1 = (struct ranrec *)ranrec1;
	struct ranrec *rr2 = (struct ranrec *)ranrec2;

	if (rr1->rank == rr2->rank){
		return 0;
	} else if (rr1->rank > rr2->rank){
		return 1;
	} else{
		return -1;
	}
}

void merge(struct pipefd *pptr, int n, char *outfile){

	int ndone = n;
	int i;
	struct ranrec *in;
	FILE *pof;

	in = (struct ranrec *)malloc(sizeof(struct ranrec)*n);

	if ((pof = fopen(outfile, "w")) == NULL){
		fprintf(stderr, "Could not open %s\n", outfile);
		exit(1);
	}

	for (i=0; i<n; i++){
		Readn(pptr[i].fdes[0], &in[i], sizeof(struct ranrec));
	}

	while (ndone > 0){
		int smallest = 0;
		for (i=0; i<n; i++){
			if ((in[smallest].rank == -1) || 
				(in[i].rank != -1 && in[i].rank < in[smallest].rank)){
				smallest = i;
			}
		}
		fprintf(pof, "%d %s", in[smallest].rank, in[smallest].name);
		if ((Readn(pptr[smallest].fdes[0], &in[smallest], sizeof(struct ranrec))) == 0){
			in[smallest].rank = -1;
			ndone--;
		}
	}
}

