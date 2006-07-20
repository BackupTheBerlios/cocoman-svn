#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int s= socket(PF_UNIX, SOCK_STREAM, 0);
    return 0;
}
