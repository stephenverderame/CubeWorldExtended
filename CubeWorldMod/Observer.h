#pragma once
#include <vector>
struct Notification {
	int type;
};
class Observer {
public:
	virtual void update(Notification type) = 0;
};
class Subject {
protected:
	std::vector<Observer *> observers;
public:
	void attatch(Observer * o) {
		observers.push_back(o);
	}
	void notify(Notification type);
};
class QuitObserver : public Observer {
private:
	bool shouldQuit;
public:
	QuitObserver() : shouldQuit(false) {};
	void update(Notification type);
	bool isQuitting() { return shouldQuit; }
};