#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	int result = 0;

	// Считываем аргументы, пока они не кончатся
	/* 
		Настраиваем функцию getopt следующим образом:
		a - опция без аргументов
		b: - опция обязательно требует аргумент
		C:: - опция необязательно требует аргумент
		d - опция без аргументов
	*/
	while ((result = getopt(argc, argv, "ab:C::d")) != -1) {
		switch (result) {
			case 'a': 
				printf("found argument \"a\".\n"); 
				break;
			case 'b': 
				printf("found argument \"b = %s\".\n",optarg); 
				break;
			case 'C': 
				printf("found argument \"C = %s\".\n",optarg); 
				break;
			case 'd': 
				printf("found argument \"d\"\n"); 
				break;
			case '?': 
				printf("Error found !\n");
				break;
		};
	};

	return 0x0;
}