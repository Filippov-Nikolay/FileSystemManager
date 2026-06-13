#pragma once
#include <iostream>
#include "LogUs.h"

using namespace std;

class Users {
protected:
	LogUs* logUs;			// Лог
	string firstName;		// Имя
	string lastName;		// Фамилия
	string login;			// Логин
	string password;		// Пароль

	bool firstEntry = true; // Первый вход
	bool access;			// Доступ
public:
	static unsigned int id;

	Users();
	Users(LogUs*, string, string, string, string, bool, bool);

	virtual void Input() = 0;
	virtual void Print() const = 0;


	// Аксессоры
	// Геттеры
	virtual string GetFirstName() const = 0;
	virtual string GetLastName() const = 0;
	virtual string GetLogin() const = 0;
	virtual string GetPassword() const = 0;
	
	virtual int GetId() const = 0;

	virtual bool GetFirstEntry() const = 0;
	virtual bool GetAccess() const = 0;


	// Сеттеры
	virtual void SetFirstName(string) = 0;
	virtual void SetLastName(string) = 0;
	virtual void SetLogin(string) = 0;
	virtual void SetPassword(string) = 0;

	virtual void SetFirstEntry(bool) = 0;
	virtual void SetAccess(bool) = 0;
};