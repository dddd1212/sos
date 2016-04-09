#include "../module1/module1_funcs.h"


int module2_func() {
	// implicit:
	int x = EXP_module1_mod_func1();

	// explicit:
	int y = mod_func2(); // The function real name is EXP_module1_mod_func2

	return x + y;
}