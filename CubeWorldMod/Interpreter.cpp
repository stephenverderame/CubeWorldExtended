#include "Interpreter.h"
void Interpreter::showMessage(std::string s, std::initializer_list<std::string> vars)
{
	std::stringstream message;
	int strings = 0;
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '%' && s[i + 1] == 's') {
			message << *(vars.begin() + strings++);
			i++;
		}
		else message << s[i];
	}
	message << "\r\n";
	*outputStream << message.str();
	notify({ 0x10 });
}

void Interpreter::handleCommand(std::string command)
{
	GameSingleton * game = GameSingleton::getInstance();
	PlayerSingleton * player = PlayerSingleton::getInstance();
	const HANDLE cubeHandle = game->getHandle();
	DWORD basePlayerClass = player->getpBaseAddr();
	const char * trueName = player->getName();
	showMessage("%s> %s", { std::string(player->getName()), command });
	if (command == "quit") {
		player->saveData();
		showMessage("Quitting program...");
		notify({ 0x20 });
	}
	else if (command.find("setwarp") != std::string::npos) {
		if (command.find('-') == std::string::npos)
			showMessage("Command usage: setwarp -[warpName]");
		else {
			std::string warpName = command.substr(command.find('-') + 1);
			int money, level;
			if (!ReadProcessMemory(cubeHandle, (LPVOID)(basePlayerClass + 0x190), &level, 4, 0)) printf("Read error %d \n", GetLastError());
			if (level < 10)
				showMessage("Sorry %s! You must be at least level 10 to warp.", { player->getName() });
			else {
				if (!ReadProcessMemory(cubeHandle, (LPVOID)(basePlayerClass + 0x1308), &money, 4, 0)) printf("Read error %d \n", GetLastError());
				if (money < 10) {
					showMessage("Sorry %s! Placing a warp costs 10 platinum", { trueName });
					return;
				}
				money -= 10;
				PositionVector position;
				if (!WriteProcessMemory(cubeHandle, (LPVOID)(basePlayerClass + 0x1308), &money, 4, 0)) printf("Write error %d \n", GetLastError());
				if (!ReadProcessMemory(cubeHandle, (LPVOID)(basePlayerClass + 0x10), &position, 24, 0)) printf("Read error %d \n", GetLastError());
				player->addWarp(warpName, position);
				showMessage("Warp '%s' set!", { warpName });
			}

		}
	}
	else if (command == "warps") {
		for (auto it = player->begin(); it != player->end(); it++) {
			showMessage("%s", { (*it).first.c_str() });
		}
	}
	else if (command.find("warpto") != std::string::npos) {
		if (command.find('-') == std::string::npos)
			showMessage("Command usage: warpto -[warpName]");
		else {
			std::string warpName = command.substr(command.find('-') + 1);
			bool exists;
			PositionVector v = player->getWarp(warpName, exists);
			if (!exists)
				showMessage("Warp %s is not saved!", { warpName });
			else
				if (player->tp(v, true) == 3)
					showMessage("You must have full mana and stamina to warp!");
		}
	}
	else if (command.find("delwarp") != std::string::npos) {
		if (command.find('-') == std::string::npos)
			showMessage("Command usage: delwarp -[warpName]");
		else {
			std::string warpName = command.substr(command.find('-') + 1);
			if (!player->delWarp(warpName))
				showMessage("Warp '%s' not found!", { warpName });
			else {
				showMessage("Warp '%s' forgotten!", { warpName });
			}
		}
	}
	else if (command == "back") {
		if (player->tpBack() == 3)
			showMessage("You must have full mana and stamina to warp!");
	}
#define INJECTION_SIZE 18
	else if (command == "petfood" && !petfoodCalled) {
		petfoodCalled = true;
		//each slot is 0x11c away
		//max of 56 slots

		inventoryVariableAddress = VirtualAllocEx(cubeHandle, 0, 4, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (inventoryVariableAddress == NULL) printf("Could not allocate space for variable! \n");
		injectionFunction = VirtualAllocEx(cubeHandle, 0, INJECTION_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (injectionFunction == NULL) printf("Could not allocate space for function! \n");
		char functionBuffer[INJECTION_SIZE];
		SIZE_T readBytes;
		DWORD relAddress;
		memcpy(&relAddress, (char*)injection::petInventoryInjection + 1, 4);
		printf("Relative address: %p \n", relAddress);
		printf("Function pointer: %p \n", injection::petInventoryInjection);
		const DWORD petInjection = (DWORD)injection::petInventoryInjection + relAddress + 5;
		if (!ReadProcessMemory(GetCurrentProcess(), (LPVOID)petInjection, functionBuffer, INJECTION_SIZE, &readBytes)) printf("Error reading petInventoryFunction? %d \n", GetLastError());
		printf("Read %d bytes from %p \n", readBytes, petInjection);
		printf("Function pointer: %p \n", injection::petInventoryInjection);
		if (!WriteProcessMemory(cubeHandle, injectionFunction, functionBuffer, readBytes, 0)) printf("Error writing petInjectionFunction? %d \n", GetLastError());
		overwriteAddress = game->getBaseAddr() + 0x4A6B2;
		jmpBackAddress = game->getBaseAddr() + 0x4A6B7;
		printf("JmpBack: %p \n", jmpBackAddress);
		for (int i = 0; i < INJECTION_SIZE; i++) {
			BYTE b;
			ReadProcessMemory(cubeHandle, (LPVOID)((DWORD)injectionFunction + i), &b, 1, 0);
			if (b == 0x89) {
				BYTE b2;
				ReadProcessMemory(cubeHandle, (LPVOID)((DWORD)injectionFunction + i + 1), &b2, 1, 0);
				if (b2 == 0x3D) {
					BYTE mov[6] = { 0x89, 0x3D, 0x90, 0x90, 0x90, 0x90 };
					memcpy(mov + 2, &inventoryVariableAddress, 4);
					WriteProcessMemory(cubeHandle, (LPVOID)((DWORD)injectionFunction + i), mov, 6, 0);
				}
			}
			if (b == 0xFF) {
				BYTE jmp[6] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0x90 };
				DWORD relativeAddress = jmpBackAddress - ((DWORD)injectionFunction + i) - 5;
				printf("Relative back: %p \n", relativeAddress);
				memcpy(jmp + 1, &relativeAddress, 4);
				WriteProcessMemory(cubeHandle, (LPVOID)((DWORD)injectionFunction + i), jmp, 6, 0);
			}
		}

		printf("Overwrite address: %p \n", overwriteAddress);
		printf("Injection function address: %p \n", injectionFunction);

		DWORD oldProtect, relativeAddress;
		BYTE jmp[5] = { 0xE9, 0x90, 0x90, 0x90, 0x90 };
		relativeAddress = (DWORD)injectionFunction - overwriteAddress - 5;
		memcpy(jmp + 1, &relativeAddress, 4);
		if (!ReadProcessMemory(cubeHandle, (LPVOID)overwriteAddress, injectionBackup, 5, 0)) printf("Could not backup data %d \n", GetLastError());
		if (!VirtualProtectEx(cubeHandle, (LPVOID)overwriteAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) printf("Error protecting newmem %d \n", GetLastError());
		if (!WriteProcessMemory(cubeHandle, (LPVOID)overwriteAddress, jmp, 5, 0)) printf("Error writing newmen %d \n", GetLastError());
		if (!VirtualProtectEx(cubeHandle, (LPVOID)overwriteAddress, 5, oldProtect, &oldProtect)) printf("Error reprotecting newmen %d \n", GetLastError());
		FlushInstructionCache(cubeHandle, 0, 0);

		showMessage("Please click the first (top left most) petfood slot. Then call this command again.");



	}
	else if (command == "petfood" && petfoodCalled) {
//		petfoodCalled = false;
		std::map<int, std::string> ids;
		std::map<std::string, std::string> tames;
		std::ifstream in;
		in.open("Ids.txt");
		if (in.is_open()) {
			std::string line;
			while (std::getline(in, line)) {
				int key = std::stoi(line.substr(0, line.find('-') - 1));
				std::string name = line.substr(line.find('-') + 2);
				for (int i = 0; i < name.size(); i++)
					name[i] = tolower(name[i]);
				ids.insert(std::make_pair(key, name));
			}
			in.close();
		}
		in.open("Pets.txt");
		if (in.is_open()) {
			std::string line;
			while (std::getline(in, line)) {
				std::string petfood = line.substr(line.find('-') + 2);
				for (int i = 0; i < petfood.size(); i++)
					petfood[i] = tolower(petfood[i]);
				std::string pet = line.substr(0, line.find('-') - 1);
				tames.insert(std::make_pair(petfood, pet));
			}
			in.close();
		}
		DWORD inventoryAddress;
		if (!ReadProcessMemory(cubeHandle, inventoryVariableAddress, &inventoryAddress, 4, 0)) printf("error reading address %d \n", GetLastError());
		inventoryAddress += 0x4;
		for (int i = 0; i < 56; i++) {
			DWORD id;
			if (!ReadProcessMemory(cubeHandle, (LPVOID)inventoryAddress, &id, 4, 0)) printf("error reading id %d \n", GetLastError());
			if (i == 0) printf("Id: %d \n", id);
			if (ids.find(id) != ids.end()) {
				if (tames.find(ids.at(id)) == tames.end()) printf("Error missspell %d has no key in Pets.txt! \n");
				else showMessage("%s --> %s", { ids.at(id), tames.at(ids.at(id)) });
			}
			inventoryAddress += 0x11C;
		}

		WriteProcessMemory(cubeHandle, (LPVOID)overwriteAddress, injectionBackup, 5, 0);
		VirtualFreeEx(cubeHandle, inventoryVariableAddress, 4, MEM_RELEASE);
		VirtualFreeEx(cubeHandle, injectionFunction, INJECTION_SIZE, MEM_RELEASE);
		FlushInstructionCache(cubeHandle, 0, 0);
		printf("Done!\n");
	}
	else {
		showMessage("back - teleports you to where your last teleport location");
		showMessage("coords - shows your in game position");
		showMessage("delwarp - delete the warp with the specefied name");
		showMessage("help - shows this message");
		showMessage("petfood - displays all the pets you can tame with your current inventory");
		showMessage("quit - quit the program");
		showMessage("setwarp - save current position");
		showMessage("warps - display all saved warps");
		showMessage("warpto - teleport to selected warp");
	}
}
