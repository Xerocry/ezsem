#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
	// Перезаписываем параметр стреды, содержащий домашний каталог пользователя
	// Третий аргумент отличен от нуля, параметр успешно перезапишется
	setenv("HOME", "/something", 1);
	printf("Home directory: \"%s\"\n", getenv("HOME"));

	// Перезаписываем параметр стреды, содержащий имя пользователя
	// Третий аргумент равен нулю, параметр не перезапишется
	setenv("HOME", "username", 0);
	printf("User name: \"%s\"\n", getenv("USER"));

	// Создаем новый параметр стреды, содержащий имя пользователя
	setenv("MY_SUPER_PARAMETR", "SUPER", 0);

	// Выведем последний параметр среды, он должен быть равен только что созданному новому параметру
	int index = 0;
	while (__environ[index] != NULL)
		++index;

	--index;
	
	printf("New parametr from __environ: \"%s\"\n", __environ[index]);
	printf("New parametr from getenv: \"%s\"\n", getenv("MY_SUPER_PARAMETR"));

	return 0x0;
};