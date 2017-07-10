#include <stdio.h> //stdio.h -> standard IO header
#include "mpc.h"
#include <math.h>

static char input[2048];

typedef struct lval{
	int type;
	long num;
	//double doub;
	char* err;
	char* sym;
	//count and pointer to a list of lval
	int count;
	struct lval** cell; // pointer to a list of pointers to lval
} lval;

enum {LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

lval* lval_num(long x){
	lval* v = malloc(sizeof(lval)); // creation of a new lval instance
	v -> type = LVAL_NUM;
	v -> num = x; // refers to the input x
	//no v.err because this is not an error case.
	return v;
}

lval* lval_err(char* m){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m)+1); // why +1 ? this is because strings are stored as an array of chars followed by a NULL byte '\0' that terminates the string
	strcpy(v->err,m);
	// error yet to be assigned since it's unknown what it is
	return v;
}

lval* lval_sym(char* s){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s)+1);
	strcpy(v->sym,s);
	return v;
}

lval* lval_sexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_pop(lval* v, int i){
	//finding the item at i
	lval* x = v->cell[i];
	
	//shifitng the memory after cell i back to where cell i was to take its place
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v-> count-i-1));
	
	v-> count--;
	
	v-> cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;	
}



void lval_del(lval* v){
	switch (v->type){
		case LVAL_NUM:
			break;
		case LVAL_ERR:
			free(v->err);
			break;
		case LVAL_SYM:
			free(v->sym);
			break;
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++){
				lval_del(v -> cell[i]); 
			}
			free(v->cell);
			break;
	}
	free(v);
}

lval* lval_take(lval* v, int i){
	lval* x = lval_pop(v,i);
	lval_del(v);
	return x;
}

// void lval_print(lval* v){
	// switch (v -> type){
		// // case being the resultant lval is a number, hence no error, and we print out the number
		// case LVAL_NUM: 
		// printf("%li", v->num); 
		// break;
		// // case being there's an error, we'll check what kind and print out the appropriate error message
		// case LVAL_ERR: // 
			// //could probably use another switch statement here though...?
			// if (v->err == LERR_DIV_ZERO){
				// printf("Error: Division by Zero DOME DOME DOME");
			// }
			// if (v->err == LERR_BAD_OP){
				// printf("Error: Inappropriate operator used!!! NOPE");
			// }
			// if (v->err == LERR_BAD_NUM){
				// printf("Error: An invalid number detected. Check for 322 pls");
			// }
			// break;
	// }
// }

// as good as concatenation/appending i think
lval* lval_add(lval* v, lval* x){
	
	v->count++; // adds 1 more to the count
	v->cell = realloc(v->cell, sizeof(lval*) * v->count); // recalculate to +1 more lval size memory
	v->cell[v->count-1] = x; // assign the pointer to this pointer lists LAST index 
	return v;
	return 0;
	
}	

lval* lval_read_num(mpc_ast_t* t){
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE?
		lval_num(x):lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t){
	
	if (strstr(t->tag, "number")) { return lval_read_num(t);}
	if (strstr(t->tag, "symbol")) {return lval_sym(t -> contents);}
	
	// if root (>) or sexpr then create empty list
	lval* x = NULL;
	
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr();}
	if (strstr(t->tag, "sexpr")){ x = lval_sexpr();}
	
	for (int i = 0; i < t->children_num; i++){
		
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->tag, "regex") == 0) {continue;}
		x = lval_add(x, lval_read(t->children[i]));
		
	}
	
	return x;
	
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close){
	putchar(open);
	for (int i = 0; i < v->count; i++){
		
		lval_print(v->cell[i]);
		
		if (i != (v->count-1)){
			putchar(' ');	
		}
		
	}
	putchar(close);
	
}

	
//print lval following new new line (ln)
void lval_print(lval* v){
	
	switch(v->type){
		case LVAL_NUM: 
		printf("%li", v->num); 
		break;
		case LVAL_SYM:
		printf("%s", v->sym);
		break;
		case LVAL_ERR:
		printf("Error: %s", v->err);
		break;
		case LVAL_SEXPR:
		lval_expr_print(v, '(', ')');
		break;
	}
	
}

void lval_println(lval* v){
	lval_print(v); 
	putchar('\n');
}

lval* builtin_op(lval* a, char* op){
	
	// ensuring that all arugments are numbers
	for (int i = 0; i < a->count; i++)
	{
		if (a->cell[i]->type != LVAL_NUM){
			lval_del(a);
			return lval_err("Cannot operate on non numbers!");
		}
	}
	
	// popping the first element
	lval* x = lval_pop(a,0);
	
	// if no arguments and sub experssions then perform unary negation 
	//WHAT IS UNARY NEGATION??!?/
	if ((strcmp(op, "-") == 0) && a->count == 0){
		x->num = -x->num;
	}
	
	// while there are still elements remaining
	
	while(a->count > 0){
		
		lval* y = lval_pop(a,0);
		if (strcmp(op,"+") == 0) {x->num = (x -> num + y->num);}
		if (strcmp(op,"-") == 0) {x->num = (x->num - y->num);}
		if (strcmp(op, "*") == 0) {x->num = (x->num * y->num);}
		if (strcmp(op, "/") == 0) {
			if (y->num == 0)
			{	
				lval_del(x); 
				lval_del(y);
				x = lval_err("Division by zero!"); 
				break;
			}
			else {
				x->num = x->num / y->num;
			}
		}
		if (strcmp(op, "%") == 0) {
			if (y->num == 0)
			{	
				lval_del(x); 
				lval_del(y);
				x = lval_err("Division by zero!"); 
				break;
			}
			else {
				x->num = x->num % y->num;
			}
		}
		if (strcmp(op, "^") == 0) {x->num = (powl(x->num,y->num));}
		if (strcmp(op, "n") == 0) {x->num = (fminl(x->num,y->num));}
		if (strcmp(op, "x") == 0) {x->num = (fmaxl(x->num,y->num));}
		
		lval_del(y);
		
	}
	
	lval_del(a);
	return x;
	
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v){
	//evaluating every cell/children
	for (int i = 0; i < v ->count; i++){
		v->cell[i] = lval_eval(v->cell[i]);
	}
	
	// checking for error
	for (int i = 0; i < v-> count; i++){
		if (v->cell[i]->type == LVAL_ERR){ return lval_take(v,i);} // extracting the cell/expression with error
	}
	
	//empty expression i.e. ()
	if (v->count == 0){ return v;}
	
	// single expression
	if (v->count == 1){return lval_take(v, 0);}
	
	// ensuring first element is a symbol
	// if not the case, then the whole expression is deleted since it's an error
	lval* f = lval_pop(v,0);
	if (f->type != LVAL_SYM){
		lval_del(f);
		lval_del(v);
		return lval_err("S-expression does not start with a symbol");
	}
	
	// call built in with operator
	
	lval* result = builtin_op(v, f-> sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v){
	// evaluating Special Expressions
	if (v->type == LVAL_SEXPR){return lval_eval_sexpr(v);}
	return v;
}


// lval* eval_op(lval* x, char* op, lval* y){
	
	// if (x.type == LVAL_ERR){ return x;}
	// if (y.type == LVAL_ERR){ return y;}
	
	// if (strcmp(op,"+") == 0) {return lval_num(x -> num + y->num);}
	// if (strcmp(op,"-") == 0) {return lval_num(x->num - y->num);}
	// if (strcmp(op, "*") == 0) {return lval_num(x->num * y->num);}
	// if (strcmp(op, "/") == 0) {
		// return y->num == 0
		// ? lval_err(LERR_DIV_ZERO)
		// : lval_num(x->num / y->num);
		// }
	// if (strcmp(op, "%") == 0) {
		// return y->num == 0 
		// ? lval_err(LERR_DIV_ZERO)
		// :lval_num(x->num % y->num);
	// }
	// if (strcmp(op, "^") == 0) {return lval_num(powl(x->num,y->num));}
	// if (strcmp(op, "n") == 0) {return lval_num(fminl(x->num,y->num));}
	// if (strcmp(op, "x") == 0) {return lval_num(fmaxl(x->num,y->num));}
	
	// //if it reaches until this point, return bad operator snice non of these operators work
	// return lval_err(LERR_BAD_OP);
// }



// lval* eval(mpc_ast_t* t){	
	// // if tag is number then jsut return it as is (since operator will be first if there's anything to add)
	// if (strstr(t->tag, "number")){
		// errno = 0;
		// long x = strtol(t->contents, NULL, 10);
		// return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
		// // if no number error, return a num type lval, if not, return error type (of bad num type) lval
	// }
	
	// //operator is always 2nd
	// char* op = t-> children[1]->contents;
	// //then comes the first number
	// lval x = eval(t->children[2]);
	// // then expression
	// int i = 3;
	// while (strstr(t->children[i]->tag, "expr")){
		// x = eval_op(x, op, eval(t->children[i]));
		// i++;
	// }	
	// return x;
// }

int main(int argc, char** argv){
	
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	//mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	
	mpca_lang(MPCA_LANG_DEFAULT,
		"									\
			number	:	/-?[0-9]+/;			\
			symbol:	'+' | '-' | '*' | '/' | '^' | '%' | 'n' | 'x';	\
			sexpr : '(' <expr>* ')' ;						\
			expr	:	<number> | <symbol> | <sexpr> ;\
			lispy : /^/ <expr>* /$/; \
		",
		Number, Symbol, Sexpr, Expr, Lispy);
	
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
			// lval result = eval(r.output);
			// lval_println(result);
			//printf("%li\n", result);
			
			lval* x = lval_eval(lval_read(r.output));
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		} else {
			//if not, print error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
			
		}
		//free(input);
	}
	
	mpc_cleanup(5,Number,Symbol, Sexpr, Expr, Lispy);
	
	return 0;
}




// run using gcc -std=c99 -Wall hello_world.c -o hello_world
// compiles code in this file, report any wranings, and outputs the program to a new file called hello_world
// -std=c99 is a flag used to tell compiler which version of C we're using, ensures code is standardized
// -o means output, hello_world means the directory/destination
// 