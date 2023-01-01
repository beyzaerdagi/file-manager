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

int getIndex(char *data)
{
    int index = 0;
    for (int i = 0; data[i] != '\0'; i++)
    {
        index++;
    }
    return index;
}

main()
{
    int fd;
	int isExit = 1;
	char *myfifo = "/tmp/myfifo";

	mkfifo(myfifo, 0666);
    char response2[128];
    char init[128] = "init";
    fd = open(myfifo, O_WRONLY);
    write(fd, init, sizeof(init));
    close(fd);
    while (isExit){

        char data[128];
        char response[128];
        fgets(data, 128, stdin);
		if (data[getIndex(data) - 1] == '\n'){
            data[getIndex(data) - 1] = '\0';
        }
        fd = open(myfifo, O_WRONLY);
        write(fd, data, sizeof(data));
		close(fd);
        if (strcmp(data, "exit") == 0){
			isExit = 0;
		}
		fd = open(myfifo, O_RDONLY);
		read(fd, response, 128);
		printf("%s\n", response);
	}
	return 0;
}