#pragma once
#include <iostream>

#include "Users.h"
#include "Log.h"

using namespace std;

class FileManager {
	Log* log;
	Users* users;

	char* command;
	char* from;
	char* where;

	bool access;
public:
	FileManager();
	FileManager(Users*, Log*);
	~FileManager();

	void InputCommand();
	void CommandDefinition(string, string, string);

	void ShowDiskContents() const;		// Показать содержимое диска
	void CreateAFolder() const;			// Создать папку
	void CreateFile() const;			// Создать файл
	void RenameFolder() const;			// Переименовать папку
	void RenameFile() const;			// Переименовать файл
	void CopyFolder();					// Копировать папку
	void CopyFile() const;				//  Копировать файл
	void MoveFolder();					// Перемещение папки
	void MoveFile();					// Перемещение файла
	void CalculateSizeFolder() const;	// Размер папки
	void CalculateSizeFile() const;		// Размер файла
	void SearchByMask() const;			// Поиск по маске
};