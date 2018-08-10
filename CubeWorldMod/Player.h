#pragma once
#include "Game.h"
#include "Timer.h"
#include <map>
#include <stack>
struct PositionVector {
	double x;
	double z;
	double y;
};
class PlayerSingleton : public Receiver{
private:
	static PlayerSingleton * instance;
private:
	char * name;
	unsigned long playerBaseAddress;
	unsigned long petBaseAddress;
	std::map<std::string, PositionVector> warps;
	std::stack<PositionVector> previousPositions;
	std::string saveFileName;
	struct {
		BYTE movManaBackup[8];
		BYTE movStamBackup[8];
	}replacement_code;
protected:
	PlayerSingleton();
	~PlayerSingleton();
public:
	static PlayerSingleton * getInstance();
	static void cleanup();
public:
	const char * getName() {
		return name;
	}
	const unsigned long getpBaseAddr() {
		return playerBaseAddress;
	}
	void addWarp(const std::string name, const PositionVector v);
	bool delWarp(std::string name);
	int tpBack();
	const PositionVector getWarp(const std::string name, bool & exists);
	void saveData();
	int tp(const PositionVector v, const bool saveLastPos);
	std::map<std::string, PositionVector>::iterator begin() { return warps.begin(); }
	std::map<std::string, PositionVector>::iterator end() { return warps.end(); }
public:
	void action();
	
};