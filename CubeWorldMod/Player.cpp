#include "Player.h"
PlayerSingleton * PlayerSingleton::instance = nullptr;
PlayerSingleton::PlayerSingleton()
{
	GameSingleton * game = GameSingleton::getInstance();
	DWORD basePetClassPointer = game->getBaseAddr() + 0x36B1C8;
	DWORD basePetClassPointer2;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePetClassPointer, &basePetClassPointer2, 4, 0)) printf("Read error %d \n", GetLastError());
	basePetClassPointer2 += 0x2E8;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePetClassPointer2, &basePetClassPointer, 4, 0)) printf("Read error %d \n", GetLastError());
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePetClassPointer, &basePetClassPointer2, 4, 0)) printf("Read error %d \n", GetLastError());
	basePetClassPointer2 += 0x18;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePetClassPointer2, &petBaseAddress, 4, 0)) printf("Read error %d \n", GetLastError());

	DWORD basePlayerClassPointer = game->getBaseAddr() + 0x36B1C8;
	DWORD basePlayerClassPointer2;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePlayerClassPointer, &basePlayerClassPointer2, 4, 0)) printf("Read error %d \n", GetLastError());
	basePlayerClassPointer2 += 0x2E8;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePlayerClassPointer2, &basePlayerClassPointer, 4, 0)) printf("Read error %d \n", GetLastError());
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePlayerClassPointer, &basePlayerClassPointer2, 4, 0)) printf("Read error %d \n", GetLastError());
	basePlayerClassPointer2 += 0x4;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePlayerClassPointer2, &basePlayerClassPointer, 4, 0)) printf("Read error %d \n", GetLastError());
	basePlayerClassPointer += 0x18;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)basePlayerClassPointer, &playerBaseAddress, 4, 0)) printf("Read error %d \n", GetLastError());

	char n[21];
	SIZE_T bytesRead;
	ReadProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x1168), n, 20, &bytesRead);
	n[bytesRead] = '\0'; //just in case name was cut off
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	saveFileName = n + std::string(".data");

	struct stat fileInfo;
	if (stat(saveFileName.c_str(), &fileInfo) == 0) {
		FILE * prevData = fopen(saveFileName.c_str(), "rb");
		if (prevData != NULL) {
			int size;
			fread(&size, 4, 1, prevData);
			for (int i = 0; i < size; i++) {
				int strlength;
				fread(&strlength, 4, 1, prevData);
				char * wayPointName = new char[strlength + 1];
				fread(wayPointName, 1, strlength, prevData);
				wayPointName[strlength] = '\0';
				PositionVector v;
				fread(&v, 24, 1, prevData);
				warps[wayPointName] = v;
				delete[] wayPointName;
			}
			printf("%s, your previous warps have been loaded!\n", name);
			fclose(prevData);
		}
	}

}

PlayerSingleton::~PlayerSingleton()
{
	delete[] name;
}

PlayerSingleton * PlayerSingleton::getInstance()
{
	if (instance == nullptr)
		instance = new PlayerSingleton();

	return instance;
}

void PlayerSingleton::cleanup()
{
	delete instance;
}

void PlayerSingleton::addWarp(const std::string name, const PositionVector v)
{
	warps[name] = v;
}

bool PlayerSingleton::delWarp(std::string name)
{
	if (warps.find(name) == warps.end()) return false;
	warps.erase(name);
	return true;
}

int PlayerSingleton::tpBack()
{
	if (previousPositions.size() < 1)
		return 1;
	int code = tp(previousPositions.top(), false);
	if (!code)
		previousPositions.pop();
	else
		return code;
	return 0;
}

const PositionVector PlayerSingleton::getWarp(const std::string name, bool & exists)
{
	if (warps.find(name) == warps.end()) {
		exists = false;
		return PositionVector();
	}
	return warps[name];
}

void PlayerSingleton::saveData()
{
	FILE * saveData = fopen(saveFileName.c_str(), "wb");
	if (saveData != NULL) {
		int size = warps.size();
		fwrite(&size, 4, 1, saveData);
		for (auto it = warps.begin(); it != warps.end(); it++) {
			int nameLength = (*it).first.size();
			fwrite(&nameLength, 4, 1, saveData);
			std::string name = (*it).first;
			fwrite(name.c_str(), 1, nameLength, saveData);
			fwrite(&(*it).second, 24, 1, saveData);
		}
		fclose(saveData);
	}
}

int PlayerSingleton::tp(const PositionVector v, const bool saveLastPos)
{
	float stamina, mana;
	GameSingleton * game = GameSingleton::getInstance();
	DWORD gameBaseAddress = game->getBaseAddr();
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x1194), &stamina, 4, 0)) return 1;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x170), &mana, 4, 0)) return 1;
	if (stamina < 1 && mana < 1) {
		return 3;
	}
	PositionVector currentPos;
	if (!ReadProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x10), &currentPos, 24, 0)) return 1;
	if (!WriteProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x10), &v, 24, 0)) return 1;
	if (saveLastPos)
		previousPositions.push(currentPos);
	stamina = 0; mana = 0;
	WriteProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x1194), &stamina, 4, 0);
	WriteProcessMemory(game->getHandle(), (LPVOID)(playerBaseAddress + 0x170), &mana, 4, 0);

	BYTE nop[8] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
	ReadProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x2187A7), replacement_code.movManaBackup, 8, 0);
	WriteProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x2187A7), nop, 8, 0);
	ReadProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x219D59), replacement_code.movStamBackup, 8, 0);
	WriteProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x219D59), nop, 8, 0);
	Timer * t = Timer::getInstance();
	t->addCommand(new TimerCommand(this, 30));
	return true;
}

void PlayerSingleton::action()
{
	GameSingleton * game = GameSingleton::getInstance();
	DWORD gameBaseAddress = game->getBaseAddr();
	WriteProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x2187A7), replacement_code.movManaBackup, 8, 0);
	WriteProcessMemory(game->getHandle(), (LPVOID)(gameBaseAddress + 0x219D59), replacement_code.movStamBackup, 8, 0);
}
