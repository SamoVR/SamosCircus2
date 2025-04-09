#include <iostream>
#include "TestClass.h"

void cau(TestClass& test)
{
	test.changed = true;
}

int main()
{
	TestClass test;
	//test.printTest();
	std::cout << test.changed << std::endl;
	cau(test);
	std::cout << test.changed;

	return 0;
}