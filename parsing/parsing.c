#include <stdio.h> //stdio.h -> standard IO header
#include "mpc.h"

static char input[2048];

int main(int argc, char** argv){
	
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	
	mpca_lang(MPCA_LANG_DEFAULT,
		"									\
			number	:	/-?[0-9]+/;			\
			operator:	'+' | '-' | '*' | '/';	\
			expr	:	<number> | '(' <operator> <expr>+ ')' ;\
			lispy : /^/ <operator><expr>+ /$/; \
		",
		Number, Operator, Expr, Lispy);
	
	puts("Welcome to GLisp");
	puts("Press Ctrl+c to exit");
	
	while (1) {
		fputs("Glispy>", stdout);
		fgets(input, 2048, stdin);
		//printf("You have entered %s", input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)){
			//On success Print the AST//
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			//if not, print error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
			
		}
	}
	
	mpc_cleanup(4,Number,Operator, Expr, Lispy);
	
	return 0;
}


// run using gcc -std=c99 -Wall hello_world.c -o hello_world
// compiles code in this file, report any wranings, and outputs the program to a new file called hello_world
// -std=c99 is a flag used to tell compiler which version of C we're using, ensures code is standardized
// -o means output, hello_world means the directory/destination
// 