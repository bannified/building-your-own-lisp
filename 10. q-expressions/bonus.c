› What are the four typical steps for adding new language features?
Add Syntax (new rule to the language grammar for the new feature)
Representation (new data type (or symbol) to represent this new feature) eg. sexpr qexpr
Parsing (A way to read said feature's symbol and syntax from the abstract syntax tree)
Semnatics (The actual mechanics of the function, how it's evaluated and manipulated)
› Create a Macro specifically for testing for the incorrect number of arguments.
#define INCORRARG(args, desired_count){ \
	if (args->count != desired_count){lval_del(args); return lval_err("Incorrect number of args given");}\
}
› Create a Macro specifically for testing for being called with the empty list.
#define ISEMPTYLIST(list){ \
	if (list->count == 0){lval_del(args); return lval_err("empty list was given");}\
}
› Add a builtin function cons that takes a value and a Q-Expression and appends it to the front.
› Add a builtin function len that returns the number of elements in a Q-Expression.
› Add a builtin function init that returns all of a Q-Expression except the final element.