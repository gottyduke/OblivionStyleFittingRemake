#pragma once


namespace RE
{
	class InventoryEntryData;
}


namespace Hooks
{
	
	inline void WriteJmp();
	inline void WriteRet();
	
	void __fastcall HookUpdateItem3D(void* a_this, RE::InventoryEntryData* a_entry);
	void InitHook();
}
