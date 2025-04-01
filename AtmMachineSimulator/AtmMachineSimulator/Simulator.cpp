#include "Simulator.h"

#define RED		"\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

Simulator::Simulator()
{
	std::cout << MAGENTA << "[DEBUG] > Simulator initialized" << RESET << std::endl;
}

Simulator::~Simulator()
{

}

void Simulator::withdraw(User& user, Bank& bank, ATM& atm)
{
	float amount;
	std::cout << std::endl << CYAN << "Enter the amount you wish to withdraw: " << RESET;
	std::cin >> amount;
	Transaction transaction;
	if (transaction.authorized(amount,false,user,bank,atm))
	{
		transaction.withdraw(amount, user, atm);
		std::cout << GREEN << "Transaction authorized." << RESET;	
	}
	else
	{
		std::cout << RED << "Transaction unauthorized." << RESET;
	}

	Simulator::init();
}

void Simulator::deposit(User& user, Bank& bank, ATM& atm)
{
	float amount;
	std::cout << std::endl << CYAN << "Enter the amount you wish to deposit: " << RESET;
	std::cin >> amount;
	Transaction transaction;
	if (transaction.authorized(amount,true, user, bank, atm))
	{
		transaction.deposit(amount, user, atm);
		std::cout << GREEN << "Transaction authorized." << RESET;
	}
	else
	{
		std::cout << RED << "Transaction unauthorized." << RESET;
	}

	Simulator::init();
}

void Simulator::transhistory(User& user)
{
	if (user.history.empty())
		std::cout << CYAN << "User history is empty." << RESET << std::endl;
	else
	{
		std::cout << std::endl << CYAN << "User history for " << RESET << user.name << CYAN << ":\n" << RESET;
		for (std::string& transaction : user.history) {
			std::cout << "> " << transaction << std::endl;
		}
	}

	Simulator::init();
}

void Simulator::transfer(User& sender)
{
	User receiver;
	float amount;

	Simulator::init();
}

void Simulator::test(User& user, Bank& bank, ATM& atm)
{
	Transaction transaction;
	transaction.addMoney(1000.0f, user);
	Simulator::init();
}

void Simulator::exit()
{
	system("cls");
	//
}

void Simulator::start()
{
	bank.name = "Tatra banka";
	bank.atmFee = 2.0f;

	atm.bank = bank;
	atm.balance = 10000.0f;

	user.name = "Ado Hornak";
	user.bank = bank;
	user.balance = 2000.0f;
	user.accountBlocked = false;
	user.pinCode = 1234;

	Simulator::init();

}

void Simulator::init()
{
	std::cout << std::endl << "Presse ENTER to continue." << std::endl;
	std::cin.ignore();
	std::cin.ignore();
	system("cls");

	Simulator::info();
	Simulator::menu();

}

void Simulator::info()
{
	std::cout << std::endl << CYAN << "=======================" << RESET;
	std::cout << std::endl << YELLOW << " ATM Machine Simulator" << RESET << std::endl;
	std::cout << CYAN << "=======================" << RESET << std::endl;

	std::cout << std::endl << CYAN << "-- User Information --" << RESET << std::endl;
	std::cout << "User Name: " << user.name << std::endl;
	std::cout << "User Bank: " << user.bank.name << std::endl;
	std::cout << "User Balance: " << user.balance << std::endl;
	std::cout << "User Wallet: " << user.wallet << std::endl;
	std::cout << "User Blocked: ";
	if (user.accountBlocked)
		 std::cout << "true" << std::endl;
	else
		std::cout << "false" << std::endl;

	std::cout << std::endl << CYAN << "-- ATM Information --" << RESET << std::endl;
	std::cout << "ATM Bank: " << atm.bank.name << std::endl;
	std::cout << "ATM Balance: " << atm.balance << std::endl;

	std::cout << std::endl << CYAN << "-- Users Bank Information-- " << RESET << std::endl;
	std::cout << "Bank Name: " << user.bank.name << std::endl;
	std::cout << "Bank Balance: " << user.bank.balance << std::endl;

}

void Simulator::menu()
{
	int choice = 0;

	std::cout << std::endl << CYAN << "==================" << RESET;
	std::cout << std::endl << YELLOW << " Choose an option" << RESET << std::endl;
	std::cout << CYAN << "==================" << RESET << std::endl;

	std::cout << std::endl << "[1] > Withdraw Money";
	std::cout << std::endl << "[2] > Insert Money";
	std::cout << std::endl << "[3] > View Transaction History";
	std::cout << std::endl << "[4] > Transfer Money";
	std::cout << std::endl << "[6] > Exit";
	std::cout << std::endl;

	std::cin >> choice;
	
	if (choice == 1)
		withdraw(user, bank, atm);
	else if (choice == 2)
		deposit(user, bank, atm);
	else if (choice == 3)
		transhistory(user);
	else if (choice == 4)
		transfer(user);
	else if (choice == 7)
		test(user,bank,atm);
	else if (choice == 6)
		exit();
	else
	{
		Simulator::init();

	}

}

