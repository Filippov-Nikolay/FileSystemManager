#include "User.h"

User::User() {
	id++;
}

User::User(LogUs* ls, string fn, string ln, string lg, string ps, bool fe, bool a) :
	Users(ls, fn, ln, lg, ps, fe, a) {
}

void User::Input() {
	// Вход или регистрация

	string tempLog;
	string tempPass;

	int userChoice;

	do {
		cout << "Вход - 1. Регистрация - 2\n> ";
		cin >> userChoice;

		if (userChoice == 2 && logUs->GetCheckingId(id)) {
			cout << "Вы пытаетесь зарегистрироваться под зарегистрированным объектом!" << endl << endl;
		}
	} while (userChoice == 2 && logUs->GetCheckingId(id));
	

	if (userChoice == 1) {
		cout << "/* ВХОД */" << endl << endl;

		do {
			do {
				cout << "Введите логин: ";
				cin >> tempLog;

				if (!logUs->SearchLogin(tempLog)) {
					cout << "Такого логина нет!\nНапишите: reg - для регистрации" << endl << endl;
				}
			} while (!logUs->SearchLogin(tempLog));

			cout << "Введите пароль: ";
			cin >> tempPass;
			logUs->Encryption(tempPass);

			if (!logUs->Authorization(tempLog, tempPass))
				cout << "Введён неверный логин или пароль!" << endl << endl;
		} while (!logUs->Authorization(tempLog, tempPass));


		// Получить из файла данные
		firstName = logUs->GetFirstName(tempLog, tempPass);
		lastName = logUs->GetLastName(tempLog, tempPass);
		login = logUs->GetLogin(tempLog, tempPass);
		password = logUs->GetPassword(tempLog, tempPass);

		firstEntry = true;
		access = true;

		logUs->InputLogUsAuthorization(firstName, lastName, login, firstEntry, access);
		cout << endl;

		cout << "/* ДОБРО ПОЖАЛОВАТЬ */" << endl << endl;
	}
	else if (userChoice == 2) {
		cout << "/* РЕГИСТРАЦИЯ */" << endl << endl;

		cout << "Введите имя: ";
		cin >> firstName;

		cout << "Введите фамилию: ";
		cin >> lastName;

		do {
			cout << "Создайте логин: ";
			cin >> login;

			if (logUs->SearchLogin(login))
				cout << "Такой логин есть! Придумайте другой" << endl << endl;

		} while (logUs->SearchLogin(login));

		cout << "Создайте пароль: ";
		cin >> password;

		logUs->Encryption(password);

		access = true;
		firstEntry = false;

		logUs->InputLogUsRegistration(id, firstName, lastName, login, password, firstEntry, access);
	}
}

void User::Print() const {
	cout << "Имя: " << firstName << endl;
	cout << "Фамилия: " << lastName << endl;
	cout << "Логин: " << login << endl;
	cout << "Пароль: " << password << endl;
}


string User::GetFirstName() const { return firstName; }
string User::GetLastName() const { return lastName; }
string User::GetLogin() const { return login; }
string User::GetPassword() const { return password; }

int User::GetId() const { return id; }

bool User::GetFirstEntry() const { return firstEntry; }
bool User::GetAccess() const { return access; }


void User::SetFirstName(string fn) { firstName = fn; }
void User::SetLastName(string ln) { lastName = ln; }
void User::SetLogin(string lg) { login = lg; }
void User::SetPassword(string ps) {
	logUs->Encryption(ps);
	password = ps;
}

void User::SetFirstEntry(bool fe) { firstEntry = fe; }
void User::SetAccess(bool a) { access = a; }