#include <stdlib.h>
#include <stdio.h>

int main(int argc,char **argv) {
	// Получение параметра стреды, содержащего домашний каталог пользователя
	printf("Home directory: \"%s\"\n", getenv("HOME"));

	// Получение параметра стреды, содержащего имя пользователя
	printf("User name: \"%s\"\n", getenv("USER"));

	// Получение параметра стреды, содержащего каталог из которого запустили программу
	printf("Program directory: \"%s\"\n", getenv("PWD"));

	return 0x0;
};