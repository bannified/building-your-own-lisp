#include <stdio.h> //stdio.h -> standard IO header
#include "mpc.h"
#include <math.h>

static char input[2048];

typedef struct {
	int type;
	long num;
	int err;
} lval;

enum {LVAL_NUM, LVAL_ERR};

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

lval lval_num(long x){
	lval v; // creation of a new lval instance
	v.type = LVAL_NUM;
	v.num = x; // refers to the input x
	//no v.err because this is not an error case.
	return v; 	
}

lval lval_err(long x){
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	// error yet to be assigned since it's unknown what it is
	return v;
}

void lval_print(lval v){
	switch (v.type){
		// case being the resultant lval is a number, hence no error, and we print out the number
		case LVAL_NUM: 
		printf("%li", v.num); 
		break;
		// case being there's an error, we'll check what kind and print out the appropriate error message
		case LVAL_ERR: // 
			//could probably use another switch statement here though...?
			if (v.err == LERR_DIV_ZERO){
				printf("Error: Division by Zero DOME DOME DOME");
			}
			if (v.err == LERR_BAD_OP){
				printf("Error: Inappropriate operator used!!! NOPE");
			}
			if (v.err == LERR_BAD_NUM){
				printf("Error: An invalid number detected. Check for 322 pls");
			}
			break;
	}
}
//print lval following new new line (ln)
void lval_println(lval v){
	lval_print(v);
	putchar('\n');
}

lval eval_op(lval x, char* op, lval y){
	
	if (x.type == LVAL_ERR){ return x;}
	if (y.type == LVAL_ERR){ return y;}
	
	if (strcmp(op,"+") == 0) {return lval_num(x.num + y.num);}
	if (strcmp(op,"-") == 0) {return lval_num(x.num - y.num);}
	if (strcmp(op, "*") == 0) {return lval_num(x.num * y.num);}
	if (strcmp(op, "/") == 0) {
		return y.num == 0
		? lval_err(LERR_DIV_ZERO)
		: lval_num(x.num / y.num);
		}
	if (strcmp(op, "%") == 0) {return lval_num(x.num % y.num);}
	if (strcmp(op, "^") == 0) {return lval_num(powl(x.num,y.num));}
	if (strcmp(op, "n") == 0) {return lval_num(fminl(x.num,y.num));}
	if (strcmp(op, "x") == 0) {return lval_num(fmaxl(x.num,y.num));}
	
	//if it reaches until this point, return bad operator snice non of these operators work
	return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t){	
	// if tag is number then jsut return it as is (since operator will be first if there's anything to add)
	if (strstr(t->tag, "number")){
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
		// if no number error, return a num type lval, if not, return error type (of bad num type) lval
	}
	
	//operator is always 2nd
	char* op = t-> children[1]->contents;
	//then comes the first number
	lval x = eval(t->children[2]);
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
			lval result = eval(r.output);
			lval_println(result);
			//printf("%li\n", result);
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