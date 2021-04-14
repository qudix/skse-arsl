#pragma once

namespace ARSL
{
	struct ArousalEffectGroup
	{
		ArousalEffectGroup() :
			value(0.f)
		{}

		std::vector<uint32_t> staticEffectIds;
		float value;
	};

	struct ArousalEffectData
	{
		ArousalEffectData() :
			value(0.f), function(0), param(0.f), limit(0.f), intAux(0)
		{}

		float value;
		int32_t function;
		float param;
		float limit;
		union
		{
			float floatAux;
			int32_t intAux;
		};

		void Set(int32_t a_functionID, float a_param, float a_limit, int32_t a_auxilliary);
	};

	using ArousalEffectGroupPtr = std::shared_ptr<ArousalEffectGroup>;

	class ArousalData
	{
	public:
		enum class LimitCheck
		{
			None,
			UpperBound,
			LowerBound
		};

		ArousalData() {}
		ArousalData(uint32_t a_count);
		ArousalData& operator=(ArousalData&& other) = default;

		void Load(SKSE::SerializationInterface* intfc, uint32_t& length);
		void Save(SKSE::SerializationInterface* intfc);

		void OnRegisterStaticEffect();
		void OnUnregisterStaticEffect(uint32_t id);

		ArousalEffectGroupPtr GetEffectGroup(int32_t effectIdx);
		ArousalEffectData& GetStaticArousalEffect(int32_t effectIdx);

		int32_t GetDynamicEffectCount();
		RE::BSFixedString GetDynamicEffect(int32_t a_number);
		float GetDynamicEffectValue(int32_t a_number);
		float GetDynamicEffectValueByName(RE::BSFixedString a_effectID);

		bool IsStaticEffectActive(int32_t a_effectIDX);
		void RemoveDynamicEffectIfNeeded(std::string a_effectName, ArousalEffectData& a_effect);

		void SetDynamicArousalEffect(RE::BSFixedString a_effectID, float a_initialValue, int32_t a_functionID, float a_param, float a_limit);
		void ModDynamicArousalEffect(RE::BSFixedString a_effectID, float a_modifier, float a_limit);

		void SetStaticArousalEffect(int32_t a_effectIDX, int32_t a_functionID, float a_param, float a_limit, int32_t a_auxilliary);
		void SetStaticArousalValue(int32_t a_effectIDX, float a_value);

		float ModStaticArousalValue(int32_t a_effectIDX, float a_diff, float a_limit);

		bool CalculateArousalEffect(ArousalEffectData& a_effect, float a_timeDiff, RE::Actor* a_actor);

		bool GroupEffects(RE::Actor* a_actor, int32_t a_idx, int32_t a_idx2);

		void RemoveEffectGroup(int32_t a_idx);

		void UpdateSingleActorArousal(RE::Actor* a_actor, float a_gameDaysPassed);

		float GetArousal() const { return m_arousal; }
		float GetLastUpdate() const { return m_lastUpdate; }

	private:
		void UpdateGroupFactor(ArousalEffectGroup& a_group, float a_oldFactor, float a_newFactor);
		void UpdateGroup(ArousalEffectGroup& a_group, float a_timeDiff, RE::Actor* a_actor);
		bool UpdateArousalEffect(ArousalEffectData& a_effect, float a_timeDiff, RE::Actor* a_actor);

		ArousalData& operator=(const ArousalData&) = delete;
		ArousalData(const ArousalData&) = delete;

		std::unordered_set<int32_t> m_staticEffectsToUpdate;
		std::vector<ArousalEffectData> m_staticEffects;
		std::vector<ArousalEffectGroupPtr> m_staticEffectGroups;
		std::unordered_set<std::string> m_dynamicEffectsToUpdate;
		std::unordered_map<std::string, ArousalEffectData> m_dynamicEffects;
		std::vector<ArousalEffectGroupPtr> m_groupsToUpdate;

		float m_arousal;
		float m_lastUpdate;
		float m_lockedArousal;
	};
}
