 #include <stdio.h> 
main ( argc, argv ) 
	int argc; 
	char *argv []; 
{ 
	if (argc > 1) { 
	   if (fopen(argv[1],"r") == NULL)
		printf("%s: неудача при попытке открыть файл %s \n",argv[0],argv[1]); 
	  else 
		printf("%s: файл %s открыт \n",argv[0],argv[1]); 
	} 
	else 
	     printf("%s: открываемый файл не указан \n", argv[0]); 
}
