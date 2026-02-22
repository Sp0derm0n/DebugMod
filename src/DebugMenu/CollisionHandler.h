#pragma once

#include "DebugItem.h"
#include "Renderer/Renderer.h"

namespace DebugMenu
{

	class CollisionHandler : public DebugItem
	{
		public:
			CollisionHandler();

			void Draw() override;
			void Reset();
			void UpdateVisibleCollisions(RE::TESObjectREFR* a_consoleRef);
			void HideAllCollisions();	
			void ResetSelectedRefs();

		private:
			struct CollisionObject
			{
				

				RE::NiAVObject*		parent = nullptr;
				const RE::hkpShape*	hkpShape = nullptr;
				float				collisionScale = 200.0f; // sat large to identify objects not getting the proper scale
				bool				useRBTransform = false;
				bool				useLocalTransform = false;
				RE::hkQuaternion	rbRotation{ RE::hkVector4( 0.0f, 0.0f, 0.0f, 1.0f ) };
				RE::NiPoint3		rbOffset{ 0.0f, 0.0f, 0.0f };
				RE::NiPoint3		localOffset{ 0.0f, 0.0f, 0.0f };
				RE::NiMatrix3		localRotation = RE::NiMatrix3();

				bool				isCharController = false;
				RE::NiPoint3		charControllerOffset{ 0.0f, 0.0f, 0.0f };
				RE::NiMatrix3		charControllerRotation = RE::NiMatrix3();

				RE::NiPoint3		testOffset{ 0.0f, 0.0f, 0.0f };

				CollisionObject(RE::NiAVObject* a_parent) : parent(a_parent) {}
				RE::NiPoint3		GetWorldPos(RE::NiPoint3 a_position);
			};

			struct CollisionLine
			{
				vec3u start;
				vec3u end;
				vec4u color;
			};

			struct CollisionTriangle
			{
				CollisionTriangle(vec3u a_p1, vec3u a_p2, vec3u a_p3) : point1(a_p1), point2(a_p2), point3(a_p3), color(MCM::settings::collisionColor) {};
				CollisionTriangle(vec3u a_p1, vec3u a_p2, vec3u a_p3, vec4u a_color) : point1(a_p1), point2(a_p2), point3(a_p3), color(a_color) {};

				vec3u point1;
				vec3u point2;
				vec3u point3;
				vec4u color;
			};

			struct CollisionMesh
			{
				CollisionMesh(std::vector<CollisionTriangle>& a_triangles);

				std::shared_ptr<Renderer::MeshDrawer> meshDrawer = nullptr;
			};

			class RefCollisionData
			{
				public:

					RE::TESObjectREFRPtr		ref = nullptr;
					RE::FormID					worldSpaceID = 0x0;
					RE::FormID					refFormID = 0x0;
					bool						isStatic = true;
					bool						badConvexHull = false;
					bool						isCreature = false;
					bool						hasCharControllerCollision = false;
					std::vector<CollisionLine>	collisionLines{};
					std::vector<CollisionMesh>	collisionMeshes{};

					RefCollisionData(RE::TESObjectREFR* a_ref);
					void	DrawObject();
					void	GetCollisionCoordinates();
					void	UpdateCollision();
					void	MarkForDeletion() { ref = nullptr; }
					float	GetSquareDistance(const RE::NiPoint3& a_point);

				private:
					RE::NiPoint3 previousPosition{ 0.0f, 0.0f, 0.0f };

					void AddCollisionLine(vec3u& a_start, vec3u& a_end);
					void AddCollisionLine(vec3u& a_start, vec3u& a_end, glm::vec4& a_color);
					void AddCollisionMesh(std::vector<CollisionTriangle>& a_triangles);
					void HandleActors(CollisionObject& a_object);
					void GetObjectCollisionCoordinates(CollisionObject& a_object);
					void GetBoxCollisionCoordinates(CollisionObject& a_object);
					void GetCapsuleCollisionCoordnates(CollisionObject& a_object);
					void GetCompresshedMeshCollisionCoordinates(CollisionObject& a_object);
					void GetConvexTransformCollisionCoordinates(CollisionObject& a_object);
					void GetConvexVerticesCollisionCoordinates(CollisionObject& a_object);
					void GetListCollisionCoordinates(CollisionObject& a_object);
					void GetMOPPCollisionCoordinates(CollisionObject& a_object);
					void LoopOverSingleShapeContainer(CollisionObject& a_object, const RE::hkpSingleShapeContainer& a_singleShapeContainer);
					
					std::vector<vec3u>								GetCircle(uint32_t a_segments, float a_radius, vec3u& a_unitXVector, vec3u& a_unitYVector, vec3u& a_center);
					std::pair<CollisionTriangle, CollisionTriangle> SquareToTriangles(vec3u& a_point1, vec3u& a_point2, vec3u& a_point3, vec3u& a_point4, vec4u& a_color = MCM::settings::collisionColor);
					
					static std::vector<RE::FormID> refsWithBadConvexHulls;
			};

			std::vector<std::unique_ptr<RefCollisionData>> visibleCollisions;
			std::vector<RE::TESObjectREFRPtr> selectedRefs;
			RE::TESObjectREFRPtr previousConsoleSelectedRef = nullptr;


			void  DrawCollisions();
			void  DrawCollision(RE::TESObjectREFR* a_ref);
			void  UpdateSelectedRefs(RE::TESObjectREFR* a_ref);
			float GetRange() override;

	};
}