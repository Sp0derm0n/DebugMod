#include "Utils.h"


void Utils::TPSGetCurrentRotation(RE::ThirdPersonState* a_tps, RE::NiQuaternion& a_rotation)
{
	using func_t = decltype(&TPSGetCurrentRotation);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50897) }; //SE ID NOT CORRECT
	return func(a_tps, a_rotation);
}
void Utils::FPSGetCurrentRotation(RE::FirstPersonState* a_fps, RE::NiQuaternion& a_rotation)
{
	using func_t = decltype(&FPSGetCurrentRotation);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50725) }; //SE ID NOT CORRECT
	return func(a_fps, a_rotation);
}
void Utils::FreeCamStateGetCurrentRotation(RE::FreeCameraState* a_fcs, RE::NiQuaternion& a_rotation)
{
	using func_t = decltype(&FreeCamStateGetCurrentRotation);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50744) }; //SE ID NOT CORRECT
	return func(a_fcs, a_rotation);
}
void Utils::FPSGetCurrentPosition(RE::FirstPersonState* a_fps, RE::NiPoint3& a_position)
{
	using func_t = decltype(&FPSGetCurrentPosition);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50726) }; //SE ID NOT CORRECT
	return func(a_fps, a_position);
}
void Utils::TPSGetCurrentPosition(RE::ThirdPersonState* a_tps, RE::NiPoint3& a_position)
{
	using func_t = decltype(&TPSGetCurrentPosition);
	REL::Relocation<func_t> func{ RELOCATION_ID(51246, 50898) }; //SE ID NOT CORRECT
	return func(a_tps, a_position);
}

RE::NiQuaternion Utils::quaternionMult(RE::NiQuaternion q1, RE::NiQuaternion q2)
{
	return RE::NiQuaternion(q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
							q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
							q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
							q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w);
}

RE::NiQuaternion Utils::quaternionMult(RE::NiQuaternion q, float a)
{
	return RE::NiQuaternion(q.w*a, q.x*a, q.y*a, q.z*a);
}

float Utils::quaternionNorm(RE::NiQuaternion q)
{
	return sqrtf(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
}

RE::NiQuaternion Utils::quaternionUnit(RE::NiQuaternion q)
{
	return quaternionMult(q, 1/quaternionNorm(q));
}

RE::NiPoint3 Utils::vectorRotation(RE::NiPoint3 a_vector, RE::NiQuaternion a_quaternion)
{
	RE::NiQuaternion vector = RE::NiQuaternion(0, a_vector.x, a_vector.y, a_vector.z);
	RE::NiQuaternion conjugatedQuaternion = RE::NiQuaternion(a_quaternion.w, -a_quaternion.x, -a_quaternion.y, -a_quaternion.z);
	RE::NiQuaternion rotatedQuaternion = quaternionMult(quaternionMult(a_quaternion, vector), conjugatedQuaternion);
	return RE::NiPoint3(rotatedQuaternion.x, rotatedQuaternion.y, rotatedQuaternion.z);
}


Utils::ScreenPositionData Utils::worldPositionToScreenPosition(RE::NiPoint3 a_position, RE::NiPoint3 a_cameraPosition, float a_FOV, RE::NiPoint3 a_cameraDirectionUnitVector, RE::NiPoint3 a_cameraWidthUnitVector, RE::NiPoint3 a_cameraHeightUnitVector, float a_canvasWidth, float a_canvasHeight)
{

	RE::NiPoint3 cameraToWorldpositionVector = a_position - a_cameraPosition;
	float cameraDirectionVectorLength = cameraToWorldpositionVector * a_cameraDirectionUnitVector; // distance to the plane along the camera direction.

	RE::NiPoint3 centerOfPlaneToWorldpositionVector = cameraToWorldpositionVector - a_cameraDirectionUnitVector; // camera dicrection can be 
	
	auto playerCamera = RE::PlayerCamera::GetSingleton();
	auto fov = playerCamera->GetRuntimeData2().worldFOV; //check 1st person fov
	

	float theta_x = (a_FOV/2 * 3.14159265359)/180; //half the horizontal angle of the camera
	float width = tanf(theta_x)*cameraDirectionVectorLength; // width of the plane in world space units 
	float height = (width*a_canvasHeight)/a_canvasWidth;


	float heightRatio = (centerOfPlaneToWorldpositionVector*a_cameraHeightUnitVector/height + 1)/2;
	float widthRatio = (centerOfPlaneToWorldpositionVector*a_cameraWidthUnitVector/width + 1)/2;

	float scale = 2000; // 200/0.1
	if (cameraToWorldpositionVector.Length() > 0.1) scale = 200/cameraToWorldpositionVector.Length(); // 200 is chosen arbitrarily such that scale is a reasonable value
	if (fabs(a_cameraDirectionUnitVector*centerOfPlaneToWorldpositionVector) < 0.01) scale = 0;
	return ScreenPositionData(RE::NiPoint2(widthRatio*a_canvasWidth, heightRatio*a_canvasHeight), scale, cameraDirectionVectorLength < 0); 	 // < 0 occurs when the world position is behind the camera

}