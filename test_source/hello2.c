
#include <stdio.h>
#include <unistd.h>


int main()
{
  int i;

  int d = 0;
  scanf("%d",&d);
  printf("%d!!\n",d);

  printf("Hello world!\n");
  for (i = 0; i < 5; ++i)
    usleep(1000000);

  return 0;
}
