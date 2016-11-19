#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
	// Порождаем процесс-потомок
	if(fork()) {
		int status = 0;

		// Ждем завершение потомка, перезаписывая переменную статуса
		const int result = wait(&status);

		printf("Parent started, pid %d.\n", getpid());
		printf("WAIT: %d\n", result);
		printf("STATUS: %d\n", status);
		
		// Используем различные макросы для получения большей информации о статусе
		printf("WIFEXITED: %d\n", WIFEXITED(status));
		printf("WEXITSTATUS: 0x%X\n", WEXITSTATUS(status));
		printf("WIFSIGNALED: %d\n", WIFSIGNALED(status));
		printf("WTERMSIG: %d\n", WTERMSIG(status));
		printf("WIFSTOPPED: %d\n", WIFSTOPPED(status));
		printf("WSTOPSIG: 0x%X\n", WSTOPSIG(status));
	}
	else {
		printf("Child finished, pid %d, ppid %d.\n\n", getpid(), getppid());

		// Завершаем процесс-потомок шестнадцатиричным кодом 0xA
		return 0xA;
	}

	return 0x0;
}
