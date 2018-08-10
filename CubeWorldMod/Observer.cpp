#include "Observer.h"

void Subject::notify(Notification type)
{
	for (auto o : observers)
		o->update(type);
}

void QuitObserver::update(Notification type)
{
	if (type.type == 0x20)
		shouldQuit = true;
}
