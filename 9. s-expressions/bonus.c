› Give an example of a variable in our program that lives on The Stack.
any local lval* pointers declared at the start of the lval_X functions

› Give an example of a variable in our program that points to The Heap.

the lval* that gets passed around from one function to another during evaluation

› What does the strcpy function do?
copy the null terminated string (second arg) to the destination (first argument)

› What does the realloc function do?
reallocates the already allocated area of memory passed as an argument to the function. first argument points to the area, second is the new size of the memory)
› What does the memmove function do?
copies the memory pointed by src (2nd argument) to dest (1st argument), count (3rd arg) determines number of bytes to copy
› How does memmove differ from memcpy?
in memcpy, the destination cannot overlap the source, while this is possible in memmove.
memmove is slightly slower since it does not make this assumption.
memcpy copies address from low to high (heap to stack?) and can overwrite addresses if there is an overlap
memmove takes this overlapping possibility into consideration and can switch to copying from high to low to prevent overlapping (and hence can be less efficient) which takes more time.
› Extend parsing and evaluation to support the remainder operator %.
done done
› Extend parsing and evaluation to support decimal types using a double field.
not done...