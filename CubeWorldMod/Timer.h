#pragma once
#include <time.h>
#include <vector>
class Receiver {
public:
	virtual void action() = 0;
};
class TimerCommand {
private:
	Receiver * receiver;
	int waitUntil;
	int repeat;
	int repeatPause;
	clock_t lastCall;
	bool firstCall;
	int executions;
public:
	TimerCommand(Receiver * cReceiver, int waitUntil = 0, int repeat = 0, int repeatPause = 0);
	void execute();
	bool canExecute();
};
class Timer {
private:
	static Timer * instance;
protected:
	Timer();
	~Timer();
public:
	static Timer * getInstance();
	static void cleanup();
private:
	std::vector<TimerCommand *> commands;
public:
	void addCommand(TimerCommand * command);
	void executeCommands();

};