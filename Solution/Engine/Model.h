#pragma once

#include "BaseModel.h"
#include "EffectListener.h"
#include <GrowingArray.h>
#include <Matrix.h>
#include "Vertices.h"
#include "LodStructs.h"


namespace Prism
{
	class Effect;
	class Surface;

	struct IndexBufferWrapper;
	struct MeshData;
	struct VertexBufferWrapper;
	struct VertexDataWrapper;
	struct VertexIndexWrapper;

	
	class Model : public BaseModel
	{
		friend class DGFXLoader;
		friend class FBXFactory;
	public:
		Model();
		~Model();

		void Init();
		void InitCube(float aWidth = 1.f, float aHeight = 1.f, float aDepth = 1.f
				, CU::Vector4f aColour = { 0.7f, 0.7f, 0.7f, 1.f });

		void AddChild(Model* aChild);
		void SetLodGroup(LodGroup* aLodGroup);

		
		void SetEffect(Effect* aEffect);

		void Render(const CU::Matrix44<float>& aOrientation, const CU::Vector3<float>& aCameraPosition);

		void ActivateAlbedo(eOwnerType aOwner);
		

	private:
		void EvaluateEffectTechnique();
		bool myIsNULLObject;

		CU::GrowingArray<D3D11_INPUT_ELEMENT_DESC*> myVertexFormat;
		VertexIndexWrapper* myIndexBaseData;
		VertexDataWrapper* myVertexBaseData;

		CU::GrowingArray<Model*> myChildren;
		CU::GrowingArray<CU::Matrix44f> myChildTransforms;
		CU::Matrix44f myOrientation;

		int myVertexCount;
		Model* myParent;
		bool myInited;
		bool myIsLodGroup;
		LodGroup* myLodGroup;
	};
}