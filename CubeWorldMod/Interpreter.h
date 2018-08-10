#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include "Player.h"
#include "Observer.h"

namespace injection {
	extern "C"  void __stdcall petInventoryInjection();
}
class Interpreter : public Subject{
private:
	std::ostream * outputStream;
	bool petfoodCalled;
private:
	void showMessage(std::string message, std::initializer_list<std::string> vars = {});
	DWORD jmpBackAddress;
	DWORD overwriteAddress;
	LPVOID inventoryVariableAddress;
	LPVOID injectionFunction;
	BYTE injectionBackup[5];
public:
	Interpreter(std::ostream * output) : petfoodCalled(false) { outputStream = output; };
	void handleCommand(std::string command);
	std::ostream * getOutput() {
		return outputStream;
	}
};