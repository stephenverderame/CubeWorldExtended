#include "Timer.h"

TimerCommand::TimerCommand(Receiver * cReceiver, int waitUntil, int repeat, int repeatPause) : receiver(cReceiver), waitUntil(waitUntil), repeat(repeat), 
	repeatPause(repeatPause), lastCall(clock()), firstCall(false), executions(0)
{

}

void TimerCommand::execute()
{
	if (canExecute()) {
		if (!firstCall)
			firstCall = true;
		else
			executions++;
		receiver->action();
		lastCall = clock();
	}
}

bool TimerCommand::canExecute()
{
	if (!firstCall) {
		if (clock() - lastCall > waitUntil * CLOCKS_PER_SEC) return true;
	}
	else if (executions < repeat) {
		if (clock() - lastCall > repeatPause * CLOCKS_PER_SEC) return true;
	}
	return false;
}
Timer * Timer::instance = nullptr;
Timer::Timer()
{
}

Timer::~Timer()
{
	for (int i = 0; i < commands.size(); i++)
		delete commands[i];
}

Timer * Timer::getInstance()
{
	if (instance == nullptr)
		instance = new Timer();
	return instance;
}

void Timer::cleanup()
{
	delete instance;
}

void Timer::addCommand(TimerCommand * command)
{
	commands.push_back(command);
}

void Timer::executeCommands()
{
	for (auto c : commands)
		c->execute();
}
