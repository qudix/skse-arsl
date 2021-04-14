#include "ArousalData.h"

#include "Util/Serial.h"
#include "Util/STL.h"

namespace ARSL
{
	float GetEffectLimitOffset(uint32_t a_effectIDX)
	{
		return (a_effectIDX == 1) ? 0.5f : 0.0f;
	}

	void ArousalEffectData::Set(int32_t a_functionID, float a_param, float a_limit, int32_t a_auxilliary)
	{
		function = a_functionID;
		param = a_param;
		limit = a_limit + GetEffectLimitOffset(a_functionID);
		intAux = a_auxilliary;
	}

	ArousalData::ArousalData(uint32_t a_count) :
		m_arousal(0.f), m_lastUpdate(0.f), m_staticEffects(a_count), m_staticEffectGroups(a_count), m_lockedArousal(std::numeric_limits<float>::quiet_NaN())
	{}

	void ArousalData::Load(SKSE::SerializationInterface* intfc, uint32_t& length)
	{
		m_arousal = serial::read<float>(intfc, length);
		m_lastUpdate = serial::read<float>(intfc, length);
		uint32_t count = serial::read<uint32_t>(intfc, length);
		for (uint32_t j = 0; j < count; ++j)
			m_staticEffects[j] = serial::read<ArousalEffectData>(intfc, length);

		count = serial::read<uint8_t>(intfc, length);
		for (uint32_t j = 0; j < count; ++j) {
			auto grp = std::make_shared<ArousalEffectGroup>();
			uint32_t grpEntiryCount = serial::read<uint32_t>(intfc, length);
			for (uint32_t k = 0; k < grpEntiryCount; ++k) {
				uint32_t effIdx = serial::read<uint32_t>(intfc, length);
				grp->staticEffectIds.emplace_back(effIdx);
				m_staticEffectGroups[effIdx] = grp;
			}

			grp->value = serial::read<float>(intfc, length);
			if (std::abs(grp->value) > 10000.f) {
				logger::info("Possibly corrupted data reseting to zero");
				grp->value = 0.f;
			}

			m_groupsToUpdate.emplace_back(std::move(grp));
		}
		count = serial::read<uint32_t>(intfc, length);
		for (uint32_t j = 0; j < count; ++j)
			m_staticEffectsToUpdate.insert(serial::read<uint32_t>(intfc, length));

		count = serial::read<uint32_t>(intfc, length);
		for (uint32_t j = 0; j < count; ++j) {
			std::string name = serial::read_string(intfc, length);
			m_dynamicEffects[name] = serial::read<ArousalEffectData>(intfc, length);
		}

		count = serial::read<uint32_t>(intfc, length);
		for (uint32_t j = 0; j < count; ++j)
			m_dynamicEffectsToUpdate.insert(serial::read_string(intfc, length));

		float recalculated = 0.f;
		for (uint32_t i = 0; i < m_staticEffects.size(); ++i) {
			if (!m_staticEffectGroups[i])
				recalculated += m_staticEffects[i].value;
		}

		for (auto const& eff : m_dynamicEffects)
			recalculated += eff.second.value;

		for (auto const& grp : m_groupsToUpdate)
			recalculated += grp->value;

		if (std::abs(recalculated - m_arousal) > 0.5)
			logger::info("Arousal data mismatch: Expected: {} Got: {}", recalculated, m_arousal);

		m_arousal = recalculated;
	}

	void ArousalData::Save(SKSE::SerializationInterface* a_intfc)
	{
		a_intfc->WriteRecordData(m_arousal);
		a_intfc->WriteRecordData(m_lastUpdate);
		serial::write_container(a_intfc, m_staticEffects);
		uint8_t groupCount = static_cast<uint8_t>(m_groupsToUpdate.size());
		a_intfc->WriteRecordData(groupCount);
		for (auto& group : m_groupsToUpdate) {
			serial::write_container(a_intfc, group->staticEffectIds);
			a_intfc->WriteRecordData(group->value);
		}

		serial::write_container(a_intfc, m_staticEffectsToUpdate);
		uint32_t size = static_cast<uint32_t>(m_dynamicEffects.size());
		a_intfc->WriteRecordData(size);
		for (auto const& kvp : m_dynamicEffects) {
			serial::write_string(a_intfc, kvp.first);
			a_intfc->WriteRecordData(kvp.second);
		}

		size = static_cast<uint32_t>(m_dynamicEffectsToUpdate.size());
		a_intfc->WriteRecordData(size);
		for (auto const& toUpdate : m_dynamicEffectsToUpdate)
			serial::write_string(a_intfc, toUpdate);
	}

	void ArousalData::OnRegisterStaticEffect()
	{
		m_staticEffects.emplace_back();
		m_staticEffectGroups.emplace_back(nullptr);
	}

	void ArousalData::OnUnregisterStaticEffect(uint32_t a_id)
	{
		// staticEffects.erase(staticEffects.begin() + id);
		// staticEffectGroups.erase(staticEffectGroups.begin() + id);
		try {
			SetStaticArousalValue(a_id, 0.f);
			SetStaticArousalEffect(a_id, 0, 0.f, 0.f, 0);
			if (m_staticEffectGroups[a_id])
				RemoveEffectGroup(a_id);
		} catch (std::exception ex) {
			logger::info("Unexpected exception in OnUnregisterStaticEffect: {}", ex.what());
		}
	}

	ArousalEffectGroupPtr ArousalData::GetEffectGroup(int32_t a_effectIdx)
	{
		if (a_effectIdx < 0 || a_effectIdx >= m_staticEffects.size())
			throw std::invalid_argument("Invalid static effect index");
		return m_staticEffectGroups[a_effectIdx];
	}

	ArousalEffectData& ArousalData::GetStaticArousalEffect(int32_t a_effectIdx)
	{
		if (a_effectIdx < 0 || a_effectIdx >= m_staticEffects.size())
			throw std::invalid_argument("Invalid static effect index");
		return m_staticEffects[a_effectIdx];
	}

	int32_t ArousalData::GetDynamicEffectCount()
	{
		return static_cast<int32_t>(m_dynamicEffects.size());
	}

	RE::BSFixedString ArousalData::GetDynamicEffect(int32_t a_number)
	{
		if (a_number >= m_dynamicEffects.size())
			return "";

		auto itr = m_dynamicEffects.begin();
		std::advance(itr, a_number);
		return itr->first.c_str();
	}

	float ArousalData::GetDynamicEffectValue(int32_t a_number)
	{
		if (a_number >= m_dynamicEffects.size())
			return std::numeric_limits<float>::lowest();

		auto itr = m_dynamicEffects.begin();
		std::advance(itr, a_number);
		return itr->second.value;
	}

	float ArousalData::GetDynamicEffectValueByName(RE::BSFixedString a_effectID)
	{
		std::string effectName(a_effectID.data());

		auto itr = m_dynamicEffects.find(effectName);
		if (itr != m_dynamicEffects.end())
			return itr->second.value;
		else
			return 0.f;
	}

	bool ArousalData::IsStaticEffectActive(int32_t a_effectIDX)
	{
		return m_staticEffectsToUpdate.find(a_effectIDX) != m_staticEffectsToUpdate.end();
	}

	void ArousalData::RemoveDynamicEffectIfNeeded(std::string a_effectName, ArousalEffectData& a_effect)
	{
		if (a_effect.function == 0 && a_effect.value == 0.f)
			m_dynamicEffects.erase(a_effectName);
	}

	void ArousalData::SetDynamicArousalEffect(RE::BSFixedString a_effectID, float a_initialValue, int32_t a_functionID, float a_param, float a_limit)
	{
		std::string effectName(a_effectID.data());
		ArousalEffectData& effect = m_dynamicEffects[effectName];

		if (a_functionID && !effect.function)
			m_dynamicEffectsToUpdate.insert(effectName);
		else if (!a_functionID && effect.function)
			m_dynamicEffectsToUpdate.erase(effectName);

		effect.Set(a_functionID, a_param, a_limit, 0);
		if (a_initialValue) {
			m_arousal += a_initialValue - effect.value;
			effect.value = a_initialValue;
		}
		RemoveDynamicEffectIfNeeded(std::move(effectName), effect);
	}

	void ArousalData::ModDynamicArousalEffect(RE::BSFixedString a_effectID, float a_modifier, float a_limit)
	{
		std::string effectName(a_effectID.data());
		ArousalEffectData& effect = m_dynamicEffects[effectName];

		float value = effect.value + a_modifier;
		float actualDiff = a_modifier;
		if ((a_modifier < 0 && a_limit > value) || (a_modifier > 0 && a_limit < value)) {
			value = a_limit;
			actualDiff = a_limit - value;
		}

		m_arousal += actualDiff;
		effect.value = value;
		RemoveDynamicEffectIfNeeded(std::move(effectName), effect);
	}

	void ArousalData::SetStaticArousalEffect(int32_t a_effectIDX, int32_t a_functionID, float a_param, float a_limit, int32_t a_auxilliary)
	{
		ArousalEffectData& effect = GetStaticArousalEffect(a_effectIDX);

		if (a_functionID && !effect.function)
			m_staticEffectsToUpdate.insert(a_effectIDX);
		else if (!a_functionID && effect.function)
			m_staticEffectsToUpdate.erase(a_effectIDX);

		effect.Set(a_functionID, a_param, a_limit, a_auxilliary);
	}

	void ArousalData::SetStaticArousalValue(int32_t a_effectIDX, float a_value)
	{
		ArousalEffectData& effect = GetStaticArousalEffect(a_effectIDX);

		float diff = a_value - effect.value;
		effect.value = a_value;
		if (!m_staticEffectGroups[a_effectIDX])
			m_arousal += diff;
	}

	float ArousalData::ModStaticArousalValue(int32_t a_effectIDX, float a_diff, float a_limit)
	{
		ArousalEffectData& effect = GetStaticArousalEffect(a_effectIDX);

		float value = effect.value + a_diff;
		float actualDiff = a_diff;
		if ((a_diff < 0 && a_limit > value) || (a_diff > 0 && a_limit < value)) {
			value = a_limit;
			actualDiff = a_limit - value;
		}

		effect.value = value;
		if (!m_staticEffectGroups[a_effectIDX])
			m_arousal += actualDiff;

		return actualDiff;
	}

	bool ArousalData::CalculateArousalEffect(ArousalEffectData& a_effect, float a_timeDiff, RE::Actor* a_actor)
	{
		bool isDone = true;
		LimitCheck checkLimit = LimitCheck::None;
		float value;

		switch (a_effect.function) {
		case 1:
			value = a_effect.value * std::pow(0.5f, a_timeDiff / a_effect.param);
			checkLimit = a_effect.param * a_effect.value < 0.f ? LimitCheck::UpperBound : LimitCheck::LowerBound;
			break;
		case 2:
			value = a_effect.value + a_timeDiff * a_effect.param;
			checkLimit = a_effect.param >= 0.f ? LimitCheck::UpperBound : LimitCheck::LowerBound;
			break;
		case 3:
			value = (stl::sin(float(a_actor->formID % 7919) * 0.01f + m_lastUpdate * a_effect.param) + 1.f) * a_effect.limit;
			break;
		case 4:
			value = m_lastUpdate < a_effect.param ? 0.f : a_effect.limit;
			break;
		default:
			return true;
		}

		switch (checkLimit) {
		case LimitCheck::UpperBound:
			if (a_effect.limit < value)
				value = a_effect.limit + GetEffectLimitOffset(a_effect.function);
			else
				isDone = false;
			break;
		case LimitCheck::LowerBound:
			if (a_effect.limit > value)
				value = a_effect.limit - GetEffectLimitOffset(a_effect.function);
			else
				isDone = false;
			break;
		default:
			break;
		}

		a_effect.value = value;
		return isDone;
	}

	bool ArousalData::GroupEffects(RE::Actor* a_actor, int32_t a_idx, int32_t a_idx2)
	{
		ArousalEffectData& first = GetStaticArousalEffect(a_idx);
		ArousalEffectData& second = GetStaticArousalEffect(a_idx2);
		ArousalEffectGroupPtr targetGrp = m_staticEffectGroups[a_idx];
		ArousalEffectGroupPtr otherGrp = m_staticEffectGroups[a_idx2];

		if (!targetGrp)
			targetGrp = otherGrp;
		else if (otherGrp)
			return targetGrp == otherGrp;

		if (!targetGrp) {
			targetGrp = std::make_shared<ArousalEffectGroup>();
			m_groupsToUpdate.push_back(targetGrp);
		}

		if (!m_staticEffectGroups[a_idx]) {
			m_staticEffectGroups[a_idx] = targetGrp;
			targetGrp->staticEffectIds.push_back(a_idx);
			m_staticEffectsToUpdate.erase(a_idx);
			m_arousal -= first.value;
		}

		if (!m_staticEffectGroups[a_idx2]) {
			m_staticEffectGroups[a_idx2] = targetGrp;
			targetGrp->staticEffectIds.push_back(a_idx2);
			m_staticEffectsToUpdate.erase(a_idx2);
			m_arousal -= second.value;
		}

		UpdateGroup(*targetGrp, 0.f, a_actor);
		return true;
	}

	void ArousalData::RemoveEffectGroup(int32_t a_idx)
	{
		ArousalEffectGroupPtr group = m_staticEffectGroups[a_idx];
		auto itr = std::find(m_groupsToUpdate.begin(), m_groupsToUpdate.end(), group);
		if (itr == m_groupsToUpdate.end())
			throw std::logic_error("Error while removing group: group does not exist!");

		m_groupsToUpdate.erase(itr);
		m_arousal -= group->value;
		for (uint32_t id : group->staticEffectIds) {
			ArousalEffectData& eff = GetStaticArousalEffect(id);
			m_staticEffectGroups[id] = nullptr;
			m_arousal += eff.value;
			if (eff.function)
				m_staticEffectsToUpdate.insert(id);
		}
	}

	void ArousalData::UpdateSingleActorArousal(RE::Actor* a_actor, float a_gameDaysPassed)
	{
		if (!m_lastUpdate) {
			std::random_device rd;
			std::default_random_engine gen{ rd() };
			std::normal_distribution<> d{ 0.5, 2.0 };
			float randomTimeDiff = std::abs(float(d(gen)));
			m_lastUpdate = a_gameDaysPassed - randomTimeDiff;
		}

		float diff = a_gameDaysPassed - m_lastUpdate;
		m_lastUpdate = a_gameDaysPassed;

		for (auto& group : m_groupsToUpdate)
			UpdateGroup(*group, diff, a_actor);

		for (auto itr = m_staticEffectsToUpdate.begin(); itr != m_staticEffectsToUpdate.end();) {
			if (m_staticEffectGroups[*itr])
				++itr;
			else {
				ArousalEffectData& effect = m_staticEffects[*itr];
				if (UpdateArousalEffect(effect, diff, a_actor)) {
					effect.function = 0;
					itr = m_staticEffectsToUpdate.erase(itr);
				} else
					++itr;
			}
		}

		for (auto itr = m_dynamicEffectsToUpdate.begin(); itr != m_dynamicEffectsToUpdate.end();) {
			ArousalEffectData& effect = m_dynamicEffects[*itr];
			if (UpdateArousalEffect(effect, diff, a_actor)) {
				effect.function = 0;
				RemoveDynamicEffectIfNeeded(*itr, effect);
				itr = m_dynamicEffectsToUpdate.erase(itr);
			} else
				++itr;
		}
	}

	void ArousalData::UpdateGroupFactor(ArousalEffectGroup& a_group, float a_oldFactor, float a_newFactor)
	{
		float value = a_group.value / a_oldFactor * a_newFactor;
		float diff = value - a_group.value;
		m_arousal += diff;
		a_group.value = value;
	}

	void ArousalData::UpdateGroup(ArousalEffectGroup& a_group, float a_timeDiff, RE::Actor* a_actor)
	{
		float value = 1.f;
		for (uint32_t id : a_group.staticEffectIds) {
			ArousalEffectData& eff = GetStaticArousalEffect(id);
			CalculateArousalEffect(eff, a_timeDiff, a_actor);
			value *= eff.value;
		}
		float diff = value - a_group.value;
		m_arousal += diff;
		a_group.value = value;
	}

	// Should only be called for non-grouped effects
	bool ArousalData::UpdateArousalEffect(ArousalEffectData& a_effect, float a_timeDiff, RE::Actor* a_actor)
	{
		float oldValue = a_effect.value;
		bool isDone = CalculateArousalEffect(a_effect, a_timeDiff, a_actor);
		float diff = a_effect.value - oldValue;
		m_arousal += diff;
		return isDone;
	}
}
