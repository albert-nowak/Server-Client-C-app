#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#define pipe "private_fifo"

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

    // Zmienne
    int s_fd;
    int c_fd;
    char buff[512];
    struct Request request;
    struct Response response;

    // Zmienne do wyswietlania czasu
    struct tm *gtime;
    time_t now;
    time(&now);             // odczytanie obecnego czasu
    gtime = gmtime(&now);   // konwersja na czas UTC

    // Początek pracy programu
    system("clear");            // czyść ekran
    printf("[%02d:%02d:%02d] Client starting\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);

    // Tworzenie prywatnej kolejki FIFO
    printf("[%02d:%02d:%02d] Creating private FIFO\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
    if(mkfifo(pipe, S_IRUSR | S_IWUSR) == -1){
        perror("Error while creating private FIFO pipe");
        exit(1);
    }

    while(1){

        // Pobranie instrukcji od użytkownika
        printf("Type request to the server:\n");
        printf("?>");
        if(!fgets(buff, 512, stdin)){
            perror("Error while getting user input");
            exit(1);
        }

        // usuniecie znaku konca linii z bufforu
        size_t length = strlen(buff);
        if (buff[length - 1] == '\n')
            buff[length - 1] = '\0';

        // przygotowanie wiadomości <int><string><int><string> w struct
        printf("[%02d:%02d:%02d] Preparing struct request\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
        request.pipe_name_len = strlen(pipe);
        strcpy(request.pipe_name, pipe);
        request.request_len = length;
        strcpy(request.request, buff);

        // Debug
        // printf("Pipe name len: %d\n",request.pipe_name_len);
        // printf("Pipe name: %s\n",request.pipe_name);
        // printf("request len: %d\n",request.request_len);
        // printf("request: %s\n",request.request);
        // char* tmp = request;
        // puts("%s",tmp);

        // otweram kolejkę servera
        printf("[%02d:%02d:%02d] Opening server FIFO\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
        if((s_fd = open("myfifo", O_WRONLY)) == -1){
            perror("Cannot open FIFO");
            exit(1);
        }

        // przekazanie structa na fifo servera
        printf("[%02d:%02d:%02d] Writing to server FIFO\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
        if(write(s_fd, &request, sizeof(struct Request)) == -1){
            perror("Can't write to server");
            exit(1);
        }

        // sprawdznie czy użytkownik nie podał exit
        if(strcmp(buff, "exit") == 0){
            printf("[%02d:%02d:%02d] Stopping client\n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
            break;
        }

        // otwieranie prywatnej kolejki fifo
        printf("[%02d:%02d:%02d] Opening \n", (gtime->tm_hour)%24, gtime->tm_min, gtime->tm_sec);
        if((c_fd = open(pipe, O_RDONLY)) == -1){
            perror("Cannot open private FIFO");
            exit(1);
        }

        // czytam z prywatnej kolejki
        int n;

        while((n = read(c_fd, &response, sizeof(struct Response))) > 0){
            write(1, response.answer, n);
        }

        if(n == -1){
            perror("Cannot read response from server");
            exit(1);
        }
        // wyswietl wyniki przetwarzania przez server
        printf("Done!\n");

    }
    unlink(pipe);
    return 0;
}
