#include "FileManager.h"

#include <direct.h>
#include <stdio.h>
#include <sstream>
// io
// #include <io.h>

/*
    Добавить путь
    изменить путь
    сохранить путь
    првоерка тек. директории
    и тд
*/

/*
    1. Исправить запись в лог, при рег. или автор.
    2. Сделать шифрование паролей (Большие буквы и цифры)

    Создать как коммандая строка?

    crdr - create dir
    rndr - rename dir
    cpdr - copy dir
    lsdr - list dir
    szdr - size dir

    crf - create file
    rnf - rename file
    szf - size file

    clfm - close file manager

    cin >> command >> from >> where;

    ПЕРЕСМОТРЕТЬ ЦИКЛ В LOGUS
*/

FileManager::FileManager() { command = from = where = nullptr; access = false; }

FileManager::FileManager(Users* u, Log* l) { users = u; log = l; }

FileManager::~FileManager() {
    delete command;
    delete from;
    delete where;
}

void FileManager::InputCommand() {
    if (!users->GetAccess()) {
        cout << "\t\t-----------------------" << endl;
        cout << "\t\t/*В доступе отказано!*/" << endl;
        cout << "\t\t-----------------------" << endl;
        return;
    }

    cout << "\t\t-----------------------" << endl;
    cout << "\t\t/* Доступе разрешён!* /" << endl;
    cout << "\t\t-----------------------" << endl;

    access = true;

    string userInput = "";
    string commandUser = "";
    string fromUser = "";
    string whereUser = "";

    cin.ignore();

    while (true) {
        cout << "> ";
        getline(cin, userInput);

        if (userInput == "clfm" || userInput == "close" || userInput == "exit")
            break;

        istringstream iss(userInput);
        iss >> commandUser >> fromUser >> whereUser;

        if (commandUser != "")
            CommandDefinition(commandUser, fromUser, whereUser);

        commandUser = "";
        fromUser = "";
        whereUser = "";
    }
    
    cout << "\t\t---------------------" << endl;
    cout << "\t\t/*   До свидания!  */" << endl;
    cout << "\t\t---------------------" << endl;
}

void FileManager::CommandDefinition(string commandUser, string fromUser, string whereUser) {
    if (!access)
        return;
    
    if (commandUser == "help" || commandUser == "h") {
        cout << "\t--------------------------------------------------------------------------------" << endl;
        cout << "\t---------------------------------ДИРЕКТОРИИ-------------------------------------" << endl;
        cout << "\tcrdr - создаёт директорию: crdr путь/название_папки" << endl << endl;
        cout << "\trndr - переименовывает директорию: rndr путь_к_папке/название_папки новое_название" << endl << endl;
        cout << "\tcpdr - копирует директорию: cpdr путь_к_папке\\название_папки путь_к_папке\\" << endl << endl;
        cout << "\tmvdr - перенести директорию: mvdr путь_к_папке\\название_папки путь\\" << endl << endl;
        cout << "\tlsdr - список файлов в каталоге: lsdr путь\\ - без указания пути, покажет тек. список" << endl << endl;
        cout << "\tszdr - размер директории: szdr путь\\" << endl << endl;
        cout << "\t------------------------------------ФАЙЛЫ---------------------------------------" << endl;
        cout << "\tcrf - создаёт файл: crf путь/название_файла.расширение" << endl << endl;
        cout << "\trnf - переименовывает файл: trnf путь_к_папке/название_файла.расширение новое_название.расширение" << endl << endl;
        cout << "\tcpf - копирует файл: cpf путь/название_файла.расширение путь_к_папке/" << endl << endl;
        cout << "\tmvf - перености файл: mvf путь/название_файла.расширение путь_к_папке/" << endl << endl;
        cout << "\tszf - размер файла: szf путь/" << endl << endl;
        cout << "\t------------------------------------ОБЩЕЕ---------------------------------------" << endl;
        cout << "\tshmsk - поиск по маске: shmsk путь_к_папке\\папка маска" << endl << endl;
        cout << "\tclear/cls/clr - очистить консоль" << endl << endl;
        cout << "\tclfm/exit/close - закрыть файловый менеджер" << endl;
        cout << "\t--------------------------------------------------------------------------------" << endl;
    }
    else if (commandUser == "crdr") {
        int lenghtFrom = (fromUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        CreateAFolder();
    }
    else if (commandUser == "rndr") {
        int lenghtFrom = (fromUser.length()) + 1;
        int tempIndex = 0;
        int index = 0;

        string temp;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';


        for (int i = 0; i < lenghtFrom; i++) {
            if (fromUser[i] == '/')
                tempIndex++; // 2
        }

        for (int i = 0, j = 0; i < lenghtFrom; i++) {
            if (fromUser[i] != '/')
                temp += fromUser[i];
            else {
                temp += fromUser[i];
                index++;
            }

            if (index == tempIndex)
                break;
        }

        whereUser = temp += whereUser;

        int lenghtWhere = (whereUser.length()) + 1;
        
        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';
        
        RenameFolder();
    }
    else if (commandUser == "cpdr") {
        // Проверка на путь или название
        // D:/t D:/test (скопировать папку t, в папку test)

        if (fromUser == whereUser) { // D:/t D:/t
            cout << "Невозможно скопировать файл с таким же названием и путём!" << endl << endl;
            return;
        }

        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        CopyFolder();
    }
    else if (commandUser == "mvdr") {
        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        MoveFolder();
    }
    else if (commandUser == "lsdr") {
        string temp = fromUser;
        fromUser = "dir ";
        fromUser += temp;

        cout << "TEST^ " << fromUser << endl;

        int lenghtFrom = (fromUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        ShowDiskContents();
    }
    else if (commandUser == "szdr") {
        int lenghtFrom = (fromUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        CalculateSizeFolder();
    }
    else if (commandUser == "crf") {
        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        CreateFile();
    }
    else if (commandUser == "rnf") {
        int lenghtFrom = (fromUser.length()) + 1;
        int tempIndex = 0;
        int index = 0;

        string temp;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';


        for (int i = 0; i < lenghtFrom; i++) {
            if (fromUser[i] == '/')
                tempIndex++; // 2
        }

        for (int i = 0, j = 0; i < lenghtFrom; i++) {
            if (fromUser[i] != '/')
                temp += fromUser[i];
            else {
                temp += fromUser[i];
                index++;
            }

            if (index == tempIndex)
                break;
        }

        whereUser = temp += whereUser;

        int lenghtWhere = (whereUser.length()) + 1;

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        RenameFile();
    }
    else if (commandUser == "cpf") {
        // Проверка на путь или название
        // D:/t D:/test (скопировать папку t, в папку test)

        if (fromUser == whereUser) { // D:/t D:/t
            cout << "Невозможно скопировать файл с таким же названием и путём!" << endl << endl;
            return;
        }

        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        CopyFile();
    }
    else if (commandUser == "mvf") {
        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        MoveFile();
    }
    else if (commandUser == "szf") {
        int lenghtFrom = (fromUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        CalculateSizeFile();
    }
    else if (commandUser == "shmsk") {
        int lenghtFrom = (fromUser.length()) + 1;
        int lenghtWhere = (whereUser.length()) + 1;

        from = new char[lenghtFrom];
        strcpy_s(from, lenghtFrom, fromUser.c_str());
        from[lenghtFrom - 1] = '\0';

        where = new char[lenghtWhere];
        strcpy_s(where, lenghtWhere, whereUser.c_str());
        where[lenghtWhere - 1] = '\0';

        SearchByMask();
    }
    else if (commandUser == "clear" || commandUser == "cls" || commandUser == "clr") {
        system("cls");
    }

    log->InputLog(users->GetLogin(), commandUser);
}

void FileManager::ShowDiskContents() const { // Показать список каталога
    if (!access)
        return;

    system(from);
}
void FileManager::CreateAFolder() const { // Создать директорию
    if (!access)
        return;

    if (_mkdir(from) == 0)
        cout << "Директория создана!" << endl;
    else
        cout << "Ошибка создания директории или такая директория уже есть!" << endl;
}
void FileManager::CreateFile() const { // Создаёт файл
    if (!access)
        return;

    ofstream out(from);

    if (out.is_open()) {
        // Если в where ничего не передали
        // создаётся пустой файл
        if (where != "")
            out << where;

        out.close();

        cout << "Файл создан!" << endl;
    }
    else
        cout << "Не удалось открыть файл!" << endl;
}
void FileManager::RenameFolder() const { // Переименовывает каталог
    if (!access)
        return;

    if (rename(from, where) == 0)
        cout << "Директория переименована!" << endl;
    else
        cout << "Ошибка переименования директории!" << endl;
}
void FileManager::RenameFile() const { // Переименовать файл
    if (!access)
        return;

    if (rename(from, where) == 0)
        cout << "Файл переименован!" << endl;
    else
        cout << "Ошибка переименования файла!" << endl;
}
void FileManager::CopyFolder() { // Копировать директорию с файлами
    // Получить кол-во файлов
    // название папки
    // папка от куда и куда (полный путь)
    // создать папку
    // помещать туда файлы
    // тоже самое что и с размером папки
    // // cpdr D:\Folder D:\t
    // D:\Folder D:\t

    if (!access)
        return;

    int rez = _mkdir(where);
    cout << rez << endl;

    if (rez == -1) {
        cout << "Такая папка уже существует!" << endl << endl;
        return;
    }

    // 0 = create
    // -1 = error

    string tempFrom = from;
    string tempWhere = where;
    string tempLastName = "";

    int temp = 0;
    int tempIndex = 0;

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '\\')
            temp++;
    }

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '\\')
            tempIndex++;
        if (temp == tempIndex)
            tempLastName += tempFrom[i];
    }

    cout << "LastPath: " << tempLastName << endl;

    // cpdr D:/test/hello D:/test/hello/world // hello

    tempWhere += tempLastName; // D:/test/hello/world/hello

    cout << "CREATE FOLDER: " << tempWhere << endl;

    delete where;

    where = new char[(tempWhere.length()) + 1];
    strcpy_s(where, (tempWhere.length()) + 1, tempWhere.c_str());

    if(_mkdir(where) == 0)
        cout << "Папка создана!" << endl;
    else
        cout << "Ошибка создания директории или такая директория уже есть!" << endl;


    // cpdr D:\Folder D:\t
    // теперь нужно работать с from и tempWhere

    string s = "dir " + tempFrom + " /B >> " + tempFrom + "\\tempFile.txt";

    cout << s << endl;

    system(s.c_str());


    // Массив который будет хранить названия файлов

    cout << "tempFrom: " << tempFrom << endl;
    cout << "tempWhere: " << tempWhere << endl;
    cout << "tempLastName: " << tempLastName << endl;

    // Открываю tempFile.txt
    ifstream inTemp((tempFrom + "\\tempFile.txt").c_str());
    string tempFile;

    do {
        // Получаю названия других файлов
        char buff[90]{};
        inTemp.getline(buff, 90);

        for (int i = 0; i < strlen(buff); i++) {
            if (buff[i] != '\n')
                tempFile += buff[i];
            else
                break;
        }
        
        if (tempFile != "tempFile.txt") {
            ifstream in((tempFrom + "\\" + tempFile).c_str(), ios::in);
            ofstream out((tempWhere + "\\" + tempFile).c_str(), ios::out | ios::trunc);

            do {
                char character = '\0';
                in.get(character);
                out.put(character);
            } while (in);

            in.close();
            out.close();
        }
        
        tempFile = "";
    } while (inTemp);

    inTemp.close();

    remove((tempFrom + "\\tempFile.txt").c_str());
}
void FileManager::CopyFile() const { // Копировать файл
    // cpf D:/Folder/file.txt D:/

    if (!access)
        return;

    string tempFrom = from;
    string tempWhere = where;
    string tempLastName = "";

    int temp = 0;
    int tempIndex = 0;

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '/')
            temp++;
    }

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '/')
            tempIndex++;
        if (temp == tempIndex)
            tempLastName += tempFrom[i + 1];
    }

    cout << "LastPath: " << tempLastName << endl;

    ifstream in;
    in.open(from, ios::in);

    ofstream out;
    out.open((tempWhere + tempLastName).c_str(), ios::out | ios::trunc);
    
    if (in) {
        do {
            char character = '\0';

            in.get(character);
            out.put(character);
        } while (in);
        
        out.close();
        in.close();

        cout << "Файл скопирован!" << endl;
    } 
    else 
        cout << "Не удалось открыть файл!" << endl;
}
void FileManager::MoveFolder() { // Переносит директорию
    // Получить исходную папку
    // Куда переместить
    // mvdr D:\Folder D:\t_move

    if (!access)
        return;

    int rez = _mkdir(where);
    cout << rez << endl;

    if (rez == -1) {
        cout << "Такая папка уже существует!" << endl << endl;
        return;
    }

    string tempFrom = from;
    string tempWhere = where;
    string tempLastName = "";

    int temp = 0;
    int tempIndex = 0;

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '\\')
            temp++;
    }

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '\\')
            tempIndex++;
        if (temp == tempIndex)
            tempLastName += tempFrom[i];
    }

    cout << "LastPath: " << tempLastName << endl;

    // cpdr D:/test/hello D:/test/hello/world // hello

    tempWhere += tempLastName; // D:/test/hello/world/hello

    cout << "CREATE FOLDER: " << tempWhere << endl;

    delete where;

    where = new char[(tempWhere.length()) + 1];
    strcpy_s(where, (tempWhere.length()) + 1, tempWhere.c_str());

    if (_mkdir(where) == 0)
        cout << "Папка создана!" << endl;
    else
        cout << "Ошибка создания директории или такая директория уже есть!" << endl;


    // cpdr D:\Folder D:\t
    // теперь нужно работать с from и tempWhere

    string s = "dir " + tempFrom + " /B >> " + tempFrom + "\\tempFile.txt";

    cout << s << endl;

    system(s.c_str());


    // Массив который будет хранить названия файлов

    cout << "tempFrom: " << tempFrom << endl;
    cout << "tempWhere: " << tempWhere << endl;
    cout << "tempLastName: " << tempLastName << endl;

    // Открываю tempFile.txt
    ifstream inTemp((tempFrom + "\\tempFile.txt").c_str());
    string tempFile;

    do {
        // Получаю названия других файлов
        char buff[90]{};
        inTemp.getline(buff, 90);

        for (int i = 0; i < strlen(buff); i++) {
            if (buff[i] != '\n')
                tempFile += buff[i];
            else
                break;
        }

        if (tempFile != "tempFile.txt") {
            ifstream in((tempFrom + "\\" + tempFile).c_str(), ios::in);
            ofstream out((tempWhere + "\\" + tempFile).c_str(), ios::out | ios::trunc);

            do {
                char character = '\0';
                in.get(character);
                out.put(character);
            } while (in);

            in.close();
            out.close();

            remove((tempFrom + "\\" + tempFile).c_str());
        }

        tempFile = "";
    } while (inTemp);

    inTemp.close();

    remove((tempFrom + "\\tempFile.txt").c_str());
    _rmdir((tempFrom).c_str());

    cout << "Директория перемещена" << endl << endl;
}
void FileManager::MoveFile() { // Перемещение файла
    // mvf D:/Folder/file.txt D:/

    if (!access)
        return;

    string tempFrom = from;
    string tempWhere = where;
    string tempLastName = "";

    int temp = 0;
    int tempIndex = 0;

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '/')
            temp++;
    }

    for (int i = 0; i < tempFrom.length(); i++) {
        if (tempFrom[i] == '/')
            tempIndex++;
        if (temp == tempIndex)
            tempLastName += tempFrom[i + 1];
    }

    cout << "LastPath: " << tempLastName << endl;
    cout << "PATH: " << tempFrom << endl;

    ifstream in;
    in.open((tempFrom).c_str(), ios::in);

    ofstream out;
    out.open((tempWhere + tempLastName).c_str(), ios::out | ios::trunc);

    if (in) {
        do {
            char character = '\0';

            in.get(character);
            out.put(character);
        } while (in);

        out.close();
        in.close();

        remove((tempFrom).c_str());

        cout << "Файл перемещён!" << endl;
    }
    else
        cout << "Не удалось открыть файл!" << endl;
}
void FileManager::CalculateSizeFolder() const { // Показывает размер директории
    // Размер всех файлов
    // Алгоритм перебора
    // Или узнать через консоль?

    if (!access)
        return;

    string tempFrom = from;
    string s = "dir " + tempFrom + " /B >> " + tempFrom + "\\tempFile.txt";
    cout << s << endl;

    system(s.c_str());

    ifstream inTemp((tempFrom + "\\tempFile.txt").c_str());
    string tempFile;

    int size = 0;

    do {
        // Получаю названия других файлов
        char buff[90]{};
        inTemp.getline(buff, 90);

        for (int i = 0; i < strlen(buff); i++) {
            if (buff[i] != '\n')
                tempFile += buff[i];
            else
                break;
        }

        ifstream in((tempFrom + "\\" + tempFile).c_str(), ios::in);

        if (in) {
            in.seekg(0, ios::end);

            size += in.tellg();

            in.close();
        }

        tempFile = "";
    } while (inTemp);

    inTemp.close();

    remove((tempFrom + "\\tempFile.txt").c_str());

    cout << "Размер директории: " << size << " Байт" << endl;
}
void FileManager::CalculateSizeFile() const { // Показывает размер файла
    if (!access)
        return;
    
    ifstream in(from, ios::binary | ios::in);

    if (in) {
        in.seekg(0, ios::end);

        cout << "Размер файла: " << in.tellg() << " Байт" << endl;

        in.close();
    } 
    else
        cout << "Не удалось открыть или найти файл!" << endl;
}
void FileManager::SearchByMask() const { // Поиск по маске
    // Через консоль?
    // Записывать рез. поиска в файл
    // Читать из файла
    // dir /b Folder\file.*
    // shmsk D:\Folder file.*
    if (!access)
        return;

    string tempFrom = from;
    string tempWhere = where;
    string tempLastName = "";
    string command = "";
   
    command = "dir /b " + tempFrom + "\\" + tempWhere;

    system(command.c_str());
}