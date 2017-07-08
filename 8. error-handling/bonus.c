› Run the previous chapter's code through gdb and crash it. See what happens.
// havent done so
› How do you give an enum a name?

typedef enum {
	Nozomi,
	Rin,
	Umi
} LilyWhite;

› What are union data types and how do they work?
Unions are often used to convert between the binary representations of integers and floats

union types allow for different data types to be stored in the same memory location.
basically a multi-type type for different types that wouldnt be associated otherwise like int and string and char

can be initialized as a local instance or made into a type

#local instance
union {
	int i;
	char str[20];
} playerData;

#global

union PlayerData {
	int i;
	char str[20];
};

› What are the advantages over using a union instead of struct?
a guess: union stores within the same memory location, while struct basically references to different memory locations with each of its variables/fields.

main difference is in memory management.
union can only save one type of data at a time. it's kind of like a static global variable (will be overwritten regardless of location it is modified)
structs are usually instanced based and can be modified independently to store its own variables in its fields AT THE COST OF MORE MEMORY

› Can you use a union in the definition of lval?
no...? you need to go through the entire expression for evaluation. 
› Extend parsing and evaluation to support the remainder operator %.
› Extend parsing and evaluation to support decimal types using a double field.