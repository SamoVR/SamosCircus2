#pragma once

#include <iostream>
#include <string>
#include "Bank.h"

class User
{
public:
	User();
	~User();

	std::string name;
	Bank bank;
	bool accountBlocked = false;
	float balance = 0.0f;
	float wallet = 0.0f;


private:

};

