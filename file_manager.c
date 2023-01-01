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
char c;
int file_length = sizeof(file_list) / sizeof(file_list[0]);
int count = 0;
pthread_t threads[4];
pthread_mutex_t lock;
pthread_cond_t cond;

struct params
{
    char *arg1;
    char *arg2;
};

void *create(char *args);
void *delete(char *args);
void *myRead(char *args);
void *writeFile(char *args);
char **matrixGenerate(int row, int column);
char **arraySplit(char *array);

int main(){

    char **myArray;
    void *status;
    int fd;
    int returnResponse = 0;
    char *myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];
    memset(file_list, '\0', sizeof(file_list));

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    while (1)
    {
        fd = open(myfifo, O_RDONLY);
        read(fd, buf, MAX_BUF);

        myArray = arraySplit(buf);

        struct params params;

        params.arg1 = myArray[1];
        params.arg2 = myArray[2];

        if (strcmp(myArray[0], "init") == 0){
            count++;
            printf("%s\n", myArray[0]);
            printf("%d\n", count);
        }
        else if (strcmp(myArray[0], "create") == 0)
        {
            pthread_create(&threads[0], NULL, create, &params);
            returnResponse = 1;
        }
        else if (strcmp(myArray[0], "delete") == 0)
        {
            pthread_create(&threads[1], NULL, delete, &params);
            returnResponse = 1;
        }
        else if (strcmp(myArray[0], "write") == 0)
        {
            pthread_create(&threads[2], NULL, writeFile, &params);
            returnResponse = 1;
        }
        else if (strcmp(myArray[0], "read") == 0)
        {
            pthread_create(&threads[3], NULL, myRead, &params);
            returnResponse = 1;
        }
        else if (strcmp(myArray[0], "exit") == 0)
        {
            printf("komut algilandi exit\n");
            strcpy(response, "Program has finished");
            count--;
            if(count==0){
                fd = open(myfifo, O_WRONLY);
                write(fd, response, sizeof(response));
                close(fd);
                exit(0);
            } 
            returnResponse = 1;
            printf("%d\n", count);
        }

        for (int i = 0; i < 4; i++)
        {
            pthread_join(threads[i], &status);
        }

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

void *create(char *args)
{
    pthread_mutex_lock(&lock);
    struct params *params = args;
    char *file_name = params->arg1;
    printf("filename : %s\n", file_name);

    int idx = -1;
    for (int i = 0; i < file_length; i++)
    {
        if (file_list[i] != NULL)
        {
            if (strcmp(file_list[i], file_name) == 0)
            {
                idx = i;
            }
        }
    }
    if (idx == -1)
    {
        for (int i = 0; i < 10; i++)
        {
            if (file_list[i][0] == '\0')
            {
                strcpy(file_list[i], file_name);
                FILE *file = fopen(file_name, "w");
                fclose(file);
                strcpy(response, "File Created");

                break;
            }
        }
    }
    else
    {
        strcpy(response, "File zaten var.");
    }
    pthread_mutex_unlock(&lock);
}

void *delete(char *args)
{

    pthread_mutex_lock(&lock);

    struct params *params = args;
    char *file_name = params->arg1;
    printf("filename : %s", file_name);

    int idx = -1;

    for (int i = 0; i < file_length; i++)
    {
        if (file_list[i] != NULL)
        {
            if (strcmp(file_list[i], file_name) == 0)
            {
                idx = i;

                break;
            }
        }
    }

    if (idx != -1)
    {

        file_list[idx][0] = '\0';
        remove(file_name);
        strcpy(response, "File Deleted!");
    }
    else
    {
        strcpy(response, "Silinecek dosya bulunmadı.");
    }
    pthread_mutex_unlock(&lock);
}

void *myRead(char *args)
{
    pthread_mutex_lock(&lock);

    struct params *params = args;
    char *file_name = params->arg1;
    printf("filename : %s", file_name);
    int idx = -1;
    for (int i = 0; i < file_length; i++)
    {
        if (file_list[i] != NULL)
        {
            if (strcmp(file_list[i], file_name) == 0)
            {
                idx = i;
                break;
            }
        }
    }

    if (idx != -1)
    {
        FILE *fptr = fopen(file_name, "r");
        while ((c = fgetc(fptr)) != EOF)
        {
            printf("%c", c);
        }
        fclose(fptr);

        strcpy(response, "File Readed!");
    }
    else
    {
        strcpy(response, "Okunacak Dosya Bulunamadı");
    }
    pthread_mutex_unlock(&lock);
}

void *writeFile(char *args)
{
    pthread_mutex_lock(&lock);
    struct params *params = args;
    char *file_name = params->arg1;
    char *data = params->arg2;
    printf("filename : %s", file_name);
    printf("data : %s", data);

    int idx = -1;

    for (int i = 0; i < 10; i++)
    {

        if (file_list[i] != NULL)
        {
            if (strcmp(file_list[i], file_name) == 0)
            {
                idx = i;
                break;
            }
        }
    }
    if (idx != -1)
    {

        FILE *file = fopen(file_name, "a+");
        if (file == NULL)
        {
            perror("fopen failed");
            // return;
        }

        fprintf(file, "%s\n", data);

        if (fclose(file) == EOF)
        {
            perror("fclose failed");
            // return;
        }
        strcpy(response, "File Writed!");
    }
    else
    {
        strcpy(response, "Yazılacak Dosya Bulunamadı");
    }
    pthread_mutex_unlock(&lock);
}

char **matrixGenerate(int row, int column)
{
    int i;
    char **matrix = malloc(row * sizeof(int *));
    for (i = 0; i < row; i++)
    {
        matrix[i] = malloc(column * sizeof(int));
    }

    return matrix;
}

char **arraySplit(char *array)
{

    int i = 0;
    char *p = strtok(array, " ");
    char **arr;
    arr = matrixGenerate(10, 10);
    while (p != NULL)
    {
        *(arr + i) = p;
        i++;
        p = strtok(NULL, " ");
    }

    return arr;
}