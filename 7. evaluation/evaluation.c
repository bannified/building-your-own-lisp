#include <stdio.h> //stdio.h -> standard IO header
#include "mpc.h"
#include <math.h>

static char input[2048];


long eval_op(long x, char* op, long y){
	if (strcmp(op,"+") == 0) {return x + y;}
	if (strcmp(op,"-") == 0) {return x - y;}
	if (strcmp(op, "*") == 0) {return x * y;}
	if (strcmp(op, "/") == 0) {return x / y;}
	if (strcmp(op, "%") == 0) {return x % y;}
	if (strcmp(op, "^") == 0) {return powl(x,y);}
	if (strcmp(op, "n") == 0) {return fminl(x,y);}
	if (strcmp(op, "x") == 0) {return fmaxl(x,y);}
	
	return 0;
}

long eval(mpc_ast_t* t){
	
	// if tag is number then jsut return it as is (since operator will be first if there's anything to add)
	if (strstr(t->tag, "number")){
		return atoi(t->contents);
	}
	
	//operator is always 2nd
	char* op = t-> children[1]->contents;
	//then comes the first number
	long x = eval(t->children[2]);
	// then expression
	int i = 3;
	while (strstr(t->children[i]->tag, "expr")){
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}
	
	return x;
}

int main(int argc, char** argv){
	
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	
	mpca_lang(MPCA_LANG_DEFAULT,
		"									\
			number	:	/-?[0-9]+/;			\
			operator:	'+' | '-' | '*' | '/' | '^' | '%' | 'n' | 'x';	\
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
			// puts("start of abstract syntax tree");
			// mpc_ast_t* a = r.output;
			// printf("tag: %s\n", a ->tag);
			// printf("contents: %s\n", a ->contents);
			// printf("number of children: %i\n", a ->children_num);
			
			// puts("first child :");
			// mpc_ast_t* c0 = a->children[0];
			// printf("first child tag: %s\n", c0->tag);
			// printf("first child contents: %s\n", c0->contents);
			// printf("first child number of children: %i\n", c0->children_num);
			// puts("end of first child");
			// puts("end of abstract syntax tree");
			
			//On success Print the AST//
			//mpc_ast_print(r.output);
			long result = eval(r.output);
			printf("%li\n", result);
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