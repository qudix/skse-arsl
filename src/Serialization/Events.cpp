#include "Events.h"

namespace Events
{
	void Update(RE::FormID a_id)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([a_id] {
			auto actor = RE::TESObjectREFR::LookupByID<RE::Actor>(a_id);
			if (actor) {
				//Actor::UpdateActor(*actor);
			}
		});
	}

	void Register()
	{

	}
}