#pragma once
#include "Interpreter.h"
#include <guiMain.h>
#include <Page.h>
class Mediator : public Observer{
private:
	std::stringstream ss;
	Interpreter * interpreter;
	gui::MainPage * mainPage;
	bool show;
	clock_t lastKeyPress;
	bool mustDisplay;
public:
	Mediator(QuitObserver * qo);
	~Mediator();
	void handleEvents();
	void update(Notification n);
};