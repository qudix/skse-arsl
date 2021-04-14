#include "Game/Arousal.h"

#include "Util/Serial.h"

namespace ARSL
{
	bool Arousal::Save(SKSE::SerializationInterface* a_intfc, uint32_t a_type, uint32_t a_version)
	{
		if (!a_intfc->OpenRecord(a_type, a_version)) {
			logger::info("Failed to open serializtion record!"sv);
			return false;
		}

		return Save(a_intfc);
	}

	bool Arousal::Save(SKSE::SerializationInterface* a_intfc)
	{
		logger::info("Saving effects...");
		a_intfc->WriteRecordData(m_effectCount);
		for (auto& [name, id] : m_effectIDs) {
			serial::write_string(a_intfc, name);
			a_intfc->WriteRecordData(id);
		}

		logger::info("Saving data sets...");
		auto count = static_cast<uint32_t>(m_arousalData.size());
		a_intfc->WriteRecordData(count);
		for (auto& [formID, data] : m_arousalData) {
			a_intfc->WriteRecordData(formID);
			data.Save(a_intfc);
		}

		return true;
	}

	bool Arousal::Load(SKSE::SerializationInterface* a_intfc, uint32_t& a_length)
	{
		m_effectCount = serial::read<uint32_t>(a_intfc, a_length);
		logger::info("Loading {} effects... ", m_effectCount);

		for (uint32_t i = 0; i < m_effectCount; ++i) {
			std::string effect = serial::read_string(a_intfc, a_length);
			uint32_t id = serial::read<uint32_t>(a_intfc, a_length);
			m_effectIDs[effect] = id;
			// logger::info("Added effect '{}' with id {}", effect.c_str(), id);
		}

		uint32_t dataCount = serial::read<uint32_t>(a_intfc, a_length);
		logger::info("Loading {} data sets... ", dataCount);

		for (uint32_t i = 0; i < dataCount; ++i) {
			RE::FormID formID = serial::read<uint32_t>(a_intfc, a_length);
			if (!a_intfc->ResolveFormID(formID, formID)) {
				logger::error("Failed to resolve formID {}"sv, formID);
				continue;
			}

			// logger::info("Loading data for actor {}...", formId);
			ArousalData data(m_effectCount);
			data.Load(a_intfc, a_length);
			m_arousalData[formID] = std::move(data);
		}

		return true;
	}

	void Arousal::Revert(SKSE::SerializationInterface*)
	{
		m_effectCount = 0;
		m_effectIDs.clear();
		m_lastLookup = 0;
		m_lastData = nullptr;
		m_arousalData.clear();

		for (auto& lock : m_locks)
			lock.clear();
	}

	bool Arousal::Lock(int32_t a_lock)
	{
		if (a_lock < 0 || a_lock >= m_locks.size())
			return false;
		if (m_locks[a_lock].test_and_set())
			return false;

		return true;
	}
	void Arousal::Unlock(int32_t a_lock)
	{
		if (a_lock < 0 || a_lock >= m_locks.size())
			return;

		m_locks[a_lock].clear();
	}
}
