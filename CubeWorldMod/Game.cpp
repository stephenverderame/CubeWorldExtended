#include "Game.h"
GameSingleton * GameSingleton::instance = nullptr;
GameSingleton::GameSingleton()
{
	gameHwnd = FindWindow(NULL, "Cube");
	gamePid = findProcessId("Cube.exe");
	gameHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, gamePid);
	gameBaseAddr = findModuleBase(gameHandle, "Cube.exe");
	RECT r;
	GetClientRect(gameHwnd, &r);
	cube_window.length = r.right - r.left;
	cube_window.height = r.bottom - r.top;
}

GameSingleton::~GameSingleton()
{
	CloseHandle(gameHandle);
}

GameSingleton * GameSingleton::getInstance()
{
	if (instance == nullptr)
		instance = new GameSingleton();
	return instance;
}

void GameSingleton::cleanup()
{
	delete instance;
}

DWORD GameSingleton::findProcessId(std::string processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);
	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE) return -1;
	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile)) {
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}
	while (Process32Next(processesSnapshot, &processInfo)) {
		if (!processName.compare(processInfo.szExeFile)) {
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	CloseHandle(processesSnapshot);
	return -1;
}

DWORD GameSingleton::findModuleBase(HANDLE process, std::string moduleName)
{
	HMODULE *hModules;
	char szBuf[50];
	DWORD cModules;
	DWORD dwBase = -1;

	EnumProcessModules(process, NULL, 0, &cModules);
	hModules = new HMODULE[cModules / sizeof(HMODULE)];

	if (EnumProcessModules(process, hModules, cModules / sizeof(HMODULE), &cModules)) {
		for (int i = 0; i < cModules / sizeof(HMODULE); i++) {
			if (GetModuleBaseName(process, hModules[i], szBuf, sizeof(szBuf))) {
				if (moduleName.compare(szBuf) == 0) {
					dwBase = (DWORD)hModules[i];
					break;
				}
			}
		}
	}

	delete[] hModules;

	return dwBase;
}
