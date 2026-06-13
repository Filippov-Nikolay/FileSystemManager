#define _CRT_SECURE_NO_WARNINGS
#include "LogUs.h"

void LogUs::CreateLogUs() const { ofstream out("LogUs.txt", ios::app); }

void LogUs::InputLogUsRegistration(int id, string firstName, string lastName, string login, string password, bool firstEntry, bool access) const {
	ofstream out("LogUs.txt", ios::app);

	time_t now = time(0);
	char* dt = ctime(&now);
	size_t len = strlen(dt);
	dt[len - 1] = '\0';

	string data = dt;

	if (out.is_open()) {
		out << "[ " << id << " | " << data << " | " << firstName << " | " << lastName <<
			" | " << login << " | " << password << " | " << (firstEntry ? "signUp" : "signIn") << " | " << (access ? "allowed" : "denied") << " ]" << endl;

		out.close();
	}
	else
		cout << "Файл не удалось открыть! (InputLogUsRegistration)" << endl;
}

void LogUs::InputLogUsAuthorization(string firstName, string lastName, string login, bool firstEntry, bool access) const {
	ofstream out("LogUs.txt", ios::app);

	time_t now = time(0);
	char* dt = ctime(&now);
	size_t len = strlen(dt);
	dt[len - 1] = '\0';

	string data = dt;

	if (out.is_open()) {
		out << "[ " << data << " | " << firstName << " | " << lastName << " | " << login << " | " 
			<< (firstEntry ? "signUp" : "signIn") << " | " << (access ? "allowed" : "denied") << " ]" << endl;

		out.close();
	}
	else
		cout << "Файл не удалось открыть! (InputLogUsAuthorization)" << endl;
}

void LogUs::Encryption(string& password) {
	// Шифровка пароля

	string symbolLowCase = "abcdefghijklmnopqrstuvwxyz";
	string symbolUpCase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < password.length(); i++) {
		for (int k = 0; k < symbolLowCase.length(); k++) {
			if (password[i] == symbolLowCase[k]) {
				password[i] = symbolLowCase[k + 1];
				break;
			}
			else if (password[i] == 'z') 
				password[i] = symbolLowCase[0];
		}
	}
}


bool LogUs::SearchFirstName(string firstName) const {
	ifstream in("LogUs.txt");
	string file;

	if (in.is_open()) {
		while (in >> file)
			if (firstName == file)
				return true;
	}
	else
		cout << "Файл не удалось открыть! (SearchFirstName)" << endl;

	return false;
}
bool LogUs::SearchLastName(string lastName) const {
	ifstream in("LogUs.txt");
	string file;

	if (in.is_open()) {
		while (in >> file)
			if (lastName == file)
				return true;
	}
	else
		cout << "Файл не удалось открыть! (SearchLastName)" << endl;

	return false;
}
bool LogUs::SearchLogin(string login) const {
	ifstream in("LogUs.txt");
	string file;

	if (in.is_open()) {
		while (in >> file)
			if (login == file)
				return true;
	}
	else
		cout << "Файл не удалось открыть! (SearchLogin)" << endl;

	return false;
}
bool LogUs::SearchPassword(string password) const {
	ifstream in("LogUs.txt");
	string file;

	if (in.is_open()) {
		while (in >> file)
			if (password == file)
				return true;
	}
	else
		cout << "Файл не удалось открыть! (SearchLogin)" << endl;

	return false;
}
bool LogUs::Authorization(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++) 
			if (buff[i] == '|') 
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password) 
			return true;
		
		for (int i = 0; i < index; i++)
			temp[i] = "";
		
		index = 0;
	} while (in);

	return false;
}


bool LogUs::GetCheckingId(int id) const {
	// cout << "GET ID" << endl << endl;

	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '[')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == '|') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		
		//cout << temp[0] << endl;
		//cout << id << endl;

		//log = 0
		//id = 0

		if (atoi(temp[0].c_str()) == 0 && atoi(temp[0].c_str()) == id)
			return false;

		if (atoi(temp[0].c_str()) == id)
			return true;
		

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);

	return false;
}

string LogUs::GetFirstName(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password)
			return temp[1];

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
string LogUs::GetLastName(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password)
			return temp[2];

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
string LogUs::GetLogin(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password)
			return temp[3];

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
string LogUs::GetPassword(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password)
			return temp[4];

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
bool LogUs::GetFirstEntry(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password) {
			if (temp[5] == "signIn")
				return false;
			else
				return true;
		}
			

		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
bool LogUs::GetAccess(string login, string password) const {
	ifstream in("LogUs.txt");

	string temp[8];
	int index = 0;

	do {
		char buff[300]{};

		in.getline(buff, 300);

		for (int i = 0; i < strlen(buff); i++)
			if (buff[i] == '|')
				for (int j = (i + 2); j < strlen(buff); j++) {
					if (buff[j] == ' ') {
						index++;
						break;
					}
					temp[index] += buff[j];
				}

		if (temp[3] == login && temp[4] == password) {
			if (temp[6] == "allowed")
				return true;
			else
				return false;
		}


		for (int i = 0; i < index; i++)
			temp[i] = "";

		index = 0;
	} while (in);
}
