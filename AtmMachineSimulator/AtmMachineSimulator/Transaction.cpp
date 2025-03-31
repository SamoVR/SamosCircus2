#include "Transaction.h"

bool Transaction::authorized(float amount, bool insert, User& user, Bank& bank, ATM& atm)
{
	if (!insert)
	{
		if (bank.balance >= amount)
			if (atm.balance >= amount)
				if (!user.accountBlocked)
					if (user.balance >= amount)
						if(amount > 0)
							return true;
	}
	else
	{
		if (!user.accountBlocked)
			if (user.wallet >= amount)
				if(amount > 0)
					return true;
	}
	return false;
}

void Transaction::withdraw(float amount, User& user, ATM& atm)
{
	user.bank.balance -= amount;
	atm.balance -= amount;
	user.balance -= amount;
	if (user.bank.name != atm.bank.name)
	{
		user.wallet += (amount - user.bank.atmFee);
		user.history.push_back("\033[1;31m [-] " + std::to_string(user.bank.atmFee) + "\033[0m");
	}
	else
		user.wallet += amount;

	user.history.push_back("\033[1;31m [-] " + std::to_string(amount) + "\033[0m");
}

void Transaction::deposit(float amount, User& user, ATM& atm)
{
	user.wallet -= amount;
	user.balance += amount;
	atm.balance += amount;
	user.bank.balance += amount;

	user.history.push_back("\033[1;32m [+] " + std::to_string(amount) + "\033[0m");
}

void Transaction::transfer(float amount, User& sender, User& receiver)
{

}

void Transaction::addMoney(float amount, User& user)
{
	user.balance += amount;
}
