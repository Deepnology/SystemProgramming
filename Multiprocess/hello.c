#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
	char buff[32] = "";
	sprintf(buff, "%d_%d: Hello World!\n", getpid(), getppid());
	printf("%s", buff);
	//use system calls
	write(STDOUT_FILENO, buff, strlen(buff));

	return 0;
}
