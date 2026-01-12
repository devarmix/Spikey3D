#include <Engine/Core/Math.h>

float Spikey::Math::Clamp(float value, float min, float max) {

	if (value < min) value = min;
	else if (value > max) value = max;

	return value;
}

int32 Spikey::Math::Clamp(int32 value, int32 min, int32 max) {

	if (value < min) value = min;
	else if (value > max) value = max;

	return value;
}

uint32 Spikey::Math::Clamp(uint32 value, uint32 min, uint32 max) {

	if (value < min) value = min;
	else if (value > max) value = max;

	return value;
}

float Spikey::Math::Lerp(float a, float b, float t) {
	return a + (b - a) * Clamp(t, 0.f, 1.f);
}

float Spikey::Math::LerpUnclamped(float a, float b, float t) {
	return a + (b - a) * t;
}

float Spikey::Math::MoveTowards(float current, float target, float maxDelta) {

	if (std::abs(target - current) <= maxDelta) {
		return target;
	}

	float sign = (target - current) < 0 ? -1.f : 1.f;
	return current + sign * maxDelta;
}

float Spikey::Math::SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {

	smoothTime = std::max(0.0001f, smoothTime);
	float num = 2 / smoothTime;
	float num2 = num * deltaTime;
	float num3 = 1 / (1 + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
	float value = current - target;
	float num4 = target;
	float num5 = maxSpeed * smoothTime;
	value = Clamp(value, 0 - num5, num5);
	target = current - value;
	float num6 = (currentVelocity + num * value) * deltaTime;
	currentVelocity = (currentVelocity - num * num6) * num3;
	float num7 = target + (value + num6) * num3;
	if (num4 - current > 0 == num7 > num4)
	{
		num7 = num4;
		currentVelocity = (num7 - num4) / deltaTime;
	}

	return num7;
}

Vec2 Spikey::Math::UpVe2() { return Vec2(0.f, 1.f); }

Vec2 Spikey::Math::DownVec2() { return Vec2(0.f, -1.f); }

Vec2 Spikey::Math::LeftVec2() { return Vec2(-1.f, 0.f); }

Vec2 Spikey::Math::RightVec2() { return Vec2(1.f, 0.f); }

Vec2 Spikey::Math::Lerp(const Vec2& a, const Vec2& b, float t) {

	t = Clamp(t, 0.f, 1.f);
	return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

Vec2 Spikey::Math::LerpUnclamped(const Vec2& a, const Vec2& b, float t) {
	return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

Vec2 Spikey::Math::MoveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta) {

	float num = target.x - current.x;
	float num2 = target.y - current.y;
	float num3 = num * num + num2 * num2;
	if (num3 == 0.f || (maxDistanceDelta >= 0.f && num3 <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	float num4 = std::sqrt(num3);
	return Vec2(current.x + num / num4 * maxDistanceDelta, current.y + num2 / num4 * maxDistanceDelta);
}

Vec2 Spikey::Math::SmoothDamp(const Vec2& current, Vec2 target, Vec2& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {

	smoothTime = std::max(0.0001f, smoothTime);
	float num = 2.f / smoothTime;
	float num2 = num * deltaTime;
	float num3 = 1.f / (1.f + num2 + 0.48f * num2 * num2 + 0.235f * num2 * num2 * num2);
	float num4 = current.x - target.x;
	float num5 = current.y - target.y;
	Vec2 vector = target;
	float num6 = maxSpeed * smoothTime;
	float num7 = num6 * num6;
	float num8 = num4 * num4 + num5 * num5;
	if (num8 > num7)
	{
		float num9 = std::sqrt(num8);
		num4 = num4 / num9 * num6;
		num5 = num5 / num9 * num6;
	}

	target.x = current.x - num4;
	target.y = current.y - num5;
	float num10 = (currentVelocity.x + num * num4) * deltaTime;
	float num11 = (currentVelocity.y + num * num5) * deltaTime;
	currentVelocity.x = (currentVelocity.x - num * num10) * num3;
	currentVelocity.y = (currentVelocity.y - num * num11) * num3;
	float num12 = target.x + (num4 + num10) * num3;
	float num13 = target.y + (num5 + num11) * num3;
	float num14 = vector.x - current.x;
	float num15 = vector.y - current.y;
	float num16 = num12 - vector.x;
	float num17 = num13 - vector.y;
	if (num14 * num16 + num15 * num17 > 0.f)
	{
		num12 = vector.x;
		num13 = vector.y;
		currentVelocity.x = (num12 - vector.x) / deltaTime;
		currentVelocity.y = (num13 - vector.y) / deltaTime;
	}

	return Vec2(num12, num13);
}

Vec3 Spikey::Math::UpVec3() { return Vec3(0.f, 1.f, 0.f); }

Vec3 Spikey::Math::DownVec3() { return Vec3(0.f, -1.f, 0.f); }

Vec3 Spikey::Math::LeftVec3() { return Vec3(-1.f, 0.f, 0.f); }

Vec3 Spikey::Math::RightVec3() { return Vec3(1.f, 0.f, 0.f); }

Vec3 Spikey::Math::BackVec3() { return Vec3(0.f, 0.f, -1.f); }

Vec3 Spikey::Math::ForwardVec3() { return Vec3(0.f, 0.f, 1.f); }

Vec3 Spikey::Math::Lerp(const Vec3& a, const Vec3& b, float t) {

	t = Clamp(t, 0.f, 1.f);
	return Vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
}

Vec3 Spikey::Math::LerpUnclamped(const Vec3& a, const Vec3& b, float t) {
	return Vec3(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t);
}

Vec3 Spikey::Math::MoveTowards(const Vec3& current, const Vec3& target, float maxDistanceDelta) {

	float num = target.x - current.x;
	float num2 = target.y - current.y;
	float num3 = target.z - current.z;
	float num4 = num * num + num2 * num2 + num3 * num3;
	if (num4 == 0.f || (maxDistanceDelta >= 0.f && num4 <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	float num5 = std::sqrt(num4);
	return Vec3(current.x + num / num5 * maxDistanceDelta, current.y + num2 / num5 * maxDistanceDelta, current.z + num3 / num5 * maxDistanceDelta);
}

Vec3 Spikey::Math::SmoothDamp(const Vec3& current, Vec3 target, Vec3& currentVelocity, float smoothTime, float maxSpeed, float deltaTime) {

	float num = 0.f;
	float num2 = 0.f;
	float num3 = 0.f;
	smoothTime = std::max(0.0001f, smoothTime);
	float num4 = 2.f / smoothTime;
	float num5 = num4 * deltaTime;
	float num6 = 1.f / (1.f + num5 + 0.48f * num5 * num5 + 0.235f * num5 * num5 * num5);
	float num7 = current.x - target.x;
	float num8 = current.y - target.y;
	float num9 = current.z - target.z;
	Vec3 vector = target;
	float num10 = maxSpeed * smoothTime;
	float num11 = num10 * num10;
	float num12 = num7 * num7 + num8 * num8 + num9 * num9;
	if (num12 > num11)
	{
		float num13 = std::sqrt(num12);
		num7 = num7 / num13 * num10;
		num8 = num8 / num13 * num10;
		num9 = num9 / num13 * num10;
	}

	target.x = current.x - num7;
	target.y = current.y - num8;
	target.z = current.z - num9;
	float num14 = (currentVelocity.x + num4 * num7) * deltaTime;
	float num15 = (currentVelocity.y + num4 * num8) * deltaTime;
	float num16 = (currentVelocity.z + num4 * num9) * deltaTime;
	currentVelocity.x = (currentVelocity.x - num4 * num14) * num6;
	currentVelocity.y = (currentVelocity.y - num4 * num15) * num6;
	currentVelocity.z = (currentVelocity.z - num4 * num16) * num6;
	num = target.x + (num7 + num14) * num6;
	num2 = target.y + (num8 + num15) * num6;
	num3 = target.z + (num9 + num16) * num6;
	float num17 = vector.x - current.x;
	float num18 = vector.y - current.y;
	float num19 = vector.z - current.z;
	float num20 = num - vector.x;
	float num21 = num2 - vector.y;
	float num22 = num3 - vector.z;
	if (num17 * num20 + num18 * num21 + num19 * num22 > 0.f)
	{
		num = vector.x;
		num2 = vector.y;
		num3 = vector.z;
		currentVelocity.x = (num - vector.x) / deltaTime;
		currentVelocity.y = (num2 - vector.y) / deltaTime;
		currentVelocity.z = (num3 - vector.z) / deltaTime;
	}

	return Vec3(num, num2, num3);
}

Vec4 Spikey::Math::Lerp(const Vec4& a, const Vec4& b, float t) {

	t = Clamp(t, 0.f, 1.f);
	return Vec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

Vec4 Spikey::Math::LerpUnclamped(const Vec4& a, const Vec4& b, float t) {
	return Vec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

Vec4 Spikey::Math::MoveTowards(const Vec4& current, const Vec4& target, float maxDistanceDelta) {

	float num = target.x - current.x;
	float num2 = target.y - current.y;
	float num3 = target.z - current.z;
	float num4 = target.w - current.w;
	float num5 = num * num + num2 * num2 + num3 * num3 + num4 * num4;
	if (num5 == 0.f || (maxDistanceDelta >= 0.f && num5 <= maxDistanceDelta * maxDistanceDelta))
	{
		return target;
	}

	float num6 = std::sqrt(num5);
	return Vec4(current.x + num / num6 * maxDistanceDelta, current.y + num2 / num6 * maxDistanceDelta, current.z + num3 / num6 * maxDistanceDelta, current.w + num4 / num6 * maxDistanceDelta);
}

void Spikey::Math::HashCombine(uint64& hash1, uint64 hash2) {
	hash1 ^= hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);
}

uint32 Spikey::Math::PackUnsignedVec4ToUint(const Vec4& v) {

	uint8 r = uint8(v.r * 255.f + 0.5f);
	uint8 g = uint8(v.g * 255.f + 0.5f);
	uint8 b = uint8(v.b * 255.f + 0.5f);
	uint8 a = uint8(v.a * 255.f + 0.5f);

	return (r << 0) | (g << 8) | (b << 16) | (a << 24);
}

Vec4 Spikey::Math::UnpackUintToUnsignedVec4(uint32 packed) {

	float r = float((packed & 0x000000ff) >> 0) / 255.f;
	float g = float((packed & 0x0000ff00) >> 8) / 255.f;
	float b = float((packed & 0x00ff0000) >> 16) / 255.f;
	float a = float((packed & 0xff000000) >> 24) / 255.f;

	return glm::vec4(r, g, b, a);
}

PackedHalf Spikey::Math::PackSignedVec4ToHalf(const Vec4& v) {

	uint16_t x = uint16_t((v.x + 1.f) * 32767.5f + 0.5f);
	uint16_t y = uint16_t((v.y + 1.f) * 32767.5f + 0.5f);
	uint16_t z = uint16_t((v.z + 1.f) * 32767.5f + 0.5f);
	uint16_t w = uint16_t((v.w + 1.f) * 32767.5f + 0.5f);

	PackedHalf out{};
	out.Data[0] = (x << 0) | (y << 16);
	out.Data[1] = (z << 0) | (w << 16);

	return out;
}

Vec4 Spikey::Math::UnpackHalfToSignedVec4(const PackedHalf& packed) {

	float x = (float((packed.Data[0] & 0x0000ffff) >> 0) / 65535.f) * 2.f - 1.f;
	float y = (float((packed.Data[0] & 0xffff0000) >> 16) / 65535.f) * 2.f - 1.f;
	float z = (float((packed.Data[1] & 0x0000ffff) >> 0) / 65535.f) * 2.f - 1.f;
	float w = (float((packed.Data[1] & 0xffff0000) >> 16) / 65535.f) * 2.f - 1.f;

	return glm::vec4(x, y, z, w);
}

Mat4x4 Spikey::Math::GetInfinitePerspectiveMatrix(float fov, float aspect, float nearProj) {

	float f = 1.0f / tanf(fov / 2.0f);
	return Mat4x4(
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, -f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearProj, 0.0f);
}