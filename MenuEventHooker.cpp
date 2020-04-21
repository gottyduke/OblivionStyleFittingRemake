#include "MenuEventHooker.h"
#include "skse64/GameMenus.h"
#include "skse64_common/SafeWrite.h"

#include "skse64/GameExtraData.h"
#include "skse64/GameMenus.h"


#define Inventory3DManager_UpdateItem3D_Address 0x00867C00
#define Hooked_UpdateItem3D_Address	GetFnAddr(&UpdateItem3D)


TESCameraController* TESCameraController::GetSingleton()
{
	return (TESCameraController*)0x01277C58;
}


MenuEventHooker* MenuEventHooker::instance = nullptr;
MenuEventHooker::~MenuEventHooker() {}


void MenuEventHooker::InitHook() {
	if (instance)
		delete(instance);
	instance = new MenuEventHooker();
	MenuManager* mm = MenuManager::GetSingleton();
	if (mm) {
		mm->MenuOpenCloseEventDispatcher()->AddEventSink((BSTEventSink<MenuOpenCloseEvent>*)instance);
		_MESSAGE("MenuEventHooker added to the sink.");

		//Inventory3DManager::UpdateItem3D
		WriteRelJump(Inventory3DManager_UpdateItem3D_Address, Hooked_UpdateItem3D_Address);
		_MESSAGE("UpdateItem3D hooked");
	}
}


void MenuEventHooker::UpdateItem3D(InventoryEntryData* objDesc)
{
	if (!objDesc)
		return;

	TESForm* form = objDesc->type;
	if (form)
	{
		switch (form->GetFormType())
		{
#pragma region WearPreview
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
#pragma endregion	//WearPreview

		case kFormType_Armor:
		case kFormType_Ammo:
		case kFormType_Weapon:
			CALL_MEMBER_FN(Inventory3DManager::GetSingleton(), Clear3D)();
			break;
		default:
			CALL_MEMBER_FN(Inventory3DManager::GetSingleton(), UpdateMagic3D)(form, 0);
			break;
		}
	}
}


#pragma region Math

#define M_PI       3.14159265358979323846

// Converted from Java to C
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToEuler/index.htm
void GetEulerAngles(NiMatrix33 * mat, float* heading, float* attitude, float* bank)
{
	if (mat->data[1][0] > 0.998) { // singularity at north pole
		*heading = atan2(mat->data[0][2], mat->data[2][2]);
		*attitude = M_PI / 2;
		*bank = 0;
	}
	else if (mat->data[1][0] < -0.998) { // singularity at south pole
		*heading = atan2(mat->data[0][2], mat->data[2][2]);
		*attitude = -M_PI / 2;
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
#pragma endregion //Math


#pragma region Camera

void GetCameraPos(NiPoint3* pos)
{
	PlayerCharacter* player = *g_thePlayer;
	PlayerCamera* camera = PlayerCamera::GetSingleton();
	float x, y, z;

	FirstPersonState* fps = (FirstPersonState*)camera->cameraStates[camera->kCameraState_FirstPerson];
	ThirdPersonState* tps = (ThirdPersonState*)camera->cameraStates[camera->kCameraState_ThirdPerson2];
	if (camera->cameraState == fps || camera->cameraState == tps)
	{
		NiNode* node = camera->cameraNode;
		if (node)
		{
			x = node->m_worldTransform.pos.x;
			y = node->m_worldTransform.pos.y;
			z = node->m_worldTransform.pos.z;
			/*
			NiAVObject* obj = node->m_children.m_data[0];
			if (node->m_children.m_size > 0 && node->m_children.m_data)
			{
				NiAVObject* obj = node->m_children.m_data[0];
				if (obj)
				{
					x = obj->m_worldTransform.pos.x;
					y = obj->m_worldTransform.pos.y;
					z = obj->m_worldTransform.pos.z;
				}
			}
			*/
		}
	}
	else
	{
		NiPoint3 playerPos;

		player->GetMarkerPosition(&playerPos);
		z = player->pos.z;
		x = player->pos.x;
		y = player->pos.y;
	}

	pos->x = x;
	pos->y = y;
	pos->z = z;
}

const float WaitDefault = 20;
void SetPlayerAngle(float rotZ, float rotX, float wait = WaitDefault)
{
	PlayerCharacter* player = *g_thePlayer;
	TESCameraController* controller = TESCameraController::GetSingleton();

	if (wait < WaitDefault)
		wait = WaitDefault;

	controller->Rotate(player->rot.z, player->rot.x, rotZ, rotX, wait, 0);
}

void LookAt(float posX, float posY, float posZ, float wait = WaitDefault)
{
	PlayerCharacter* player = *g_thePlayer;
	NiPoint3 cameraPos;
	double x, y, z, xy;
	double rotZ, rotX;

	GetCameraPos(&cameraPos);

	x = posX - cameraPos.x;
	y = posY - cameraPos.y;
	z = posZ - cameraPos.z;

	xy = sqrt(x * x + y * y);
	rotZ = atan2(x, y);
	rotX = atan2(-z, xy);

	if (rotZ - player->rot.z > M_PI)
		rotZ -= M_PI * 2;
	else if (rotZ - player->rot.z < -M_PI)
		rotZ += M_PI * 2;

	SetPlayerAngle(rotZ, rotX, wait);
}

struct AngleZX
{
	double  z;
	double  x;
	double  distance;
};

void GetAngle(const NiPoint3& from, const NiPoint3& to, AngleZX* angle)
{
	float x = to.x - from.x;
	float y = to.y - from.y;
	float z = to.z - from.z;
	float xy = sqrt(x * x + y * y);

	angle->z = atan2(x, y);
	angle->x = atan2(-z, xy);
	angle->distance = sqrt(xy * xy + z * z);
}

// unusing
void RotateCameraMatrix()
{
	PlayerCamera* pCam = PlayerCamera::GetSingleton();
	NiNode* cameraNode = PlayerCamera::GetSingleton()->cameraNode;
	PlayerCharacter* pPC = (*g_thePlayer);

	//AngleZX baseAngle;
	//GetAngle(cameraNode->m_worldTransform.pos, pPC->pos, &baseAngle);
	//_MESSAGE("Angle pPC->pos[%f,%f,%f] cameraNode->m_worldTransform.pos[%f,%f,%f] baseAngle[%f,%f]", 
	//	pPC->pos.x, pPC->pos.y, pPC->pos.z, cameraNode->m_worldTransform.pos.x, cameraNode->m_worldTransform.pos.y, cameraNode->m_worldTransform.pos.z, baseAngle.x, baseAngle.z);

	NiPoint3 rotation;
	GetEulerAngles(&(cameraNode->m_worldTransform.rot), &rotation.x, &rotation.y, &rotation.z);
	//memcpy(m_camRotHolder, cameraNode->m_worldTransform.rot.data, sizeof(float) * 9);

	const float f3rdy = 0.5f;
	const float f3rdz = 0.1f;
	const float fcombaty = 0.3f;
	const float fcombatz = 0.3f;

	NiPoint3 lookVector = pPC->pos - cameraNode->m_worldTransform.pos;
	if (lookVector.y > 0)
	{
		if (pPC->actorState.IsWeaponDrawn())
			rotation.y += fcombaty;
		else
			rotation.y += f3rdy;
		if (rotation.y >= M_PI)
			rotation.y += -2 * M_PI;
	}
	else
	{
		if (pPC->actorState.IsWeaponDrawn())
			rotation.y += -fcombaty;
		else
			rotation.y += -f3rdy;
		if (rotation.y <= -M_PI)
			rotation.y += 2 * M_PI;
	}
	if (pPC->actorState.IsWeaponDrawn())
		rotation.z += -fcombatz;
	else
		rotation.z += -f3rdz;

	SetEulerAngles(&(cameraNode->m_worldTransform.rot), rotation.x, rotation.y, rotation.z);
}

void MenuEventHooker::ForceThirdPersonInstant()
{
	PlayerCamera* pCam = PlayerCamera::GetSingleton();
	if (pCam->cameraState->stateId < PlayerCamera::kCameraState_ThirdPerson2)
	{
		m_camStateId = pCam->cameraState->stateId;
		m_b3rdForced = true;

		ThirdPersonState* pCamState = (ThirdPersonState*)pCam->cameraStates[PlayerCamera::kCameraState_ThirdPerson2];
		CALL_MEMBER_FN(pCam, SetCameraState)(pCamState);
		CALL_MEMBER_FN(pCam, UpdateThirdPerson)((*g_thePlayer)->actorState.IsWeaponDrawn());


		PlayerCharacter* pPC = (*g_thePlayer);
		Vector3 vPc = Vector3(0, pPC->rot.y, pPC->rot.z);
		vPc.Normalize();

		//_MESSAGE("Beforce rotation : [%f, %f] [%f, %f]", pCamState->unkA4[2], pCamState->unkA4[3], *(float*)((UInt32)pCamState + 0xAC), *(float*)((UInt32)pCamState + 0xB0));
		//*(float*)((UInt32)pCamState + 0xAC) = *(float*)((UInt32)pCamState + 0xB0) = M_PI;	//diffRotZ;			// AC | diff from player rotZ	//diffRotX;			// B0 | diff from player rotX
		*(float*)((UInt32)pCamState + 0xAC) = M_PI;	//diffRotZ;			// AC | diff from player rotZ	//diffRotX;			// B0 | diff from player rotX
		//_MESSAGE("   After rotation : [%f, %f] [%f, %f]", pCamState->unkA4[2], pCamState->unkA4[3], *(float*)((UInt32)pCamState + 0xAC), *(float*)((UInt32)pCamState + 0xB0));
		pCam->Update();

		//_MESSAGE("Beforce position : [%f, %f] [%f, %f]", pCamState->unk54[0], pCamState->unk54[1], *(float*)((UInt32)pCamState + 0x54), *(float*)((UInt32)pCamState + 0x58));
		*(float*)((UInt32)pCamState + 0x54) = *(float*)((UInt32)pCamState + 0x58) = vPc.y * 20;	//dstPosY;			// 54 | destination	//curPosY;			// 58 | current position
		//_MESSAGE("   After position : [%f, %f] [%f, %f]", pCamState->unk54[0], pCamState->unk54[1], *(float*)((UInt32)pCamState + 0x54), *(float*)((UInt32)pCamState + 0x58));
		pCam->Update();
	}
}

void MenuEventHooker::RotateCamera()
{
	PlayerCamera* pCam = PlayerCamera::GetSingleton();
	ThirdPersonState* pCamState = (ThirdPersonState*)pCam->cameraState;

	//_MESSAGE("Beforce	pCamState->fOverShoulderPos : [%f,%f,%f] basePos[%f,%f,%f] diffRotZ[%f]", 
	//	pCamState->fOverShoulderPosX, pCamState->fOverShoulderCombatAddY, pCamState->fOverShoulderPosZ,
	//	*(float*)((UInt32)pCamState + 0x48), *(float*)((UInt32)pCamState + 0x4C), *(float*)((UInt32)pCamState + 0x50),
	//	*(float*)((UInt32)pCamState + 0xAC));
	m_camShoulderPosHolder[0] = pCamState->fOverShoulderPosX;
	m_camShoulderPosHolder[1] = pCamState->fOverShoulderCombatAddY;
	m_camShoulderPosHolder[2] = pCamState->fOverShoulderPosZ;
	m_camDiffRotZ = *(float*)((UInt32)pCamState + 0xAC);

	*(float*)((UInt32)pCamState + 0x48) = pCamState->fOverShoulderPosX = -50;
	*(float*)((UInt32)pCamState + 0x4C) = pCamState->fOverShoulderCombatAddY = -10;
	*(float*)((UInt32)pCamState + 0x50) = pCamState->fOverShoulderPosZ = -40;
	*(float*)((UInt32)pCamState + 0xAC) = M_PI -0.3f;	//diffRotZ;
	//_MESSAGE("Atfer		pCamState->fOverShoulderPos : [%f,%f,%f] basePos[%f,%f,%f] diffRotZ[%f]",
	//	pCamState->fOverShoulderPosX, pCamState->fOverShoulderCombatAddY, pCamState->fOverShoulderPosZ,
	//	*(float*)((UInt32)pCamState + 0x48), *(float*)((UInt32)pCamState + 0x4C), *(float*)((UInt32)pCamState + 0x50),
	//	*(float*)((UInt32)pCamState + 0xAC));
	pCam->Update();
	//_MESSAGE("Updated		pCamState->fOverShoulderPos : [%f,%f,%f] basePos[%f,%f,%f] diffRotZ[%f]",
	//	pCamState->fOverShoulderPosX, pCamState->fOverShoulderCombatAddY, pCamState->fOverShoulderPosZ,
	//	*(float*)((UInt32)pCamState + 0x48), *(float*)((UInt32)pCamState + 0x4C), *(float*)((UInt32)pCamState + 0x50),
	//	*(float*)((UInt32)pCamState + 0xAC));
}

void MenuEventHooker::RollbackCamera()
{
	PlayerCamera* pCam = PlayerCamera::GetSingleton();
	ThirdPersonState* pCamState3rd;
	if (m_b3rdForced)
	{
		TESCameraState* pCamState = (TESCameraState*)pCam->cameraStates[m_camStateId];
		CALL_MEMBER_FN(pCam, SetCameraState)(pCamState);
		//CALL_MEMBER_FN(pCam, UpdateThirdPerson)((*g_thePlayer)->actorState.IsWeaponDrawn());
		pCam->Update();

		pCamState3rd = (ThirdPersonState*)pCam->cameraStates[PlayerCamera::kCameraState_ThirdPerson2];
	}
	else {
		pCamState3rd = (ThirdPersonState*)pCam->cameraState;
	}

	*(float*)((UInt32)pCamState3rd + 0x48) = pCamState3rd->fOverShoulderPosX = m_camShoulderPosHolder[0];
	*(float*)((UInt32)pCamState3rd + 0x4C) = pCamState3rd->fOverShoulderCombatAddY = m_camShoulderPosHolder[1];
	*(float*)((UInt32)pCamState3rd + 0x50) = pCamState3rd->fOverShoulderPosZ = m_camShoulderPosHolder[2];
	*(float*)((UInt32)pCamState3rd + 0xAC) = m_camDiffRotZ;
	pCam->Update();

	//_MESSAGE("RollbackThirdPerson pCam->unkD2 : [%f]", pCam->unkD2);
}
#pragma endregion //Camera


//#include "NativeFunctions.h"
bool MenuEventHooker::OnInventoryOpen()
{
	PlayerCamera* pCam = PlayerCamera::GetSingleton();
	if (!pCam || pCam->cameraState == NULL) {
		_MESSAGE("PlayerCamera is null");
		return false;
	}

	m_b3rdForced = false;
	m_camStateId = 0xff;
	ForceThirdPersonInstant();
	RotateCamera();

	//rotate pc
	//pPC->rot.z = angle(cameraNode->m_worldTransform.pos, pPC->pos);
	//pPC->rot.z = angle(cameraNode->m_worldTransform.pos, pPC->GetNiNode()->m_worldTransform.pos);

	//SetLookAt((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, TESObjectREFR * akTarget, false);
	//SetHeadTracking((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, false);
	//CALL_MEMBER_FN(*g_thePlayer, QueueNiNodeUpdate)(false);


	return true;
}

void MenuEventHooker::OnInventoryClose()
{
	//memcpy(PlayerCamera::GetSingleton()->cameraNode->m_worldTransform.rot.data, m_camRotHolder, sizeof(float) * 9);
	RollbackCamera();
}

EventResult MenuEventHooker::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* src) {
	UIStringHolder* uistr = UIStringHolder::GetSingleton();
	if (uistr)
	{
		if (evn->menuName == uistr->inventoryMenu ||
			evn->menuName == uistr->barterMenu ||
			evn->menuName == uistr->containerMenu
			)
		{
			if (evn->opening)
			{
				//_MESSAGE("Inventory open");
				OnInventoryOpen();
			}
			else
			{
				OnInventoryClose();
				//_MESSAGE("Inventory close");
			}
		}
	}
	return kEvent_Continue;
}
