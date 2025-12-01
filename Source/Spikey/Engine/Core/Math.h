#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <Glm/glm.hpp>
#include <Glm/gtx/transform.hpp>
#include <Glm/gtx/quaternion.hpp>

#include <Engine/Core/Common.h>

using Quaternion = glm::quat;
using Vec2 = glm::vec2;
using Vec2Int = glm::ivec2;
using Vec2Uint = glm::uvec2;
using Vec3 = glm::vec3;
using Vec3Int = glm::ivec3;
using Vec3Uint = glm::uvec3;
using Vec4 = glm::vec4;
using Vec4Int = glm::ivec4;
using Mat2x2 = glm::mat2;
using Mat3x3 = glm::mat3;
using Mat4x4 = glm::mat4;

struct PackedHalf {
	uint32 Data[2];
};

namespace Spikey::Math {

	constexpr float32 PI = 3.14159274f;
	constexpr float32 E = 2.71828175f;
	constexpr float32 DEG_TO_RAD = PI / 180;
	constexpr float32 RAD_TO_DEG = 57.29578f;

	float32 Clamp(float32 value, float32 min, float32 max);
	int32 Clamp(int32 value, int32 min, int32 max);
	uint32 Clamp(uint32 value, uint32 min, uint32 max);

	float32 Lerp(float32 a, float32 b, float32 t);
	float32 LerpUnclamped(float32 a, float32 b, float32 t);
	float32 MoveTowards(float32 current, float32 target, float32 maxDelta);
	float32 SmoothDamp(float32 current, float32 target, float32& currentVelocity, float32 smoothTime, float32 maxSpeed, float32 deltaTime);

	Vec2 UpVe2();
	Vec2 DownVec2();
	Vec2 LeftVec2();
	Vec2 RightVec2();

	Vec2 Lerp(const Vec2& a, const Vec2& b, float32 t);
	Vec2 LerpUnclamped(const Vec2& a, const Vec2& b, float32 t);
	Vec2 MoveTowards(const Vec2& current, const Vec2& target, float32 maxDistanceDelta);
	Vec2 SmoothDamp(const Vec2& current, Vec2 target, Vec2& currentVelocity, float32 smoothTime, float32 maxSpeed, float32 deltaTime);

	Vec3 UpVec3();
	Vec3 DownVec3();
	Vec3 LeftVec3();
	Vec3 RightVec3();
	Vec3 BackVec3();
	Vec3 ForwardVec3();

	Vec3 Lerp(const Vec3& a, const Vec3& b, float32 t);
	Vec3 LerpUnclamped(const Vec3& a, const Vec3& b, float32 t);
	Vec3 MoveTowards(const Vec3& current, const Vec3& target, float32 maxDistanceDelta);
	Vec3 SmoothDamp(const Vec3& current, Vec3 target, Vec3& currentVelocity, float32 smoothTime, float32 maxSpeed, float32 deltaTime);

	Vec4 Lerp(const Vec4& a, const Vec4& b, float32 t);
	Vec4 LerpUnclamped(const Vec4& a, const Vec4& b, float32 t);
	Vec4 MoveTowards(const Vec4& current, const Vec4& target, float32 maxDistanceDelta);

	template<typename T>
	constexpr void HashCombine(uint64& seed, const T& v) {
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	uint32 DivideRoundUp(uint32 a, uint32 b);

	uint32 PackUnsignedVec4ToUint(const Vec4& v);
	Vec4 UnpackUintToUnsignedVec4(uint32 packed);
	PackedHalf PackSignedVec4ToHalf(const Vec4& v);
	Vec4 UnpackHalfToSignedVec4(const PackedHalf& packed);
	Mat4x4 GetInfinitePerspectiveMatrix(float32 fov, float32 aspect, float32 nearProj);
}