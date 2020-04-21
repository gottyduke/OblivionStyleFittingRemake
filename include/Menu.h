#pragma once

#include "RE/Skyrim.h"
#include "SKSE/CodeGenerator.h"


class MenuEventHandler final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	using EventResult = RE::BSEventNotifyControl;
	
	static MenuEventHandler* GetSingleton()
	{
		static MenuEventHandler singleton;
		return std::addressof(singleton);
	}

	EventResult ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_eventSource) override;
	void RegisterEvent();

	MenuEventHandler(const MenuEventHandler&) = delete;
	MenuEventHandler(MenuEventHandler&&) = delete;
	MenuEventHandler& operator=(const MenuEventHandler&) = delete;
	MenuEventHandler& operator=(MenuEventHandler&&) = delete;
	
private:
	MenuEventHandler() = default;
	virtual ~MenuEventHandler() = default;
};