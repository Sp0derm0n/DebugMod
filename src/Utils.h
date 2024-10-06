#pragma once

namespace Utils
{
	void TPSGetCurrentRotation(RE::ThirdPersonState* a_tps, RE::NiQuaternion& a_rotation);
	void FPSGetCurrentRotation(RE::FirstPersonState* a_fps, RE::NiQuaternion& a_rotation);
	void FreeCamStateGetCurrentRotation(RE::FreeCameraState* a_fcs, RE::NiQuaternion& a_rotation);
	void FPSGetCurrentPosition(RE::FirstPersonState* a_fps, RE::NiPoint3& a_position);
	void TPSGetCurrentPosition(RE::ThirdPersonState* a_tps, RE::NiPoint3& a_position);

	RE::NiQuaternion quaternionMult(RE::NiQuaternion q1, RE::NiQuaternion q2);
	RE::NiQuaternion quaternionMult(RE::NiQuaternion q, float a);
	RE::NiQuaternion quaternionUnit(RE::NiQuaternion q);
	RE::NiPoint3 vectorRotation(RE::NiPoint3 a_vector, RE::NiQuaternion a_quaternion);
	float quaternionNorm(RE::NiQuaternion q);
	
	struct ScreenPositionData
	{
		RE::NiPoint2 position;
		float scale;
		bool isBehindCamera;
	};

	ScreenPositionData worldPositionToScreenPosition(RE::NiPoint3 a_position, RE::NiPoint3 a_cameraPosition, float a_FOV, RE::NiPoint3 a_cameraDirectionUnitVector, RE::NiPoint3 a_cameraWidthUnitVector, RE::NiPoint3 a_cameraHeightUnitVector, float a_canvasWidth, float a_canvasHeight);
}