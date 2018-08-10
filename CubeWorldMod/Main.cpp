#define _CRT_SECURE_NO_WARNINGS
#include "GUIMediator.h"
int main() {
	bool running = true;
	bool show = false;
	printf("This is the legacy client, open the modern client in-game with Ctrl + Z. Do not close this client, it will be used to display errors.\n");
	printf("Commands are no longer supported on this CLI legacy client.\n");

	QuitObserver quitCheck;
	Mediator mediator(&quitCheck);
	while (running) {
		if (quitCheck.isQuitting()) break;
		mediator.handleEvents();
		Timer::getInstance()->executeCommands();
	}
	GameSingleton::cleanup();
	PlayerSingleton::cleanup();
	Timer::cleanup();
}