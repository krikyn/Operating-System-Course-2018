#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char const *argv[])
{
	if (argc != 2){
		printf("Wrong arguments, usage: /.../name [integer]{value that will be added to 5, which will be squared}\n");
		exit(0);
	}

	unsigned char program_code[] = {
		0x55,                    //      push   rbp
		0x48, 0x89, 0xe5,        //      mov    rbp,rsp
		0x89, 0x7d, 0xfc,        //      mov    DWORD PTR [rbp-0x4],edi
		0x83, 0x45, 0xfc, 0x02,  //  <---it's what we will cahnge     //mov    DWORD PTR [rbp-0x4],0x1
		0x8b, 0x45, 0xfc,        //      mov    eax,DWORD PTR [rbp-0x4]
		0x0f, 0xaf, 0x45, 0xfc,  //      imul   eax,DWORD PTR [rbp-0x4]
		0x5d,                    //      pop    rbp
		0xc3,                    //      ret  
	};

	void *allocated_memory = mmap(NULL, sizeof(program_code), PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if (allocated_memory == MAP_FAILED){
		perror("Can not allocate memory for program");
	}
	
	int add_number = atoi(argv[1]);
	program_code[10] = add_number;
	memcpy(allocated_memory, program_code, sizeof(program_code));

	if (mprotect(allocated_memory, sizeof(program_code), PROT_WRITE | PROT_EXEC) == -1){
		perror("Error with change protect parametrs on program memmory");
	}

	int (*f)() = allocated_memory;

	printf("Return of the program: %d = [(5 + %i)^2]\n", f(5), add_number);

	if (munmap(allocated_memory, sizeof(program_code)) == -1){
		perror("Can not free allocated memmory");
	}

	return 0;
}