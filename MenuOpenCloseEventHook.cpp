#include "MenuOpenCloseEventHook.h"
#include "skse64/GameExtraData.h"
#include "skse64/GameMenus.h"
#include "skse64/NiNodes.h"
#include "skse64/skse64_common/SafeWrite.h"
#include "REL/Relocation.h"

MenuOpenCloseEventHook* MenuOpenCloseEventHook::instance = nullptr;
bool MenuOpenCloseEventHook::actionRequested = false;
Actor* MenuOpenCloseEventHook::actionTarget = nullptr;

bool helperWasOpen = false;
BSFixedString closeCalledBy;

MenuOpenCloseEventHook::~MenuOpenCloseEventHook() {
}

void MenuOpenCloseEventHook::RequestAction(Actor* a) {
	actionRequested = true;
	actionTarget = a;
}

void MenuOpenCloseEventHook::InitHook() {
	if (instance)
		delete(instance);
	instance = new MenuOpenCloseEventHook();
	MenuManager* mm = MenuManager::GetSingleton();
	if (mm) {
		mm->MenuOpenCloseEventDispatcher()->AddEventSink((BSTEventSink<MenuOpenCloseEvent>*)instance);
		_MESSAGE("MenuOpenCloseEventHook added to the sink.");
	}
}

void MenuOpenCloseEventHook::ResetHook() {
	actionRequested = false;
	actionTarget = nullptr;
	helperWasOpen = false;
}



void MenuOpenCloseEventHook::UpdateItem3D(InventoryEntryData* objDesc)
{
	if (!objDesc)
		return;

	TESForm* form = objDesc->type;
	if (form)
	{
		//UINT32 formID = form->formID;
		//UINT8 formType = form->formType;
		//_MESSAGE("[0x%08x] %0d", formID, formType);

		//const char* name = form->GetName();
		//_MESSAGE(name);

		switch (form->GetFormType())
		{
		case kFormType_Armor:
		case kFormType_Ammo:
		case kFormType_Weapon:
			CALL_MEMBER_FN(Inventory3DManager::GetSingleton(), Clear3D)();
			break;
			//case kFormType_Armor:
			//{
			//	//_MESSAGE("kFormType_Armor");
			//	//TESObjectARMO* pArmor = DYNAMIC_CAST(form, TESForm, TESObjectARMO);
			//	//if (pArmor)
			//	//{
			//	//	CALL_MEMBER_FN(EquipManager::GetSingleton(), EquipItem)((Actor*)* g_thePlayer, form, objDesc->extraData->GetNthItem(0), objDesc->countDelta, NULL, false, false, false, NULL);
			//	//}
			//}
			//break;

			//case kFormType_Ammo:
			//{
			//	//_MESSAGE("kFormType_Ammo");
			//	//TESAmmo* pAmmo = DYNAMIC_CAST(form, TESForm, TESAmmo);
			//	//if (pAmmo)
			//	//{
			//	//	CALL_MEMBER_FN(EquipManager::GetSingleton(), EquipItem)((Actor*)* g_thePlayer, form, objDesc->extraData->GetNthItem(0), objDesc->countDelta, NULL, false, false, false, NULL);
			//	//}
			//}
			//break;

			//case kFormType_Weapon:
			//{
			//	//_MESSAGE("kFormType_Weapon");
			//	//TESObjectWEAP* pWeapon = DYNAMIC_CAST(form, TESForm, TESObjectWEAP);
			//	//if (pWeapon)
			//	//{
			//	//	_MESSAGE("equipType");
			//	//	BGSEquipSlot* equipSlot = pWeapon->equipType.GetEquipSlot();
			//	//	_MESSAGE("equipSlot");
			//	//	if (equipSlot)
			//	//		CALL_MEMBER_FN(EquipManager::GetSingleton(), EquipItem)((Actor*)* g_thePlayer, form, objDesc->extraData->GetNthItem(0), objDesc->countDelta, equipSlot, false, false, false, NULL);
			//	//	else
			//	//		CALL_MEMBER_FN(EquipManager::GetSingleton(), EquipItem)((Actor*)* g_thePlayer, form, objDesc->extraData->GetNthItem(0), objDesc->countDelta, NULL, false, false, false, NULL);
			//	//}
			//}
			//break;

		default:
			//DEFINE_MEMBER_FN(UpdateItem3D, void, 0x00867C00, InventoryEntryData * objDesc);
			CALL_MEMBER_FN(Inventory3DManager::GetSingleton(), UpdateMagic3D)(form, 0);
			//WriteRelJump(0x00867C00, GetFnAddr(&MenuEventHooker::UpdateItem3D));
			break;
		}
	}
}

// math.h
#define MATH_PI       3.14159265358979323846

// Converted from Java to C
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
void GetEulerAngles(NiMatrix33 * mat, float* heading, float* attitude, float* bank)
{
	if (mat->data[1][0] > 0.998) { // singularity at north pole
		*heading = atan2(mat->data[0][2], mat->data[2][2]);
		*attitude = MATH_PI / 2;
		*bank = 0;
	}
	else if (mat->data[1][0] < -0.998) { // singularity at south pole
		*heading = atan2(mat->data[0][2], mat->data[2][2]);
		*attitude = -MATH_PI / 2;
		*bank = 0;
	}
	else {
		*heading = atan2(-mat->data[2][0], mat->data[0][0]);
		*bank = atan2(-mat->data[1][2], mat->data[1][1]);
		*attitude = asin(mat->data[1][0]);
	}
}

// Converted from Java to C
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToMatrix/index.htm
void SetEulerAngles(NiMatrix33* mat, float heading, float attitude, float bank)
{
	double ch = cos(heading);
	double sh = sin(heading);
	double ca = cos(attitude);
	double sa = sin(attitude);
	double cb = cos(bank);
	double sb = sin(bank);

	mat->data[0][0] = ch * ca;
	mat->data[0][1] = sh * sb - ch * sa * cb;
	mat->data[0][2] = ch * sa * sb + sh * cb;
	mat->data[1][0] = sa;
	mat->data[1][1] = ca * cb;
	mat->data[1][2] = -ca * sb;
	mat->data[2][0] = -sh * ca;
	mat->data[2][1] = sh * sa * cb + ch * sb;
	mat->data[2][2] = -sh * sa * sb + ch * cb;
}

float dot(NiPoint3 a, NiPoint3 b)  //calculates dot product of a and b
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float mag(NiPoint3 a)  //calculates magnitude of a
{
	return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

int angle(NiPoint3 v1, NiPoint3 v2)
{
	return std::acos(dot(v1, v2) / (mag(v1) * mag(v2)));
}

bool MenuOpenCloseEventHook::OnInventoryOpen()
{
	_MESSAGE("OnInventoryOpen");
	//Inventory3DManager::UpdateItem3D
	REL::Offset<std::uintptr_t> vTable(0x00887970);
	_Update = vTable.WriteVFunc(0xE9, &MenuOpenCloseEventHook::UpdateItem3D);
	_MESSAGE("UpdateItem3D hooked");

	PlayerCamera* cam = PlayerCamera::GetSingleton();
	if (!cam) {
		_MESSAGE("PlayerCamera is null");
		return false;
	}
	//_MESSAGE("Camera State = %d", cam->cameraState->stateId);
	//if (cam->cameraState->stateId != PlayerCamera::kCameraState_ThirdPerson2)
	//{
	//	//cam->cameraState->stateId = PlayerCamera::kCameraState_ThirdPerson2;
	//	SKSECameraEvent* evnt = new SKSECameraEvent(cam->cameraStates[cam->cameraState->stateId], cam->cameraStates[PlayerCamera::kCameraState_ThirdPerson2]);
	//	g_cameraEventHandler.ReceiveEvent(evnt, NULL);
	//}

	NiNode* cameraNode = cam->cameraNode;

	PlayerCharacter* pPC = (*g_thePlayer);
	//CALL_MEMBER_FN(PlayerCamera::GetSingleton(), UpdateThirdPerson)(pPC->actorState.IsWeaponDrawn());

	NiPoint3 rotation;
	GetEulerAngles(&(cameraNode->m_worldTransform.rot), &rotation.x, &rotation.y, &rotation.z);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			m_camRotHolder[i][j] = cameraNode->m_worldTransform.rot.data[i][j];

	NiPoint3 lookPoint = pPC->pos - cameraNode->m_worldTransform.pos;

	float f3rdy = 0.5f;
	float f3rdz = 0.1f;
	float fcombaty = 0.3f;
	float fcombatz = 0.3f;

	if (lookPoint.y > 0)
	{
		if (pPC->actorState.IsWeaponDrawn())
			rotation.y += fcombaty;
		else
			rotation.y += f3rdy;
		//if (rotation.y >= MATH_PI)
		//	rotation.y += -MATH_PI;
	}
	else
	{
		if (pPC->actorState.IsWeaponDrawn())
			rotation.y += -fcombaty;
		else
			rotation.y += -f3rdy;
		//if (rotation.y <= -MATH_PI)
		//	rotation.y += MATH_PI;
	}
	if (pPC->actorState.IsWeaponDrawn())
		rotation.z += -fcombatz;
	else
		rotation.z += -f3rdz;

	SetEulerAngles(&(cameraNode->m_worldTransform.rot), rotation.x, rotation.y, rotation.z);

	//pPC->rot.y = angle(pPC->pos , cameraNode->m_worldTransform.pos);

	return true;
}

void MenuOpenCloseEventHook::OnInventoryClose()
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			PlayerCamera::GetSingleton()->cameraNode->m_worldTransform.rot.data[i][j] = m_camRotHolder[i][j];

}

EventResult MenuOpenCloseEventHook::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* src) {
	UIStringHolder* uistr = UIStringHolder::GetSingleton();
	if (uistr)
		if (evn->menuName == uistr->inventoryMenu ||
			evn->menuName == uistr->barterMenu ||
			evn->menuName == uistr->containerMenu
			) {
		if (evn->opening)
		{
			_MESSAGE("Inventory open");
			OnInventoryOpen();
		}
		else
		{
			OnInventoryClose();
			_MESSAGE("Inventory close");
		}
		}
	return kEvent_Continue;
}
