#pragma once

#include <iostream>
#include <string>
#include "Bank.h"

class ATM
{
	public:
		ATM();
		~ATM();
		Bank bank;
		float balance = 10000.0f; //10k default

	private:


};

