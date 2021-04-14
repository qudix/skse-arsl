#pragma once

namespace serial
{
	inline void write_string(SKSE::SerializationInterface* intfc, std::string const& string)
	{
		uint32_t length = static_cast<uint32_t>(string.length());
		intfc->WriteRecordData(length);
		intfc->WriteRecordData(string.c_str(), length);
	}

	template <typename T>
	inline void write_container(SKSE::SerializationInterface* intfc, T const& container)
	{
		uint32_t size = static_cast<uint32_t>(container.size());
		intfc->WriteRecordData(&size, sizeof(size));
		for (auto const& kvp : container)
			intfc->WriteRecordData(&kvp, sizeof(kvp));
	}

	template <typename T>
	inline T read(SKSE::SerializationInterface* intfc, uint32_t& length)
	{
		uint32_t size = sizeof(T);
		if (size > length)
			throw std::length_error("Savegame data ended unexpected");

		length -= size;
		T result{};
		intfc->ReadRecordData(&result, size);
		return result;
	}

	inline std::string read_string(SKSE::SerializationInterface* intfc, uint32_t& length)
	{
		uint32_t strlength = serial::read<uint32_t>(intfc, length);
		if (strlength > length)
			throw std::length_error("Savegame data ended unexpected");

		length -= strlength;
		char* buf = new char[strlength + 1];
		intfc->ReadRecordData(buf, strlength);
		buf[strlength] = '\0';
		std::string result(buf);
		delete[] buf;
		return result;
	}
}
