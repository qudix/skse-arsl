#pragma once

namespace ARSL
{
    class Body
    {
	public:
		static Body* GetSingleton()
		{
			static Body singleton;
			return &singleton;
		}

		void Update(RE::Actor* a_actor);

	private:
		Body() = default;
		Body(const Body&) = delete;
		Body(Body&&) = delete;
		~Body() = default;

		Body& operator=(const Body&) = delete;
		Body& operator=(Body&&) = delete;
    };
}
