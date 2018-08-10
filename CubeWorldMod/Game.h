#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <string>
class GameSingleton {
private:
	static GameSingleton * instance;
protected:
	GameSingleton();
	~GameSingleton();
public:
	static GameSingleton * getInstance();
	static void cleanup();

private:
	HWND gameHwnd;
	HANDLE gameHandle;
	DWORD gamePid;
	struct {
		int length;
		int height;
	}cube_window;
	DWORD gameBaseAddr;
private:
	DWORD findProcessId(std::string processName);
	DWORD findModuleBase(HANDLE process, std::string moduleName);
public:
	const HWND getHwnd() {
		return gameHwnd;
	};
	const HANDLE getHandle() {
		return gameHandle;
	}
	const DWORD getProcessId() {
		return gamePid;
	}
	const DWORD getBaseAddr() {
		return gameBaseAddr;
	}
	const int getWndLength() {
		return cube_window.length;
	}
	const int getWndHeight() {
		return cube_window.height;
	}

};