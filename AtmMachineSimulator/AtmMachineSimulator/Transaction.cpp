#include "Transaction.h"

bool Transaction::authorized(float amount, User user, Bank bank, ATM atm)
{
	if (bank.balance >= amount)
		if (atm.balance >= amount)
			if (!user.accountBlocked)
				if (user.balance >= amount)
				return true;

	return false;
}

void Transaction::withdraw(float amount, User user, Bank bank, ATM atm)
{
	bank.balance -= amount;
	atm.balance -= amount;
	user.balance -= amount;
	user.wallet += amount;

	std::cout << std::endl << bank.balance;
	std::cout << std::endl << atm.balance;
	std::cout << std::endl << user.balance;
	std::cout << std::endl << user.wallet;
}