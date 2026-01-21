#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <Glm/glm.hpp>
#include <Glm/gtx/transform.hpp>
#include <Glm/gtx/quaternion.hpp>

#include <Core/Common.h>

struct PackedHalf {
	uint32 Data[2];
};

namespace Spikey::Math {

	constexpr float PI = 3.14159274f;
	constexpr float E = 2.71828175f;
	constexpr float DEG_TO_RAD = PI / 180;
	constexpr float RAD_TO_DEG = 57.29578f;

	float Clamp(float value, float min, float max);
	int32 Clamp(int32 value, int32 min, int32 max);
	uint32 Clamp(uint32 value, uint32 min, uint32 max);

	float Lerp(float a, float b, float t);
	float LerpUnclamped(float a, float b, float t);
	float MoveTowards(float current, float target, float maxDelta);
	float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

	Vec2 UpVe2();
	Vec2 DownVec2();
	Vec2 LeftVec2();
	Vec2 RightVec2();

	Vec2 Lerp(const Vec2& a, const Vec2& b, float t);
	Vec2 LerpUnclamped(const Vec2& a, const Vec2& b, float t);
	Vec2 MoveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta);
	Vec2 SmoothDamp(const Vec2& current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

	Vec3 UpVec3();
	Vec3 DownVec3();
	Vec3 LeftVec3();
	Vec3 RightVec3();
	Vec3 BackVec3();
	Vec3 ForwardVec3();

	Vec3 Lerp(const Vec3& a, const Vec3& b, float t);
	Vec3 LerpUnclamped(const Vec3& a, const Vec3& b, float t);
	Vec3 MoveTowards(const Vec3& current, const Vec3& target, float maxDistanceDelta);
	Vec3 SmoothDamp(const Vec3& current, Vec3 target, Vec3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime);

	Vec4 Lerp(const Vec4& a, const Vec4& b, float t);
	Vec4 LerpUnclamped(const Vec4& a, const Vec4& b, float t);
	Vec4 MoveTowards(const Vec4& current, const Vec4& target, float maxDistanceDelta);

	uint32 DivideRoundUp(uint32 a, uint32 b);

	uint32 PackUnsignedVec4ToUint(const Vec4& v);
	Vec4 UnpackUintToUnsignedVec4(uint32 packed);
	PackedHalf PackSignedVec4ToHalf(const Vec4& v);
	Vec4 UnpackHalfToSignedVec4(const PackedHalf& packed);
	Mat4x4 GetInfinitePerspectiveMatrix(float fov, float aspect, float nearProj);
}