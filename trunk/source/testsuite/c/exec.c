#include <unistd.h>

int main(int argc, char **argv) {
    char *const args[] = {"somefile"};
    execve("touch", args, NULL);
    return 0;
} 
