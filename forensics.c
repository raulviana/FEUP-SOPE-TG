#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#define READ 0
#define WRITE 1
#define MAXLINE 512


//Chama o processo externo "file" para obter o tipo de ficheiro
char getFileInfo(char* filename, char filetype[]){

int fd1[2], fd2[2], n;
pid_t pid;
pipe(fd1); pipe(fd2);

pid = fork();
if(pid > 0){
  //parent
  close(fd1[0]); close(fd2[1]);
  write(fd1[1], filename, sizeof(filename));
  n = read(fd2[0], filetype, MAXLINE);
  filetype[n] = 0;
	int i = 0;
	while(filetype[i]!='\0') {
      if(filetype[i]==':') {
        filetype[i]=',';
        }
        i++;
  }
  return *filetype;
}
else{
  char in[MAXLINE];
	int n2;
  close(fd1[1]); close(fd2[0]);
  dup2(fd1[0], STDIN_FILENO);
  dup2(fd2[1], STDOUT_FILENO);
  n2 = read(fd1[0], in, sizeof(filename));
  in[n2] = 0;
  close (fd1[0]);
  execl("/usr/bin/file", "file", in, NULL);
  close(fd2[1]);
  exit(0);
}
}

//Imprime caracteristicas do ficheiro
//_____________________________________

void printFileInfo(char* filename) {
	struct stat file_info;
	char time_buffer[80];
	char filetype[80];
	getFileInfo(filename, filetype);
	strtok(filetype, "\n");
	

//	printf("%s", filename);
	if (stat(filename, &file_info) < 0) {
		printf("Unable to get file info"); //nao será print. mas terminr com um codigo de erro???
		exit(1);
	}
	printf("%s", filetype);
	printf(", "); 
	printf("%ld,", file_info.st_size);

	struct tm *accessed_stamp = localtime(&file_info.st_mtime);
	strftime(time_buffer, 80, "%FT%T", accessed_stamp);
	printf("%s, ", time_buffer);

	struct tm *created_stamp = localtime(&file_info.st_atime);
	strftime(time_buffer, 80, "%FT%T", created_stamp);
	printf("%s, ", time_buffer);

	struct tm *modified_stamp = localtime(&file_info.st_ctime);
	strftime(time_buffer, 80, "%FT%T", modified_stamp);
	printf("%s\n", time_buffer);
}
//-----------------------------------------------------------



void recurs_traverse(char* list[]) {

}



int main(int argc, char* argv[], char* envp[]) {
char* arguments[argc-1];

//SAVING ARGUMENTS 
	int i;

        printf( "argc:     %d\n", argc );
        printf( "argv[0]:  %s\n", argv[0] );

        if ( argc == 1 ) {
                printf( "No arguments were passed.\n" );
        } else {
                for ( i = 1; i < argc; i++) {
					arguments[i-1] = argv[i];
                }
		}

		printf("Resultant array is\n"); //confirms if arguments array is correct
		for (i = 0; i < argc-1; i++)
			printf("%s\n", arguments[i]);
/*
 //ENVIROMENT VARIABLES
 char** env;
 char* enviroment[0]; // ??size of array 
 int c = 0;
  //	variables to use in pipes
    for(env=envp;*env!=0;env++)
    {
        char* thisEnv = *env;
		enviroment[c]= thisEnv;
       printf("%s\n",thisEnv);
		c++;
    }
	printf("Resultant array is\n"); //confirms if arguments array is correct
		for (i = 0; i < c; i++)
			printf("%s\n", enviroment[i]);
 
*/
//pid_t pid, pidSon;
//int status;
//char folderContent[80];

//Dummy variables

	//struct stat stat_buf;*/

	/*
	O ciclo while abaixo tem como função abrir diretórios recursivamente.
	Por algum motivo o programa é incapaz de distinguir um diretório de um ficheiro com a função S_ISDIR(stat_buf.st_mode)
	*/
	/*while (Dirent != NULL) {
		printf("\nWhile ");
		name = Dirent->d_name;
		if (name[0] != '.') {
			stat(name, &stat_buf);
			if (stat(name, &stat_buf) == -1) {
				printf("\nError in stat --- %s", name);
				exit(1);
			}
			if (S_ISDIR(stat_buf.st_mode)) { //Testa se é uma pasta se for replica-se, se não for, deixa correr 
											 // folderContent[i] é uma pasta;
				pid = fork();
				printf("fork");
				if (pid > 0) {
					//este é o pai
					printf("\nParent %s", name);
					//pidSon = wait(&status);
					if (pidSon != 0) {
						printf("Error parsing file system");
						exit(1);
					}
				}
				else {
					//este é o filho
					printf("\nChild %s", name);
					D = opendir(name);
					Dirent = readdir(D);
					name = Dirent->d_name;

				}
			}
			else printf("%s", name);
		}
		else printf(" . ");
		printf("\nPID = %d", pid);
		Dirent = readdir(D);
	}

	printf("\nEnd of While ");
*/



	/*
	-------------------------------------------------
	Process the file and produce the required output
	------------------------------------------------
	*/
	
	char outFilePath[80] = "./outFile.csv";
	int fd_out;

	if (1) { //argument "-o" present
	fd_out = open(outFilePath, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd_out == -1) {
	printf("Couldn't open file to output\n");
	return 1;
	}
	dup2(fd_out, STDOUT_FILENO);
	printFileInfo(argv[1]);
	fflush(stdout);
	close(fd_out);
	dup2(1, STDOUT_FILENO);
	}
	else {
	printFileInfo(argv[1]);
	}
	
	exit(0);
}

