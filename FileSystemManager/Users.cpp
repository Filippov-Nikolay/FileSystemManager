#include "Users.h"

unsigned int Users::id = 0U;

Users::Users() {
	firstName = "";
	lastName = "";
	login = "";
	password = "";
	firstEntry = true;
	access = false;
	logUs->CreateLogUs();
}

Users::Users(LogUs* ls, string fn, string ln, string lg, string ps, bool fe, bool a) {
	logUs = ls;
	firstName = fn;
	lastName = ln;
	login = lg;
	password = ps;
	firstEntry = fe;
	access = a;
}
