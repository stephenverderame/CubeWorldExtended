#include "GUIMediator.h"

Mediator::Mediator(QuitObserver * qo) : show(false), lastKeyPress(0)
{
	GameSingleton * game = GameSingleton::getInstance();
	int length = game->getWndLength();
	int height = game->getWndHeight();
	gui::GUI::bindWindow(game->getHwnd());
	interpreter = new Interpreter(&ss);
	interpreter->attatch(this);
	interpreter->attatch(qo);
	gui::GUI::bindWindow(game->getHwnd());
	gui::TextField * commandBox = new gui::TextField("cmd", length * 0.005, height - (height * 0.01), length * 0.2, 40, ES_LOWERCASE | WS_THICKFRAME | WS_TABSTOP);
	gui::TextField * displayBox = new gui::TextField("display", length * 0.005, height - (height * 0.2), length * 0.2, height * 0.15, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER);
	gui::Page * page = new gui::Page("p1", { commandBox, displayBox });
	mainPage = new gui::MainPage({ page, new gui::Page("emptyPage") });
	mainPage->navigateTo("emptyPage");
	displayBox->disableTyping();
}

Mediator::~Mediator()
{
	delete interpreter;
	GUI_CLEANUP(mainPage);
}

void Mediator::handleEvents()
{
	GameSingleton * game = GameSingleton::getInstance();
	MSG msg;
	if (PeekMessage(&msg, game->getHwnd(), 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		mainPage->handleMessage(&msg);
	}
	if (GetAsyncKeyState('Z') < 0 && GetAsyncKeyState(VK_CONTROL) < 0 && clock() - lastKeyPress > CLOCKS_PER_SEC * 0.5) {
		lastKeyPress = clock();
		if (show) mainPage->navigateTo("emptyPage");
		else mainPage->navigateTo("p1");
		show = !show;
		ShowCursor(show);
	}
	if (GetAsyncKeyState(VK_RETURN) < 0 && show && clock() - lastKeyPress > CLOCKS_PER_SEC * 0.5) {
		lastKeyPress = clock();
		std::string command = ((gui::TextField *)mainPage->getCurrentPage()->getControl("cmd"))->getText();
		interpreter->handleCommand(command);
		((gui::TextField *)mainPage->getCurrentPage()->getControl("cmd"))->setText("");
	}
	if (mustDisplay) {
		mustDisplay = false;
		gui::TextField * displayBox = ((gui::TextField *)mainPage->getCurrentPage()->getControl("display"));
		displayBox->setText((char*)ss.str().c_str());
		SendMessage(displayBox->getHandle(), EM_SETSEL, 0, -1);
		SendMessage(displayBox->getHandle(), EM_SETSEL, -1, -1);
		SendMessage(displayBox->getHandle(), EM_SCROLLCARET, 0, 0);
	}
}

void Mediator::update(Notification n)
{
	if (n.type == 0x10)
		mustDisplay = true;
}
