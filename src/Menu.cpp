#include "Menu.h"
#include "Camera.h"

#include "SKSE/API.h"


auto MenuEventHandler::ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource)
-> EventResult
{
	if (!a_event) {
		return EventResult::kContinue;
	}

	const auto uiStr = RE::InterfaceStrings::GetSingleton();
	const auto camera = CameraController::GetSingleton();
	if (a_event->menuName == uiStr->inventoryMenu ||
		a_event->menuName == uiStr->barterMenu ||
		a_event->menuName == uiStr->containerMenu) {
		if (a_event->opening) {
			camera->Rotate();
		} else {
			camera->Restore();
		}
	}
	
	return EventResult::kContinue;
}


void MenuEventHandler::RegisterEvent()
{
	auto ui = RE::UI::GetSingleton();

	ui->AddEventSink(this);
	_MESSAGE("Registered menu event handler");
}