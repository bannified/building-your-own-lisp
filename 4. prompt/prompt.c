#include <stdio.h> //stdio.h -> standard IO header

static char input[2048];

int main(int argc, char** argv){
	
	puts("Welcome to GLisp");
	puts("Press Ctrl+c to exit");
	
	while (1) {
		fputs("Glispy>", stdout);
		fgets(input, 2048, stdin);
		printf("You have entered %s", input);
	}

	return 0;
}


// run using gcc -std=c99 -Wall hello_world.c -o hello_world
// compiles code in this file, report any wranings, and outputs the program to a new file called hello_world
// -std=c99 is a flag used to tell compiler which version of C we're using, ensures code is standardized
// -o means output, hello_world means the directory/destination
// 