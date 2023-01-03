#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

//find input data length
int findLength(char *data){
    int len = 0;
    for (int i = 0; data[i] != '\0'; i++){
        len++;
    }
    return len;
}

main() {
    int fd;
    int isExit = 1;
    char *myfifo = "/tmp/myfifo";

    mkfifo(myfifo, 0666);
    char response2[128];
    //if client terminal is open
    char client[128] = "client_created";
    fd = open(myfifo, O_WRONLY);
    write(fd, client, sizeof(client));
    close(fd);

    while (isExit){
        char data[128];
        char response[128];
        //get input data from client
        fgets(data, 128, stdin);
        //delete space
	if (data[findLength(data) - 1] == '\n'){
            data[findLength(data) - 1] = '\0';
        }
        //send manager input data
        fd = open(myfifo, O_WRONLY);
        write(fd, data, sizeof(data));
	close(fd);
        //if client enter exit, exit terminal
        if (strcmp(data, "exit") == 0){
		isExit = 0;
	}
        //print response from manager
	fd = open(myfifo, O_RDONLY);
	read(fd, response, 128);
	printf("%s\n", response);
   }
	return 0;
}
