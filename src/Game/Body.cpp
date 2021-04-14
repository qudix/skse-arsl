#include "Game/Body.h"

namespace ARSL
{
	constexpr const char* ATTACHMENT_HOLDER = "Arousal [ARSL]";

	bool AttachMesh(RE::TESObjectREFR* a_ref, const char* a_path, [[maybe_unused]] const char* a_nodeName)
	{
		auto av = a_ref->Get3D1(false);
		if (!av)
			return false;

		RE::BSFixedString parentName("NPC Root [Root]");
		auto rootDestination = av->GetObjectByName(parentName.data());
		if (!rootDestination)
			return false;

		auto rootNode = rootDestination->AsNode();
		if (!rootNode)
			return false;

		bool createdHolder = false;
		RE::BSFixedString holderName(ATTACHMENT_HOLDER);

		auto holder = rootDestination->GetObjectByName(holderName.data());
		if (!holder) {
			//holder = RE::NiNode::Create(0);
			//holder->name = holderName.data();
			//createdHolder = true;
		}

		//auto holderNode = holder->AsNode();
		//if (!holderNode)
		//	return false;

		//RE::BSFixedString targetNodeName(a_nodeName);
		//auto target = holderNode->GetObjectByName(targetNodeName.data());
		//if (target) {
			//...
		//}

		RE::BSFixedString nifPath(a_path);
		RE::NiStream niStream;
		RE::BSResourceNiBinaryStream binaryStream(nifPath.data());
		if (!binaryStream.good()) {
			logger::warn("No file exists at ({})", a_path);
			return false;
		}

		// Calling this crashes...
		if (!niStream.LoadStream(&binaryStream)) {
			logger::warn("File at ({}) is not a valid nif", a_path);
			return false;
		}

		return true;
	}

	void Body::Update(RE::Actor* a_actor)
	{
		bool success = AttachMesh(a_actor, "meshes\\test_mesh.nif", "");
		logger::info("AttachMesh: {}", success);
	}
}
