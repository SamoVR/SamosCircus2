#pragma once

#include <iostream>
#include <Windows.h>
#include "Bank.h"
#include "ATM.h"
#include "User.h"

class Transaction
{
public:
	bool authorized(float amount,bool insert, User& user, Bank& bank, ATM& atm);
	void withdraw(float amount, User& user, ATM& atm);
	void deposit(float amount, User& user, ATM& atm);

private:

};

