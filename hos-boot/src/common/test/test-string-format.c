#include "string.h"
#include "stdio.h"

int main(int argc, char **argv)
{
	char buf[16];
	int a = formatunsigned(0x1badb002, 16, buf);
	printf("LENGTH %d: %s\n", a, buf);
	return 0;
}
