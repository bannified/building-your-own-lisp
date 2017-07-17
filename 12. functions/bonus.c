› Define a Lisp function that returns the first element from a list.
def {get-first} (\ { list } { head list })
get-first {2 3 4 5 6 7}
>> 2	
› Define a Lisp function that returns the second element from a list.
def {get-second} (\ {list} {head (tail list)})
OR
def {get-second} (\ {list} {get-first (tail list)})
get-second {2 3 4 5 6 7}
>> 3
› Define a Lisp function that calls a function with two arguments in reverse order.
def {reverse-two} (\ {x y func} {func y x})

---
def {subtraction} (\ {x y} {- x y})
subtraction 3 1
>> 2
subtraction 1 3
>> -1
reverse-two 3 1 subtraction
>> -1
› Define a Lisp function that calls a function with arguments, then passes the result to another function.
def {transfer} (\ {func args} {func args} ) ??? not so sure
› Define a builtin_fun C function that is equivalent to the Lisp fun function.
› Change variable arguments so at least one extra argument must be supplied before it is evaluated.