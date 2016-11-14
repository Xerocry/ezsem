#include <stdio.h>
#include <unistd.h>

int main() {
	printf("Child process pid #%d.\n", getpid());

	// Вызываем список текущих процессов в виде дерева
	system("ps -xf");
	return 0x0;
}
