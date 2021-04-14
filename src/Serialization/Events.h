#pragma once

namespace Events
{
	class EventEnablePlugin : public SKSE::RegistrationSet<bool>
	{
	public:
		using Base = SKSE::RegistrationSet<bool>;

		EventEnablePlugin() :
			Base("EnablePlugin"sv)
		{}

		static EventEnablePlugin* GetSingleton()
		{
			static EventEnablePlugin singleton;
			return &singleton;
		}
	private:
		EventEnablePlugin(const EventEnablePlugin&) = delete;
		EventEnablePlugin(EventEnablePlugin&&) = delete;
		~EventEnablePlugin() = default;

		EventEnablePlugin& operator=(const EventEnablePlugin&) = delete;
		EventEnablePlugin& operator=(EventEnablePlugin&&) = delete;
	};
}
