#include <stdio.h> //stdio.h -> standard IO header
#include "mpc.h"
#include <math.h>

static char input[2048];

#define LASSERT(args, cond, fmt, ...)\
	if (!(cond)) {\
		lval* err = lval_err(fmt, ##__VA_ARGS__);	\
		lval_del(args); \
		return err;\
		}

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
    "Function '%s' passed incorrect type for argument %i. " \
    "Got %s, Expected %s.", \
    func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
    "Function '%s' passed incorrect number of arguments. " \
    "Got %i, Expected %i.", \
    func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
    "Function '%s' passed {} for argument %i.", func, index);
	
struct lval;
struct lenv;
typedef struct lval lval; // naming these strcuts as lval;
typedef struct lenv lenv; // naming the struct as lenv;
typedef lval* (*lbuiltin)(lenv*, lval*);
	
struct lval{
	int type;
	
	//basic
	long num;
	char* err;
	char* sym;
	
	//function
	lbuiltin builtin; // builtin function
	lenv* env;
	lval* formals;
	lval* body;
	
	//expression
	int count;
	lval** cell; // pointer to a list of pointers to lval
};


struct lenv{
	lenv* par; // parent environment (also allows for nested functions/methods !!!!)
	int count;
	char** syms;
	lval** vals;
};


enum {LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN};

enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

lval* lval_builtin(lbuiltin fun){
	lval* v = malloc(sizeof(lval));
	v -> type = LVAL_FUN;
	//v -> count = 0;
	//v -> cell = NULL;
	v -> builtin = fun;
	return v;	
}

lenv* lenv_new(void){
	lenv* e = malloc(sizeof(lenv));
	e->par = NULL;
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	return e;	
}

void lenv_del(lenv* e);

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
		case LVAL_QEXPR:
			for (int i = 0; i < v->count;i++)
			{
				lval_del(v->cell[i]);				
			}
			free(v->cell);
			break;
		case LVAL_FUN:
			if (!v->builtin){ // if there's no builtin --> there's a function and hence we need to delete the function
				lenv_del(v->env);
				lval_del(v->formals);
				lval_del(v->body);
			}
			break;
	}
	free(v);
}

void lenv_del(lenv* e){
	
	for (int i = 0; i < e->count; i++){
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

lenv* lenv_copy(lenv* e);

lval* lval_copy(lval* v){
	
	lval* x = malloc(sizeof(lval));
	x->type = v ->type;
	
	switch (v->type){
		case LVAL_FUN:
			if (v->builtin){
				x->builtin = v->builtin;
				} else {
					x->builtin = NULL;
					x->env = lenv_copy(v->env);
					x->body = lval_copy(v->body);
					x->formals = lval_copy(v->formals);
				}
			break;
		case LVAL_NUM:
			x->num = v->num; break;		
		case LVAL_ERR:
			x->err = malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err);
			break;		
		case LVAL_SYM:
			x->sym = malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym);
			break;		
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			x->count = v->count;
			x->cell = malloc(sizeof(lval*) * v->count);
			for (int i = 0; i< v->count; i++){
					x->cell[i] = lval_copy(v->cell[i]); // copying each subexpression in the pointers list
			}
			break;
	}
	return x;	
}

lval* lval_lambda(lval* formals, lval* body){
	
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	
	//set builtin to null
	v->builtin = NULL;
	
	//build new environment
	v->env = lenv_new();
	
	//set formals and body
	v-> formals = formals;
	v->body = body;
	return v;	
}

void lenv_put(lenv* e, lval* k, lval* v){
	for (int i = 0; i<e->count; i++){ // finding a matching variable name
		if (strcmp(e->syms[i], k->sym)==0){ // found matching variable name
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}
	
	// no existing variable name found, time to create and reallocate more memory to put new variable in.
	e->count ++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);

	e->vals[e->count - 1] = lval_copy(v);
	e->syms[e->count - 1] = malloc(sizeof(k->sym) + 1); // remember the terminating null char
	strcpy(e->syms[e->count - 1], k->sym);
}

lval* lval_qexpr(void){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v-> count = 0;
	v->cell = NULL;
	return v;	
}

lval* lval_num(long x){
	lval* v = malloc(sizeof(lval)); // creation of a new lval instance
	v -> type = LVAL_NUM;
	v -> num = x; // refers to the input x
	//no v.err because this is not an error case.
	return v;
}

lval* lval_err(char* fmt, ...){
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	
	/* old implementation
	//v->err = malloc(strlen(m)+1); // why +1 ? this is because strings are stored as an array of chars followed by a NULL byte '\0' that terminates the string
	//strcpy(v->err,m);
	// error yet to be assigned since it's unknown what it is
	*/
	
	/*new implementation with formatting*/
	// creating and initialization of a va list (variable argument list)
	va_list va;
	va_start(va, fmt);
	
	//allocating 512 bytes of space
	v-> err = malloc(512);
	//printf the error string with a max of 511 characters
	vsnprintf(v->err, 511, fmt, va);
	// reallocate number of bytes used
	v->err = realloc(v->err, strlen(v->err) +1);
	
	//cleanup the va list
	va_end(va);
	
	
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

lval* lenv_get(lenv* e, lval* k){
	
	//ITERATING OVER ENVIRONMENT SYMBOLS
	for (int i = 0; i < e->count; i++){
		if (strcmp(e->syms[i], k->sym)==0) { // FINDING A SYMBOL MATCH
			return lval_copy(e->vals[i]); // returning a copy of the corresponding value
		}
	}
	
	if (e->par){
		return lenv_get(e->par, k);
	} else {
	return lval_err("unbound symbol '%s'", k->sym);
	} // no symbols found to match, hence error is returned;	
}

lval* lval_take(lval* v, int i){
	lval* x = lval_pop(v,i);
	lval_del(v);
	return x;
}



lenv* lenv_copy(lenv* e){	
	lenv* n = malloc(sizeof(lenv));
	n->par = e->par;
	n->count = e->count;
	n->syms = malloc(sizeof(char*) * n->count);
	n->vals = malloc(sizeof(lval*) * n->count);
	for(int i = 0; i<e->count; i++){
		n->syms[i] = malloc(strlen(e->syms[i])+1);
		strcpy(n->syms[i], e->syms[i]);
		n->vals[i] = lval_copy(e->vals[i]);
	}
	return n;	
}

void lenv_def(lenv* e, lval* k, lval* v){
	
	while(e->par) {e=e->par;}
	lenv_put(e,k,v);
	
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
	if (strstr(t->tag, "qexpr")) {x = lval_qexpr();}
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr();}
	if (strstr(t->tag, "sexpr")){ x = lval_sexpr();}
	
	for (int i = 0; i < t->children_num; i++){
		
		if (strcmp(t->children[i]->contents, "(") == 0) {continue;}
		if (strcmp(t->children[i]->contents, ")") == 0) {continue;}
		if (strcmp(t->children[i]->tag, "regex") == 0) {continue;}
		if (strcmp(t->children[i]->contents, "{") == 0) {continue;}
		if (strcmp(t->children[i]->contents,"}") == 0) {continue;}
		x = lval_add(x, lval_read(t->children[i]));
		
	}
	
	return x;
	
}

lval* lval_join(lval* x, lval* y){
	// ffor each cell in y add it to x
	while (y->count){
		x = lval_add(x, lval_pop(y,0));
	}
	lval_del(y);
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
		case LVAL_QEXPR:
		lval_expr_print(v, '{', '}');
		break;
		case LVAL_FUN:
			if (v->builtin){
				printf("<builtin>");
			} else {
				printf("(\\ ");
				lval_print(v->formals);
				putchar(' ');
				lval_print(v->body);
				putchar(')');
			}		
		break;
	}
}





void lval_println(lval* v){
	lval_print(v); 
	putchar('\n');
}

lval* builtin_op(lenv* e, lval* a, char* op){
	
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

lval* lval_eval(lenv* e, lval* v);
lval* builtin(lval* a, char* func);
lval* lval_call(lenv* e, lval* f, lval* v);
lval* lval_eval_sexpr(lenv* e,lval* v){
	//evaluating every cell/children
	for (int i = 0; i < v ->count; i++){
		v->cell[i] = lval_eval(e, v->cell[i]);
	}
	
	// checking for error
	for (int i = 0; i < v-> count; i++){
		if (v->cell[i]->type == LVAL_ERR){ return lval_take(v,i);} // extracting the cell/expression with error
	}
	
	//empty expression i.e. ()
	if (v->count == 0){ return v;}
	
	// single expression
	if (v->count == 1){return lval_eval(e, lval_take(v,0));}
	
	// ensuring first element is a symbol
	// if not the case, then the whole expression is deleted since it's an error
	lval* f = lval_pop(v,0);
	if (f->type != LVAL_FUN){
		lval* err = lval_err(
		"S-expression starts with incorrect type. "
		"Got %s, Expected %s.",
		ltype_name(f->type), ltype_name(LVAL_FUN));
		lval_del(f);
		lval_del(v);
		return err;
	}
	
	// call built in with operator
	
	lval* result = lval_call(e,f,v);
	lval_del(f);
	return result;
}

lval* lval_eval(lenv* e, lval* v){
	if (v->type == LVAL_SYM){
		lval* x = lenv_get(e,v);
		lval_del(v);
		return x;
	}
	// evaluating Special Expressions
	if (v->type == LVAL_SEXPR){return lval_eval_sexpr(e, v);}
	return v;
}


lval* builtin_head(lenv* e, lval* a){
	
	//checking for error conditions//
	LASSERT(a, a->count==1,"Function 'head' passed too many arguments! Got %i, Expected %i .", a->count, 1);
	
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'head' passed incorrect types!!");
	
	LASSERT(a, a->cell[0]->count != 0,"Function head passed {} (nothing)");
	
	lval* v = lval_take(a, 0);
	
	// delete everything until only head is left
	while (v->count > 1){		
		lval_del(lval_pop(v, 1));		
	}
	return v;	
}

lval* builtin_tail(lenv* e, lval* a){
	
	// check for error conditions//
	LASSERT(a, a->count==1,"Function 'tail' passed too many arguments!!");
	
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,"Function 'tail' passed incorrect types!!");
	
	LASSERT(a, a->cell[0]->count != 0,"Function 'tail' passed {} (nothing)");
	
	// taking first cell
	lval* v = lval_take(a,0);
	
	// delete first element and return
	lval_del(lval_pop(v,0));
	
	return v;
	
}

lval* builtin_list(lenv* e, lval* a){
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_eval(lenv* e, lval* a){
	LASSERT(a, a->count == 1,
	"Function 'eval' passed too many arguments!");
	LASSERT(a,a->cell[0]->type == LVAL_QEXPR,
	"Function 'eval' passed wrong type");
	lval* x = lval_take(a,0);
	x->type = LVAL_SEXPR;
	return lval_eval(e, x);	
}

lval* builtin_join(lenv* e, lval* a){
	for (int i = 0; i < a -> count; i++){
		LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed wrong type!");
	}
	lval* x = lval_pop(a,0);
	
	while (a->count){
		x = lval_join(x, lval_pop(a, 0));
	}
	
	lval_del(a);
	return x;	
}

lval* builtin_add(lenv* e, lval* a){
	return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a){
	return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a){
	return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a){
	return builtin_op(e, a, "/");
}

lval* builtin_pow(lenv* e, lval* a){
	return builtin_op(e,a, "^");
}

lval* builtin_max(lenv* e, lval* a){
	return builtin_op(e,a,"x");
}

lval* builtin_min(lenv* e, lval* a){
	return builtin_op(e,a,"n");
}



lval* builtin_var(lenv* e, lval* a, char* func){
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
	"Function 'def' was passed incorrect type! (Expected: Qexpr)"); // ensuring that this is all in {} brackets
	
	//first argument is symbol list // i.e. { lolol kekeke } 100 referring to {lolol kekeke}
	lval* syms = a->cell[0];
	
	//ensure all elements of first list are symbols; ensuring that "lolol" are all symbols/allowed in the symbols regex
	for (int i = 0; i < syms->count; i++){
		LASSERT(a, syms->cell[i]->type == LVAL_SYM,
		"Function 'def' cannot define non-symbol"); // this would mean that there's a non-symbol in the list, hence it is not allowed
	}	//check for non symbol ended
	
	// check correct number of symbols and values
	LASSERT(a, syms->count == a->count - 1,  // a->count-1 EXCLUDES the first qexpr {lolol kekeke}
	"Function 'def' cannot define incorrect number of values to symbols. Ensure there's same number of symbols to values!!!") 
	
	//assigning copies of  values to symbols (making them variables)
	for (int i = 0; i < syms->count; i++){
		if(strcmp(func, "def") == 0){
			lenv_def(e, syms->cell[i], a->cell[i+1]); // defining global variables
		}
		if (strcmp(func, "=") == 0){
		lenv_put(e, syms->cell[i], a->cell[i+1]); // defining local variables (in local environment)
		}		
	}
	lval_del(a);
	return lval_sexpr();
	
}

lval* builtin_lambda(lenv* e, lval* a){
	// check two arguments, each of which are q expressions (first one being formals, second one being the body)
	//LASSERT(a, a->count == 2, "Lambda does not have arguments");
	LASSERT_NUM("\\", a, 2);
	//LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "First argument is not a Qexpr.");
	LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
	//LASSERT(a, (a->cell[1]->type == LVAL_QEXPR), "First argument is not a Qexpr.");
	LASSERT_TYPE("\\",a, 1, LVAL_QEXPR);
	
	// checking if first q expr contains only symbols);
	for (int i = 0; i < a->cell[0]->count; i++){
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM), 
		"Cannot define non-symbol, Got %s, Expected %s.", 
		ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));	
	}
	// pop first two arguments and pass them into lval_lambda
	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);
	lval_del(a);
	
	return lval_lambda(formals, body);
}


lval* builtin_def(lenv* e, lval* a){
	return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e ,lval* a){
	return builtin_var(e, a, "=");	
}

lval* lval_call(lenv* e, lval* f, lval* a){
	//if builtin then just simply call that
	if (f->builtin){
		return f->builtin(e,a);
	}
	
	int given = a->count;
	int total = f->formals->count;
	
	while (a->count){		
		//if no more formal arguments to bind to
		if (f->formals->count == 0){
			lval_del(a);
			return lval_err("Function passed too many arguments. "
			"Got %i, Expected %i. ", given, total);
		}		
		
		//pop first symbol from formals
		lval* sym = lval_pop(f->formals,0);
		
		//special case to deal with '&'
		if (strcmp(sym->sym, "&") == 0){
		
			if (f->formals->count != 1){
				lval_del(a);
				return lval_err("Function format invalid."
				"Symbol '&' not followed by single symbol.");
			}
		
			// next formal should be bound to remaining arguments
			lval* nsym = lval_pop(f->formals, 0);
			lenv_put(f->env, nsym, builtin_list(e,a));
			lval_del(sym);
			lval_del(nsym);
			break;		
		}
		
		//pop the next argument from the list
		lval* val = lval_pop(a,0);
		
		// bind a copy into the function's environment
		lenv_put(f->env, sym, val);
		
		//delete sym and val
		lval_del(sym);
		lval_del(val);
	}
	
	
	//arguments list all bound can can be del/cleaned up
	lval_del(a);
	
	//if '&' remains in formal list bind to empty list
	if (f->formals->count > 0 &&
	strcmp(f->formals->cell[0]->sym,"&") == 0){
		//check to ensure that & is not passed invalidly
		if (f->formals->count != 2) {
			return lval_err("Function format invalid." 
			"Symbol '&' not followed by single symbol.");
		}
		
		// pop and delete & symbol
		lval_del(lval_pop(f->formals, 0));
		
		//pop next symbol and create empty list
		lval* sym = lval_pop(f->formals,0);
		lval* val = lval_qexpr();
		
		//bind to env and delete
		lenv_put(f->env, sym, val);
		lval_del(sym);
		lval_del(val);
	}
		//if all formals have been bound evaluate
	if (f->formals->count == 0){
		// set env for evaluation environment
		f->env->par = e;
		//evaluate and return
		return builtin_eval(
		f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
	} else {
		return lval_copy(f);
	}
}	
	
	// //assign each argument to each formal in order
	// for (int i = 0; i < a->count; i++){
		// lenv_put(f->env, f->formals->cell[i], a->cell[i]);
	// }
	
	// lval_del(a);
	
	// //setting parent environment
	// f->env->par = e;
	
	// //evaluation
	// return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));	

void lenv_add_builtin(lenv* e, char* name, lbuiltin func){
	lval* k = lval_sym(name); // making an actual symbol value out of name
	lval* v = lval_builtin(func); // making a function reference/clone out of the func
	lenv_put(e, k, v);
	lval_del(k);
	lval_del(v);
}

void lenv_add_builtins(lenv* e){
	
	//lambda functions
	lenv_add_builtin(e, "\\", builtin_lambda);
	lenv_add_builtin(e, "=", builtin_put);
	
	//variables functions
	lenv_add_builtin(e, "def", builtin_def);
	
	//list functions
	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "join", builtin_join);
	lenv_add_builtin(e, "eval", builtin_eval);
	
	//mathematical functions/operators
	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);
	lenv_add_builtin(e, "^", builtin_pow);
	lenv_add_builtin(e, "x", builtin_max);
	lenv_add_builtin(e, "n", builtin_min);
}
	
// lval* builtin(lval* a, char* func){
	// if (strcmp("list", func) == 0) {return builtin_list(a);}
	// if (strcmp("head", func) == 0) {return builtin_head(a);}
	// if (strcmp("tail", func) == 0) {return builtin_tail(a);}
	// if (strcmp("join", func) == 0) {return builtin_join(a);}
	// if (strcmp("eval", func) == 0) {return builtin_eval(a);}
	// if (strcmp("+-/*%^nx", func) == 0) {return builtin_op(a, func);}

	// lval_del(a);
	// return lval_err("Unknown function!");
	
// }

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
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");
	
	mpca_lang(MPCA_LANG_DEFAULT,
		"									\
			number	:	/-?[0-9]+/;			\
			symbol:	/[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/; \
			sexpr : '(' <expr>* ')' ;						\
			qexpr : '{' <expr>* '}'; \
			expr	:	<number> | <symbol> | <sexpr> | <qexpr> ;\
			lispy : /^/ <expr>* /$/; \
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
	
	lenv* e = lenv_new();
	lenv_add_builtins(e);
	
	puts("Welcome to GLisp");
	puts("Press Ctrl+c to exit");
	
	while (1) {
		fputs("Glispy>", stdout);
		fgets(input, 2048, stdin);
		//printf("You have entered %s", input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)){
			lval* x = lval_eval(e,lval_read(r.output));
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		} else {
			//if not, print error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);			
		}
	}
	lenv_del(e);
	mpc_cleanup(6,Number,Symbol, Sexpr, Qexpr, Expr, Lispy);
	
	return 0;
}




// run using gcc -std=c99 -Wall hello_world.c -o hello_world
// compiles code in this file, report any wranings, and outputs the program to a new file called hello_world
// -std=c99 is a flag used to tell compiler which version of C we're using, ensures code is standardized
// -o means output, hello_world means the directory/destination
// 