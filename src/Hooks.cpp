#include "Hooks.h"

#include "RE/Skyrim.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/RegistrationSet.h"
#include "SKSE/SafeWrite.h"


namespace Hooks
{
	namespace
	{
		struct Jmp64 final : SKSE::CodeGenerator
		{
			// branch > 2gb
			explicit Jmp64(const std::size_t a_jmpAddr) : CodeGenerator(0x14)
			{
				jmp(ptr[rip + 0]);
				dq(a_jmpAddr);
			}
		};


		// [<SkyrimSE.exe>+2F27180] *g_Inventory3DManager
		const REL::Offset<std::uintptr_t> baseFunc = REL::ID(50884);
		const std::uintptr_t hookFunc = unrestricted_cast<std::uintptr_t>(&HookUpdateItem3D);

		inline Jmp64 jmpHook(hookFunc);
		inline std::uint8_t head[14];
	}


	void __fastcall HookUpdateItem3D(void* a_this, RE::InventoryEntryData* a_entry)
	{
		if (!a_entry) {
			return;
		}

		auto i3d = RE::Inventory3DManager::GetSingleton();
		switch (a_entry->GetObject()->GetFormType()) {
		case RE::FormType::Armor:
		case RE::FormType::Weapon:
		case RE::FormType::Ammo:
			i3d->Clear3D();
			return;
		default:;
		}

		WriteRet();

		i3d->UpdateItem3D(a_entry);

		WriteJmp();
	}

	
	void WriteJmp()
	{
		for (std::size_t i = 0; i < 14; ++i) {
			SKSE::SafeWrite8(baseFunc.GetAddress() + i, jmpHook.getCode()[i]);
		}
	}


	void WriteRet()
	{
		for (std::size_t i = 0; i < 14; ++i) {
			SKSE::SafeWrite8(baseFunc.GetAddress() + i, head[i]);
		}
	}


	void InitHook()
	{
		jmpHook.ready();
		assert(jmpHook.getSize() == 14);

		std::memcpy(&head, reinterpret_cast<void*>(baseFunc.GetAddress()), 14);

		WriteJmp();
		_MESSAGE("Hook initialized");
	}
}
