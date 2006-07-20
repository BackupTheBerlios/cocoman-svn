#include <stdio.h>
int main(int argc, char **argv) {
    FILE* fp = fopen("somefile", "w");
/*     char message[] = "testing\n"; */
/*     write(fp, message, 0); */
    fclose(fp);
}
