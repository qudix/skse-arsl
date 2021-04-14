#pragma once

namespace Papyrus::PapyrusUtil
{
	using VM = RE::BSScript::IVirtualMachine;

	void Test(RE::StaticFunctionTag*, RE::Actor* a_actor)
	{
		auto body = ARSL::Body::GetSingleton();
		body->Update(a_actor);
	}

	void Bind(VM& a_vm)
	{
		const auto obj = "qdx_ar"sv;

		BIND(Test);
	}
}
