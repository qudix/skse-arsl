#pragma once

#include "Util/STL.h"

namespace Papyrus::PapyrusArousal
{
    using VM = RE::BSScript::IVirtualMachine;
    using namespace ARSL;

    uint32_t GetStaticEffectCount(RE::StaticFunctionTag*)
    {
		auto arousal = Arousal::GetSingleton();
        return arousal->m_effectCount;
    }

    std::string GetUnusedEffectId(int32_t a_id)
    {
		return "Unused" + std::to_string(a_id);
    }

    int32_t GetHighestUnusedEffectId()
    {
        int32_t result = -1;
		auto arousal = Arousal::GetSingleton();
		auto& effectIDs = arousal->m_effectIDs;
		while (effectIDs.find(GetUnusedEffectId(result + 1)) != effectIDs.end())
            result += 1;

        return result;
    }

    uint32_t RegisterStaticEffect(RE::StaticFunctionTag*, RE::BSFixedString a_name)
    {
		auto arousal = Arousal::GetSingleton();
		auto itr = arousal->m_effectIDs.find(a_name.data());
		if (itr != arousal->m_effectIDs.end())
            return itr->second;

        int32_t unusedId = GetHighestUnusedEffectId();
        if (unusedId != -1) {
			itr = arousal->m_effectIDs.find(GetUnusedEffectId(unusedId));
			assert(itr != arousal->m_effectIDs.end());
            uint32_t effectId = itr->second;
			arousal->m_effectIDs.erase(itr);
			arousal->m_effectIDs[a_name.data()] = effectId;
            return effectId;
        }

        arousal->m_effectIDs[a_name.data()] = arousal->m_effectCount;
        for (auto& data : arousal->m_arousalData)
            data.second.OnRegisterStaticEffect();

        const auto result = arousal->m_effectCount;
		arousal->m_effectCount++;
        return result;
    }

    bool UnregisterStaticEffect(RE::StaticFunctionTag*, RE::BSFixedString a_name)
    {
		auto arousal = Arousal::GetSingleton();
		auto itr = arousal->m_effectIDs.find(a_name.data());
		if (itr != arousal->m_effectIDs.end()) {
            uint32_t id = itr->second;
			arousal->m_effectIDs.erase(itr);
            int32_t unusedId = GetHighestUnusedEffectId();
			arousal->m_effectIDs[GetUnusedEffectId(unusedId + 1)] = id;
			for (auto& data : arousal->m_arousalData)
                data.second.OnUnregisterStaticEffect(id);

            return true;
        }

        return false;
    }

    ArousalData& _GetOrCreateArousalData(uint32_t a_formID)
    {
		auto arousal = Arousal::GetSingleton();
		if (arousal->m_lastLookup == a_formID && arousal->m_lastData)
			return *arousal->m_lastData;

		auto& result = arousal->m_arousalData[a_formID];
		arousal->m_lastLookup = a_formID;
		arousal->m_lastData = &result;
        return result;
    }

    ArousalData& GetArousalData(RE::Actor* a_actor)
    {
		if (!a_actor)
            throw std::invalid_argument("Attempt to get arousal data for none actor");

        return _GetOrCreateArousalData(a_actor->formID);
    }

    ArousalEffectData& GetStaticArousalEffect(RE::Actor* a_actor, int32_t a_effectIDX)
    {
		ArousalData& data = GetArousalData(a_actor);
		return data.GetStaticArousalEffect(a_effectIDX);
    }

    int32_t GetDynamicEffectCount(RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
            return data.GetDynamicEffectCount();
        }
        catch (std::exception) { return 0; }
    }

    RE::BSFixedString GetDynamicEffect(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_number)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
            return data.GetDynamicEffect(a_number);
        }
        catch (std::exception) { return ""; }
    }

    float GetDynamicEffectValueByName(RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_effectID)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			return data.GetDynamicEffectValueByName(a_effectID);
        }
        catch (std::exception) { return 0.0; }
    }

    float GetDynamicEffectValue(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_number)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			return data.GetDynamicEffectValue(a_number);
        }
        catch (std::exception) { return std::numeric_limits<float>::lowest(); }
    }

    bool IsStaticEffectActive(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			return data.IsStaticEffectActive(a_effectIDX);
        }
        catch (std::exception) { return false; }
    }

    float GetStaticEffectValue(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			if (auto group = data.GetEffectGroup(a_effectIDX))
                return group->value;
			ArousalEffectData& effect = data.GetStaticArousalEffect(a_effectIDX);
            return effect.value;
        }
        catch (std::exception) { return 0.f; }
    }

    float GetStaticEffectParam(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX)
    {
        try {
			ArousalEffectData& effect = GetStaticArousalEffect(a_actor, a_effectIDX);
            return effect.param;
        }
        catch (std::exception) { return 0.f; }
    }

    int32_t GetStaticEffectAux(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX)
    {
        try {
			ArousalEffectData& effect = GetStaticArousalEffect(a_actor, a_effectIDX);
            return effect.intAux;
        }
        catch (std::exception) { return 0; }
    }

    void SetDynamicArousalEffect(RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_effectID, float a_initialValue, int32_t a_functionID, float a_param, float a_limit)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			data.SetDynamicArousalEffect(a_effectID, a_initialValue, a_functionID, a_param, a_limit);
        }
        catch (std::exception) {}
    }

    void ModDynamicArousalEffect(RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_effectID, float a_modifier, float a_limit)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			data.ModDynamicArousalEffect(a_effectID, a_modifier, a_limit);
        }
        catch (std::exception) {}
    }

    void SetStaticArousalEffect(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX, int32_t functionID, float a_param, float a_limit, int32_t a_auxilliary)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			data.SetStaticArousalEffect(a_effectIDX, functionID, a_param, a_limit, a_auxilliary);
        }
        catch (std::exception) {}
    }

    void SetStaticArousalValue(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX, float a_value)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			data.SetStaticArousalValue(a_effectIDX, a_value);
        }
        catch (std::exception) {}
    }

    float ModStaticArousalValue(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX, float a_diff, float a_limit)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			return data.ModStaticArousalValue(a_effectIDX, a_diff, a_limit);
        }
        catch (std::exception) { return 0.f; }
    }

    void SetStaticAuxillaryFloat(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX, float a_value)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			ArousalEffectData& effect = data.GetStaticArousalEffect(a_effectIDX);

            effect.floatAux = a_value;
        }
        catch (std::exception) {}
    }

    void SetStaticAuxillaryInt(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_effectIDX, int32_t a_value)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			ArousalEffectData& effect = data.GetStaticArousalEffect(a_effectIDX);

            effect.intAux = a_value;
        }
        catch (std::exception) {}
    }

    float GetActorArousal(RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        try {
			return GetArousalData(a_actor).GetArousal();
        }
        catch (std::exception) { return 0.f; }
    }

    bool GroupEffects(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_idx, int32_t a_idx2)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			return data.GroupEffects(a_actor, a_idx, a_idx2);
        }
        catch (std::exception) { return false; }
    }

    bool RemoveEffectGroup(RE::StaticFunctionTag*, RE::Actor* a_actor, int32_t a_idx)
    {
        try {
			ArousalData& data = GetArousalData(a_actor);
			data.RemoveEffectGroup(a_idx);
            return true;
        }
        catch (std::exception) { return false; }
    }

    int32_t CleanUpActors(RE::StaticFunctionTag*, float a_lastUpdateBefore)
    {
		auto arousal = Arousal::GetSingleton();
        int32_t removed = 0;
		for (auto itr = arousal->m_arousalData.begin(); itr != arousal->m_arousalData.end();) {
			if (itr->second.GetLastUpdate() < a_lastUpdateBefore) {
				itr = arousal->m_arousalData.erase(itr);
                ++removed;
            }
            else
                ++itr;
        }
        return removed;
    }

    void UpdateActorArousal(RE::StaticFunctionTag*, RE::Actor* a_actor, float a_gameDaysPassed)
    {
        try
        {
			ArousalData& data = GetArousalData(a_actor);
			data.UpdateSingleActorArousal(a_actor, a_gameDaysPassed);
        }
        catch (std::exception) {}
    }

    void UpdateActorsArousal(RE::StaticFunctionTag*, std::vector<RE::Actor*> a_actors, float a_gameDaysPassed)
	{
		try {
			for (auto actor : a_actors) {
				ArousalData& data = GetArousalData(actor);
				data.UpdateSingleActorArousal(actor, a_gameDaysPassed);
			}
		} catch (std::exception) {}
	}

    std::vector<RE::Actor*> GetActorList(RE::StaticFunctionTag*)
    {
		auto arousal = Arousal::GetSingleton();
        std::vector<RE::Actor*> result;
		for (auto& [id, data] : arousal->m_arousalData) {
			RE::Actor* actor = dynamic_cast<RE::Actor*>(RE::TESForm::LookupByID(id));
            if (actor)
				result.push_back(actor);
		}
        return result;
    }

    bool Lock(RE::StaticFunctionTag*, int32_t a_lock)
    {
		auto arousal = Arousal::GetSingleton();
		return arousal->Lock(a_lock);
    }

    void Unlock(RE::StaticFunctionTag*, int32_t a_lock)
    {
		auto arousal = Arousal::GetSingleton();
		arousal->Unlock(a_lock);
    }

    std::vector<RE::Actor*> DuplicateActorArray(RE::StaticFunctionTag*, std::vector<RE::Actor*> a_actors, int32_t a_count)
    {
        std::vector<RE::Actor*> result;
		result.reserve(a_count);
		for (auto& actor : a_actors) {
            result.push_back(actor);
        }

        return result;
    }

    void Bind(VM& a_vm)
    {
		stl::build_table();

        const auto obj = "qdx_ar"sv;

        BIND(GetStaticEffectCount);
		BIND(RegisterStaticEffect);
		BIND(UnregisterStaticEffect);
		BIND(IsStaticEffectActive);
		BIND(GetDynamicEffectCount);
		BIND(GetDynamicEffect);
		BIND(GetDynamicEffectValueByName);
		BIND(GetDynamicEffectValue);
		BIND(GetStaticEffectValue);
		BIND(GetStaticEffectParam);
		BIND(GetStaticEffectAux);
		BIND(SetStaticArousalEffect);
		BIND(SetDynamicArousalEffect);
		BIND(ModDynamicArousalEffect);
		BIND(SetStaticArousalValue);
		BIND(SetStaticAuxillaryFloat);
		BIND(SetStaticAuxillaryInt);
		BIND(ModStaticArousalValue);

		BIND(GetActorArousal);
		BIND(UpdateActorArousal);
		BIND(UpdateActorsArousal);

        BIND(GroupEffects);
		BIND(RemoveEffectGroup);

        BIND(CleanUpActors);

        BIND(Lock, true);
		BIND(Unlock, true);

		BIND(DuplicateActorArray, true);
    }
}