// › Write a recursive function to compute the number of leaves of a tree.
int leaves_number(tree){	
	if (tree -> childrenCount == 0){return 1;}
	
	
	int total = 0;
	while (int i = 0 ; tree -> childrenCount; i++){
		total += leaves_number->children[i];
	}
	return total;	
}
	
// › Write a recursive function to compute the number of branches of a tree.
int branches_number(tree){
	
	if (tree -> childrenCount == 0){return 1;}
	
	int total = 0;
	while(int i = 0; tree->childrenCount; i++){
		total += branches_number(tree->children[i]); 
	}	
	return total;
}
// › Write a recursive function to compute the most number of children spanning from one branch of a tree.
int most_children_branch(tree){
	if (tree -> childrenCount == 0){return 0;}
	
	int max = 0;
	while(int i = 0; tree->childrenCount; i++){
		int currentChild= tree->children[i];
		if (currentChild -> childrenCount > max){ // not sure if this part is needed....
			max = currentChild -> childrenCount;
		}
		int maxC = most_children_branch(currentChild);
		if (maxC > max){
			max = maxC;
		}
	}
	return max;
}
// › How would you use strstr to see if a node was tagged as an expr?
if (strstr(node -> tags, "expr")) {return 1;}
// › How would you use strcmp to see if a node had the contents '(' or ')'?
if (strcmp(node->contents, "(") == 0 || strcmp(node->contents,")") == 0) {return 1;}
// › Add the operator %, which returns the remainder of division. For example % 10 6 is 4.
# done in evaluationPlus.exe
// › Add the operator ^, which raises one number to another. For example ^ 4 2 is 16.
# requires math.h header
# done in evaluationPlus.exe
// › Add the function min, which returns the smallest number. For example min 1 5 3 is 1.
# done in evaluationPlusPlus.exe as 'n' operator
# note: caused Parser Undefined error when using multi char operators like "min" and "max"
// › Add the function max, which returns the biggest number. For example max 1 5 3 is 5.
# done in evaluationPlusPlus.exe as 'x' operator
// › Change the minus operator - so that when it receives one argument it negates it.
# you mean make it negative...? or what