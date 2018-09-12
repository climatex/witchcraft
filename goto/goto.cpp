// Can be compiled under Visual Studio (target: x86 - Win32) or similar

#include <stdio.h>
#include "goto.h"

int return_one(const int one = 1)
{
global_label:
	printf("\rIn return_one()...");
	prep(global_label, "global_label");			// Initializes global goto label "global_label"

	return one;						// Will return either 1 or 2
}

int return_two(const int two = 2)
{
	printf("\rIn return_two()...");
	prep(exit_label, "exit_label");				// Initializes global goto label "exit_label"
	
	goto("global_label");					// Will jump to return_one() if label initialized.
	return two;						// Returns 2 always

exit_label:
	printf("Exiting program with return code, argc == %d", two);
	return two;						// Actually, returns from main(), and with argc
}

int main(int argc, char* argv[])
{	
	printf(" returns %d\n\n", return_two());  		// return_two(), will return 2	
	printf(" returns %d\n\n", return_one()); 		// return_one(), will return 1
	printf(" returns %d\n\n", return_two()); 		// Fun begins

	goto("exit_label"); //in return_two

 	// Won't get here
	return 0;
}


