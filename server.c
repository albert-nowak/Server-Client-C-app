#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#define pipe "myfifo"		// server main FIFO pipe

struct Request {
    int     pipe_name_len;
    char    pipe_name[20];
    int     request_len;
    char    request[512];
};

struct Response{
    char answer[512];
};

int main(){

	// Potrzebne zmienne
	int s_fd, c_fd;				// file descriptors
	pid_t pid;				// fork
	
	char* argv[100];		// tablica na argumenty programu
	struct Request request;
	struct Response response;


	// Zmienne do wyswietlania czasu
	struct tm *gtime;
    time_t now;
    time(&now);  			// odczytanie obecnego czasu
    gtime = gmtime(&now);	// konwersja na czas UTC

    system("clear");
	printf("[%02d:%02d:%02d] Server starting\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);

	printf("[%02d:%02d:%02d] Creating FIFO\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
	if(mkfifo(pipe, S_IRUSR | S_IWUSR) == -1){
		perror("Error while creating FIFO pipe");
		exit(1);
	}

	while(1){

		// Otwieram łącze do odczytu
		printf("[%02d:%02d:%02d] Opening FIFO to read\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
		if( (s_fd = open(pipe, O_RDONLY)) == -1 ){
			perror("Error while opening pipe");
			exit(1);
		}

		// Czytam z fifo servera, na które pisze klient
		printf("[%02d:%02d:%02d] Reading from server FIFO\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
		if(read(s_fd, &request, sizeof(struct Request)) == -1){
             perror("Error while reading request!");
             exit(1);
        }

        // Debug
  		printf("Pipe name len: %d\n",request.pipe_name_len);
  		printf("Pipe name: %s\n",request.pipe_name);
 		printf("request len: %d\n",request.request_len);
		printf("request: %s\n",request.request);

        // rozdzielenie polecenia od parametrów
        char *p;
        int k = 0;
        p = strtok(request.request, " ");     //pierwszy token
        while(p != NULL){
            argv[k] = p;
            p = strtok(NULL, " ,");
            k++;
        }
        // ustawiam ostatnią pozycję listy argumentow NULLem dla execv  | slajd 52
        argv[k] = NULL;

        if(strcmp(argv[0],"exit") == 0){
        	break;
        }   

		// ------------------- //
		// Realizacja requesta //
		// ------------------- //

		if( (pid = fork()) == -1 ){
			perror("Error while creating fork");
			exit(1);
		}

		if(pid == 0){
			// Otwieram prywatną kolejkę klienta
	        if( (c_fd = open(request.pipe_name, O_WRONLY)) == -1 ){
				perror("Error while opening client's FIFO for write");
				exit(1);
			}
			dup2(c_fd, 1);
			dup2(c_fd, 2);
		    if((execvp(argv[0], argv)) == -1){
		    	perror("Error while executing program");
		    	exit(1);
		    }

		} else {
		    wait(NULL);
		}

       
	}
	close(c_fd);
    close(s_fd);
	unlink(pipe);

	return 0;
}


