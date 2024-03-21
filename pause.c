#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    int msg = 1;
    if (argc > 1) {
        msg = (int) strtol(argv[1], NULL, 10);
    };

    printf("message: %d\n", msg);

    const char *status_path = "/tmp/dora.fifo";
    int fd = open(status_path, O_WRONLY);

    int returned = write(fd, &msg, 1);
    printf("%d\n", returned);
    close(fd);
    return 0;
};
