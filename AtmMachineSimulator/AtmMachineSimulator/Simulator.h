#pragma once

#include <iostream>
#include <Windows.h>

#include "Bank.h"
#include "ATM.h"
#include "User.h"
#include "Transaction.h"

class Simulator
{
public:
	Simulator();
	~Simulator();
	void start();
	void info();
	void menu();

private:
	void init();
	void withdraw(User& user, Bank& bank, ATM& atm);
	void insert(User& user, Bank& bank, ATM& atm);
	void exit();

	Bank bank;
	ATM atm;
	User user;

};

