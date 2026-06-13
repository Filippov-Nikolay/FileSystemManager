#include <iostream>
#include <Windows.h>
#include <conio.h> // Для пароля

#include "Log.h"
#include "User.h"
#include "FileManager.h"


using namespace std;

/*
    1. Исправить запись в лог, при рег. или автор.
    2. Сделать шифрование паролей (Большие буквы и цифры)

    Создать как коммандая строка?

    crdr - create dir
    rndr - rename dir
    lsdr - list dr
    szdr - size dir

    crf - create file
    rnf - rename file
    szf - size file

    cin >> command >> from >> where;

    login - уникальный, можно не писать ФИО

    Добавить директорию по умолчанию?
*/

int main() {
    setlocale(0, "");
    system("chcp 1251");
    SetConsoleCP(1251);

    /*int c = 0;
    string pwd;
    cout << "Enter password: ";

    while (true)
    {
        c = _getch();
        if (c == 0)
            c = _getch();
        if (c == '\r' || c == '\n')
            break;
        cout << '*';
        pwd += c;
    }

    cout << "\nYou entered: " << pwd.c_str();
    return 0;*/

    Log log;

    Users* user = new User;
    user->Input();
    user->Input();

    FileManager test(user, &log);
    test.InputCommand();

    return 0;
}