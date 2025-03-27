#pragma once

#include <string>

class Bank
{
public:
	Bank();
	~Bank();
	std::string name;
	float balance = 100000.0f; //100k default
	float atmFee = 2.0f;

private:

};

