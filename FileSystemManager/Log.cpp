#define _CRT_SECURE_NO_WARNINGS
#include "Log.h"

Log::Log() { ofstream out("LogUs.txt", ios::app); }

void Log::InputLog(string login, string actions) {
	cout << "Log true" << endl;

	ofstream out("Log.txt", ios::app);

	time_t now = time(0);
	char* dt = ctime(&now);
	size_t len = strlen(dt);
	dt[len - 1] = '\0';

	string data = dt;

	if (out.is_open()) {
		out << "[ " << data << " | " << login << " | " << actions << " ] " << endl;

		out.close();
	}
	else
		cout << "Файл не удалось открыть! (InputLogUsRegistration)" << endl;
}
