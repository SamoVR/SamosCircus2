#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "Bank.h"

class User
{
public:
	User();
	~User();

	std::vector<std::string> history;
	std::string name;
	Bank bank;
	bool accountBlocked = false;
	float balance = 0.0f;
	float wallet = 0.0f;
	int pinCode = 0000;


private:

};

