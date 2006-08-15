#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int pid = fork();
    if (pid == 0) {
        printf("parent\n");
    } else {
        printf("child\n");
    }

    return 0;
}
