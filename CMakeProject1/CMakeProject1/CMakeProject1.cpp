// CMakeProject1.cpp : Defines the entry point for the application.
//

#include "CMakeProject1.h"

class Animal {
public:
	virtual ~Animal() = default;
	virtual void speak() const = 0;
	virtual void sleep() const { std::cout << "zzz\n"; }
};

class Dog : public Animal {
public:
	void speak() const override { std::cout << "Woof!\n"; }
};

class Cat : public Animal {
public:
	void speak() const override { std::cout << "Meow!\n"; }
	void sleep() const override { std::cout << "cat sleeps 16h\n"; }
};

class Dawg : public Animal {};

int main()
{
	std::vector < std::unique_ptr<Animal>> animals;
	animals.push_back(std::make_unique<Dog>());
	animals.push_back(std::make_unique<Cat>());
	animals.push_back(std::make_unique<Dawg>());

	for (auto& a : animals) {
		a->speak();
		a->sleep();
	}
}
