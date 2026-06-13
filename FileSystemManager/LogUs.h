#pragma once

#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

class LogUs {
public:
	void CreateLogUs() const;
	void InputLogUsRegistration(int, string, string, string, string, bool, bool) const;
	void InputLogUsAuthorization(string, string, string, bool, bool) const;
	void Encryption(string&);


	bool SearchFirstName(string) const;
	bool SearchLastName(string) const;
	bool SearchLogin(string) const;
	bool SearchPassword(string) const;
	bool Authorization(string, string) const;


	bool GetCheckingId(int) const;

	string GetFirstName(string, string) const;
	string GetLastName(string, string) const;
	string GetLogin(string, string) const;
	string GetPassword(string, string) const;

	bool GetFirstEntry(string, string) const;
	bool GetAccess(string, string) const;
};