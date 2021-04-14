#pragma once

#include "ArousalData.h"

namespace ARSL
{
	class Arousal
	{
	public:
		static Arousal* GetSingleton()
		{
			static Arousal singleton;
			return &singleton;
		}

		bool Save(SKSE::SerializationInterface* a_intfc, uint32_t a_type, uint32_t a_version);
		bool Save(SKSE::SerializationInterface* a_intfc);
		bool Load(SKSE::SerializationInterface* a_intfc, uint32_t& a_length);
		void Revert(SKSE::SerializationInterface* a_intfc);

		bool Lock(int32_t a_lock);
		void Unlock(int32_t a_lock);

	public:
		uint32_t m_effectCount = 0;
		std::unordered_map<std::string, uint32_t> m_effectIDs;
		std::unordered_map<uint32_t, ArousalData> m_arousalData;

		uint32_t m_lastLookup;
		ArousalData* m_lastData = nullptr;

	private:
		Arousal() = default;
		Arousal(const Arousal&) = delete;
		Arousal(Arousal&&) = delete;
		~Arousal() = default;

		Arousal& operator=(const Arousal&) = delete;
		Arousal& operator=(Arousal&&) = delete;

		std::array<std::atomic_flag, 3> m_locks;
	};
}
