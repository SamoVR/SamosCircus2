#pragma once

#include <iostream>
#include "Bank.h"
#include "ATM.h"
#include "User.h"

class Transaction
{
public:
	bool authorized(float amount, User user, Bank bank, ATM atm);
	void withdraw(float amount, User user, Bank bank, ATM atm);

private:

};

