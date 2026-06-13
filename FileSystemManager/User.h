#pragma once
#include "Users.h"

class User : public Users {
public:
	User();
	User(LogUs* ls, string fn, string ln, string lg, string ps, bool fe, bool a);

	void Input();
	void Print() const;


	// Аксессоры
	// Геттеры
	string GetFirstName() const;
	string GetLastName() const;
	string GetLogin() const;
	string GetPassword() const;

	int GetId() const;

	bool GetFirstEntry() const;
	bool GetAccess() const;


	// Сеттеры
	void SetFirstName(string);
	void SetLastName(string);
	void SetLogin(string);
	void SetPassword(string);

	void SetFirstEntry(bool);
	void SetAccess(bool);
};

