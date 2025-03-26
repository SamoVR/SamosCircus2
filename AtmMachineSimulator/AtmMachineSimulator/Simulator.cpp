#include "Simulator.h"

Simulator::Simulator()
{
	std::cout << "[DEBUG] > Simulator initialized" << std::endl;
}

Simulator::~Simulator()
{

}


void Simulator::withdraw(User user, Bank bank, ATM atm)
{
	float amount;
	std::cout << std::endl << "Enter an amount you wish to withdraw: ";
	std::cin >> amount;
	Transaction transaction;
	if (transaction.authorized(amount,user,bank,atm))
	{
		transaction.withdraw(amount, user, bank, atm);
	}
	else
		std::cout << "Transaction is not authorized, press ENTER to continue.";

	std::cout << "Transaction authorized, press ENTER to continue.";
	std::cin.ignore();
	std::cin.ignore();

	system("cls");
	Simulator::info();
	Simulator::menu();
}

void Simulator::insert(User user, Bank bank, ATM atm)
{

}

void Simulator::exit()
{

}

void Simulator::start()
{
	bank.name = "TatraBanka";

	atm.bank = bank;
	atm.balance = 10000.0f;

	user.name = "Ado Hornak";
	user.bank = bank;
	user.balance = 200.0f;

	Simulator::init();

}

void Simulator::init()
{
	std::cout << "Presse ENTER to continue";
	std::cin.ignore();
	system("cls");

	Simulator::info();
	Simulator::menu();

}

void Simulator::info()
{
	std::cout << std::endl << "=======================";
	std::cout << std::endl << " ATM Machine Simulator" << std::endl;
	std::cout << "=======================" << std::endl;

	std::cout << std::endl << "-- User Information --" << std::endl;
	std::cout << "User Name: " << user.name << std::endl;
	std::cout << "User Bank: " << user.bank.name << std::endl;
	std::cout << "User Balance: " << user.balance << std::endl;

	std::cout << std::endl << "-- ATM Information --" << std::endl;
	std::cout << "ATM Bank: " << atm.bank.name << std::endl;
	std::cout << "ATM Balance: " << atm.balance << std::endl;

	std::cout << std::endl << "-- Bank Information-- " << std::endl;
	std::cout << "Bank Name: " << bank.name << std::endl;
	std::cout << "Bank Balance: " << bank.balance << std::endl;

}

void Simulator::menu()
{
	int choice = 0;

	std::cout << std::endl << "================================" << std::endl;

	std::cout << std::endl << "==================";
	std::cout << std::endl << " Choose an option" << std::endl;
	std::cout << "==================" << std::endl;

	std::cout << std::endl << "[1] > Withdraw Money";
	std::cout << std::endl << "[2] > Insert Money";
	std::cout << std::endl << "[3] > Exit";
	std::cout << std::endl;

	std::cin >> choice;
	
	if (choice == 1)
		withdraw(user,bank,atm);
	else if (choice == 2)
		insert(user,bank,atm);
	else if (choice == 3)
		exit();
	else
	{
		system("cls");
		Simulator::info();
		Simulator::menu();

	}

}

