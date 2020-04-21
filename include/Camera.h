#pragma once


class CameraController
{
public:
	static CameraController* GetSingleton()
	{
		static CameraController singleton;
		return std::addressof(singleton);
	}

	void Rotate() noexcept;
	void Restore() const noexcept;
	
	CameraController(const CameraController&) = delete;
	CameraController(CameraController&&) = delete;
	CameraController& operator=(const CameraController&) = delete;
	CameraController& operator=(CameraController&&) = delete;
	
private:
	float _cameraRotationData[3][3];
	
	CameraController() = default;
	~CameraController() = default;
};