#include <explode.h>


char** explode(char* needle, char* hackstack){
	int i;
	char** params = (char**)malloc(BUFFER_SIZE*sizeof(char*));
	for (i=0; i<BUFFER_SIZE; i++)
		params[i] = NULL;
	char* element = strtok(hackstack, needle);

	i = 0;
	while ((i < BUFFER_SIZE) && (element != NULL)){
		params[i] = (char*)malloc(sizeof(char)*(strlen(element)+1));
		strcpy(params[i], element);
		element = strtok(NULL, needle);
		i++;
	}
	params[i] = NULL;
	if (i>=BUFFER_SIZE){
		perror("Buffer overflow: Max args reached");
		return NULL;
	}

	return params;
}