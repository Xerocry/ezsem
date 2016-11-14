#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
	// Переменная для последующего использования wait
	int temp;

	printf("Parent process pid #%d.\n", getpid());

	// Выполняем исполняемый файл, в процессе-потомке
	if(!fork())
		execl("child", "child", NULL);

	// Ждем завершение процесса-потомка
	printf("Waiting child process pid #%d.\n", wait(&temp));
	return 0x0;
}
