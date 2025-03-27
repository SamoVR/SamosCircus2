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
		user.wallet += (amount - user.bank.atmFee);
	else
		user.wallet += amount;
}

void Transaction::insert(float amount, User& user, ATM& atm)
{
	user.wallet -= amount;
	user.balance += amount;
	atm.balance += amount;
	bank.balance += amount;

}
