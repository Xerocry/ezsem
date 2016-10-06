#include <stdio.h> 

extern int foo();

int main() 
{ 
	printf ("Hello, everybody ! Number from extern function: %d", foo());
	return 0; 
}
