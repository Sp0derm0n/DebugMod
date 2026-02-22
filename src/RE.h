#pragma once

namespace RE
{
	class hkpConvexTransformShapeBase : public hkpConvexShape
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShapeBase;
			inline static constexpr auto VTABLE = VTABLE_hkpConvexTransformShapeBase;
			
			~hkpConvexTransformShapeBase() override; // 00

			hkpSingleShapeContainer childShape; // 28
			mutable int32_t childSize;			// 38
	};
	static_assert(sizeof(hkpConvexTransformShapeBase) == 0x40);

	class hkpConvexTransformShape : public hkpConvexTransformShapeBase
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexTransformShape;
			inline static constexpr auto VTABLE = VTABLE_hkpConvexTransformShape;

			~hkpConvexTransformShape() override; // 00

			// hkTransform
			hkTransform transform;		// 40
	};
	static_assert(sizeof(hkpConvexTransformShape) == 0x80);

	class hkpConvexVerticesConnectivityUtil
	{
		public:
			struct Edge
			{
				uint32_t start;		// -1 mean not set
				uint32_t end;
			};

			struct FaceEdge
			{
				int startIndex;
				int endIndex;
				FaceEdge* next;
			};

			struct vertexInfo
			{
				uint32_t newIndex;
				uint32_t outside;
				float side;
			};

			static hkResult ensureConnectivity(const hkpConvexVerticesShape* shape);
	};
	static_assert(sizeof(hkpConvexVerticesConnectivityUtil) == 0x1);

	class hkpConvexVerticesConnectivity : public hkReferencedObject
	{
		public:
			inline static constexpr auto RTTI = RTTI_hkpConvexVerticesConnectivity;
			inline static constexpr auto VTABLE = VTABLE_hkpConvexVerticesConnectivity;

			// An array holding the vertex indices for all the the faces
			// Each faces indices is stored in a clockwise is outwards order
			// Faces indices are stored one after another - in the order of the faces inside
			// of the hkpConvexVerticesShape
			hkArray<uint16_t> vertexIndices; // 10

			// The number of indices used by each face. Maximum number of indices (and therefore edges) per
			// face is 256.
			hkArray<uint8_t> numVerticesPerFace; // 20

			hkpConvexVerticesConnectivity() {}
			~hkpConvexVerticesConnectivity() override = default;

			__forceinline void* operator new(size_t nbytes)
			{
				RE::hkReferencedObject* b = static_cast<RE::hkReferencedObject*>(RE::hkMemoryRouter::GetInstance().Heap->BlockAlloc(static_cast<uint32_t>(nbytes)));
				b->memSizeAndFlags = static_cast<uint16_t>(nbytes);
				return b;
			}
			__forceinline void* operator new(size_t, void* p)
			{
				return p;
			}
			__forceinline void* operator new[](size_t, void* p)
			{
				((*((int*)0)) = 0);
				return p;
			}

			__forceinline void operator delete(void* p)
			{
				SKSE::log::info("deleting");
				RE::hkReferencedObject* b = static_cast<hkReferencedObject*>(p);
				RE::hkMemoryRouter::GetInstance().Heap->BlockFree(p, b->memSizeAndFlags);
			}
	};
	static_assert(sizeof(hkpConvexVerticesConnectivity) == 0x30);

	// Belonging to hkpConvexVerticesShape
	void GetOriginalVertices(const RE::hkpConvexVerticesShape* a_shape, RE::hkArray<RE::hkVector4>& a_verticesOut);
	void GetFirstVertex(const RE::hkpConvexVerticesShape* a_shape, RE::hkVector4& a_vertexOut);

}
