#include <stdio.h>
#include <dlfcn.h>

char* function_static();
char* function_dynamic1();

int main() {
	printf("%s",function_static());
	printf("%s",function_dynamic1());

	void *dynlib;
	char * (*func)();

	dynlib = dlopen("libdynamic2.so",RTLD_LAZY);
	if (!dynlib){
		fprintf(stderr,"Dlopen error: %s\n", dlerror());
		return 0;
	};

	func = dlsym(dynlib, "function_dynamic2");	

	printf("%s",(*func)());

	dlclose(dynlib);

	return 0;
}
