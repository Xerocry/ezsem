#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int index = 0;

	// Выводим все параметры среды
	// Последний параметр имеет значение NULL, для обозначения конца списка
	while (__environ[index] != NULL) {
		printf("%s\n", __environ[index]);
		++index;
	};

	return 0x0;
};