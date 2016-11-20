#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "singlecore.h"

#define FILE_LENGTH 15
#define STRING_LENGTH 15

int main(int argc, char* argv[]) {
	// Открываем файл на чтение
	int file = open("readfile.txt", O_RDONLY);
	if(file < 0) {
		// В случае неудачи выводим сообщение об ошибке
		perror("It's impossible to open file.");
		return 0x1;	
	}

	// Если не получилось перераспределить ресурсы процессорра на одно ядро, то выходим с ошибкой
	if(disableMultithreading(getpid()) < 0)
		return 0x2;

	if(!fork()) {
		// Child
	
		printf("Child pid %d, ppid %d. Read from file: ", getpid(), getppid());

		// Считываем побайтово файл и выводим его содержимое 
		char symbol;
		int index = 0;
		while(index < FILE_LENGTH) {
			pread(file, &symbol, 1, index++);
			printf("%c", symbol);
		}
		printf("\n");

		// Передаем дескриптор в качестве аргумента
		char buffer[STRING_LENGTH];	
		sprintf(buffer, "%d", file);
		execl("p1.12.ch", "p1.12.ch", buffer, NULL);
	}
	else {
		// Parent
	
		printf("Parent pid %d, ppid %d. Read from file: ", getpid(), getppid());
		
		// Считываем побайтово файл и выводим его содержимое 
		char symbol;
		int index = 0;
		while(index < FILE_LENGTH) {
			pread(file, &symbol, 1, index++);
			printf("%c", symbol);
		}
		printf("\n");

		// Закрываем дескриптор
		close(file);
	}


}
