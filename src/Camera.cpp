#include "Camera.h"

#include <cmath>
#include <thread>
#include <chrono>
#include "SKSE/RegistrationSet.h"

#include "RE/Skyrim.h"


namespace RE
{
	void SetEulerAnglesXYZ(NiMatrix3& a_out, NiPoint3& a_angles)
	{
		const auto ch = cos(a_angles.x);
		const auto sh = sin(a_angles.x);
		const auto ca = cos(a_angles.y);
		const auto sa = sin(a_angles.y);
		const auto cb = cos(a_angles.z);
		const auto sb = sin(a_angles.z);

		a_out.entry[0][0] = ch * ca;
		a_out.entry[0][1] = sh * sb - ch * sa * cb;
		a_out.entry[0][2] = ch * sa * sb + sh * cb;
		a_out.entry[1][0] = sa;
		a_out.entry[1][1] = ca * cb;
		a_out.entry[1][2] = -ca * sb;
		a_out.entry[2][0] = -sh * ca;
		a_out.entry[2][1] = sh * sa * cb + ch * sb;
		a_out.entry[2][2] = -sh * sa * sb + ch * cb;
	}
}


// maybe implement a setting interface
constexpr auto TPS_Y = 0.5f;
constexpr auto TPS_Z = 0.1f;
constexpr auto CAMBAT_Y = 0.3f;
constexpr auto CAMBAT_Z = 0.3f;
constexpr auto PI = 3.14159265358979323846;

// basically a refractor + re-syntax version of Beinz's code
void CameraController::Rotate() noexcept
{
	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto camera = RE::PlayerCamera::GetSingleton();
	auto node = camera->cameraRoot;

	if (!player || !camera || !node) {
		_DMESSAGE("NULL");
		return;
	}

	RE::NiPoint3 rotation;
	node->world.rotate.ToEulerAnglesXYZ(rotation);

	std::memcpy(_cameraRotationData, node->world.rotate.entry, 3 * 3 * sizeof(float));

	const auto focus = player->GetPosition() - node->world.translate;

	if (player->IsWeaponDrawn()) {
		rotation.y += CAMBAT_Y * (focus.y > 0 ? 1 : -1);
		rotation.z -= CAMBAT_Z;
	} else {
		rotation.y += TPS_Y * (focus.y > 0 ? 1 : -1);
		rotation.z -= TPS_Z;
	}

	SetEulerAnglesXYZ(node->world.rotate, rotation);

	auto tps = static_cast<RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());
	camera->SetState(tps);

	// TODO
}


void CameraController::Restore() const noexcept
{
	const auto camera = RE::PlayerCamera::GetSingleton();
	auto node = camera->cameraRoot;

	//std::memcpy(node->world.rotate.entry, _cameraRotationData, 3 * 3 * sizeof(float));
}
