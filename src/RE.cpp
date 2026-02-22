#include "RE.h"

RE::hkResult RE::hkpConvexVerticesConnectivityUtil::ensureConnectivity(const hkpConvexVerticesShape* shape)
{
	using func_t = decltype(&RE::hkpConvexVerticesConnectivityUtil::ensureConnectivity);
	static REL::Relocation<func_t> func{ RELOCATION_ID(78848, 80836) };
	return func(shape);
}

void RE::GetOriginalVertices(const RE::hkpConvexVerticesShape* a_shape, RE::hkArray<RE::hkVector4>& a_verticesOut)
{
	using func_t = decltype(&RE::GetOriginalVertices);
	static REL::Relocation<func_t> func{ RELOCATION_ID(64067, 65093) };
	return func(a_shape, a_verticesOut);
}

void RE::GetFirstVertex(const RE::hkpConvexVerticesShape* a_shape, RE::hkVector4& a_vertexOut)
{
	using func_t = decltype(&RE::GetFirstVertex);
	static REL::Relocation<func_t> func{ RELOCATION_ID(64078, 65104) };
	return func(a_shape, a_vertexOut);
}