#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

#define MAX_BUF 128

char file_list[10][100];
char response[128];
int clientCount = 0;
pthread_t threads[4];
pthread_mutex_t lock;
pthread_cond_t cond;

//keep client data
struct params{
    char *arg1;
    char *arg2;
};

void *createFile(char *args);
void *deleteFile(char *args);
void *readFile(char *args);
void *writeFile(char *args);
void str_sep(char *str, char **seperated);

int main(){

    char *args[20];
    void *status;
    int fd;
    int returnResponse = 0;
    char *myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    while (1){
        //open pipe for read
        fd = open(myfifo, O_RDONLY);
        read(fd, buf, MAX_BUF);
        //seperate input data
        str_sep(buf, args);
        //find args length
        int len = 0;
        for (int i = 0; args[i] != '\0'; i++){
            len++;
        }

        struct params params;
        params.arg1 = args[1];
        params.arg2 = args[2];

        //if client terminal is open increase client counter
        if (strcmp(args[0], "client_created") == 0){
            
            clientCount++;
            printf("%s\n", args[0]);

        }else if (strcmp(args[0], "create") == 0 && len == 2){
            //create thread and assigned to the thread list
            pthread_create(&threads[0], NULL, createFile, &params);
            returnResponse = 1;

        }else if (strcmp(args[0], "delete") == 0 && len == 2){
            
            pthread_create(&threads[1], NULL, deleteFile, &params);
            returnResponse = 1;

        }else if (strcmp(args[0], "write") == 0 && len == 3){
            
            pthread_create(&threads[2], NULL, writeFile, &params);
            returnResponse = 1;

        }else if (strcmp(args[0], "read") == 0 && len == 2){
            
            pthread_create(&threads[3], NULL, readFile, &params);
            returnResponse = 1;

        }else if (strcmp(args[0], "exit") == 0 && len == 1){
            
            strcpy(response, "Program Has Finished...");
            //if client terminal is close then decrease client counter
            clientCount--;
            //if client counter is 0 then close manager terminal and return response
            if(clientCount == 0){
                fd = open(myfifo, O_WRONLY);
                write(fd, response, sizeof(response));
                close(fd);
                exit(0);
            } 
            returnResponse = 1;

        }else{
            //if user enter wrong command print message
            strcpy(response, "The Wrong Command Was Entered");
            printf("Commands:\n");
            printf("    create: create ex.txt\n");
            printf("    write: write ex.txt hey\n");
            printf("    read: read ex.txt\n");
            printf("    delete: delete ex.txt\n");
            printf("    exit: exit\n ");
            returnResponse = 1;
        }

        //join all threads
        for (int i = 0; i < 4; i++)
        {
            pthread_join(threads[i], &status);
        }

        //if enter command send response to client
        if(returnResponse == 1){
            fd = open(myfifo, O_WRONLY);
            write(fd, response, sizeof(response));
            close(fd);
        }
    }
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    exit(0);
}

void *createFile(char *args)
{
    pthread_mutex_lock(&lock);
    struct params *params = args;
    //the argument in params is assigned to the file name
    char *file_name = params->arg1;

    int fileExist = -1;
    for (int i = 0; i < 10; i++){
        if (file_list[i] != NULL){
            //if file name already exist then assign fileExist to i
            if (strcmp(file_list[i], file_name) == 0){
                fileExist = i;
            }
        }
    }

    //file does not exit then add file to file list
    if (fileExist == -1){
        for (int i = 0; i < 10; i++)
        {
            if (file_list[i][0] == '\0'){
                strcpy(file_list[i], file_name);
                FILE *file = fopen(file_name, "w");
                fclose(file);
                strcpy(response, "File Created");
                break;
            }
        }
    }else{
        //if file exist then return just response
        strcpy(response, "The File Already Exists");
    }
    pthread_mutex_unlock(&lock);
}

void *deleteFile(char *args){

    pthread_mutex_lock(&lock);
    struct params *params = args;
    //the argument in params is assigned to the file name
    char *file_name = params->arg1;

    int fileExist = -1;
    for (int i = 0; i < 10; i++){
        if (file_list[i] != NULL){
            //if file name found then assign fileExist to i
            if (strcmp(file_list[i], file_name) == 0){
                fileExist = i;
                break;
            }
        }
    }

    if (fileExist != -1){
        file_list[fileExist][0] = '\0';
        //remove file from system
        remove(file_name);
        strcpy(response, "File Deleted");
    }else{
        strcpy(response, "No File To Delete Was Found");
    }
    pthread_mutex_unlock(&lock);
}

void *readFile(char *args){

    pthread_mutex_lock(&lock);
    struct params *params = args;
    //the argument in params is assigned to the file name
    char *file_name = params->arg1;

    int fileExist = -1;
    for (int i = 0; i < 10; i++){
        if (file_list[i] != NULL){
            //if file name found then assign fileExist to i and break 
            if (strcmp(file_list[i], file_name) == 0){
                fileExist = i;
                break;
            }
        }
    }
    if (fileExist != -1)
    {
        //print the inside file to console
        FILE *fptr = fopen(file_name, "r");
        char c;
        while ((c = fgetc(fptr)) != EOF){
            printf("%c", c);
        }
        fclose(fptr);
        strcpy(response, "File Readed");
    }
    else
    {
        strcpy(response, "No File To Read Was Found");
    }
    pthread_mutex_unlock(&lock);
}

void *writeFile(char *args){

    pthread_mutex_lock(&lock);
    struct params *params = args;
    //the argument in params is assigned to the file name
    char *file_name = params->arg1;
    //the argument in params is assigned to the data
    char *data = params->arg2;
    printf("input data : %s\n", data);

    int fileExist = -1;
    for (int i = 0; i < 10; i++){

        if (file_list[i] != NULL){
            //if file name found then assign fileExist to i and break
            if (strcmp(file_list[i], file_name) == 0){
                fileExist = i;
                break;
            }
        }
    }

    if (fileExist != -1){

        FILE *file = fopen(file_name, "a+");
        if (file == NULL){
            perror("fopen failed");
        }
        //write data to file
        fprintf(file, "%s\n", data);

        if (fclose(file) == EOF){
            perror("fclose failed");
        }
        strcpy(response, "Written To The File");
    }else{
        strcpy(response, "No File To Write Was Found");
    }
    pthread_mutex_unlock(&lock);
}

// the input line is seperated
// and its elements is assigned to the array given as a parameter
// the last element is NULL
void str_sep(char *str, char **seperated){

    int i = 0;
    char *token = strtok(str, " ");
    while (token != NULL)
    {
        // printf("token %s ",token);
        seperated[i++] = token;
        token = strtok(NULL, " ");
    }
    seperated[i] = NULL;
}
