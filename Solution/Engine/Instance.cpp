#include "stdafx.h"

#include "Animation.h"
#include "Camera.h"
#include "Effect.h"
#include "EffectContainer.h"
#include "Instance.h"
#include "Model.h"
#include "ModelAnimated.h"
#include "ModelProxy.h"


Prism::Instance::Instance(ModelProxy& aModel, const CU::Matrix44<float>& anOrientation, eOctreeType anOctreeType
		, const float& aObjectCullingRadius)
	: myProxy(aModel)
	, myOctreeType(anOctreeType)
	, myOrientation(anOrientation)
	, myScale({1,1,1})
	, myObjectCullingRadius(aObjectCullingRadius)
	, myHierarchyIsBuilt(false)
	, myIsHovered(false)
	, myIsSelected(false)
	, mySelectionColor(0.f, 0.f, 1.f, 0.1f)
{
}

Prism::Instance::~Instance()
{
}

void Prism::Instance::Update(float aDelta)
{
	if (myProxy.IsLoaded() == true && myProxy.IsAnimated())
	{
		if (myHierarchyIsBuilt == false)
		{
			if (myProxy.myModelAnimated->myAnimation != nullptr)
			{
				myAnimation = myProxy.myModelAnimated->myAnimation;
			}
			BuildHierarchy(myHierarchy, myProxy.myModelAnimated);
			myHierarchyIsBuilt = true;
			
		}

		myHierarchy.Update(aDelta, myAnimation->GetAnimationLenght());
		if (myAnimation != nullptr)
		{
			myAnimation->Update(myTotalTime, myBones);
		}
		myTotalTime += aDelta;
	}
}

void Prism::Instance::Render(const Camera& aCamera)
{
	if (myProxy.IsLoaded())
	{

		myProxy.GetEffect()->SetViewProjectionMatrix(aCamera.GetViewProjection());
		myProxy.GetEffect()->SetScaleVector(myScale);
		myProxy.GetEffect()->SetCameraPosition(aCamera.GetOrientation().GetPos());

		if (myProxy.IsAnimated() == true)
		{
			myProxy.myModelAnimated->ActivateAlbedo(myOwnerType);
			myProxy.GetEffect()->SetBones(myBones);
			RenderModelAnimated(myProxy.myModelAnimated, myOrientation, aCamera, myHierarchy);
		}
		else
		{
			
			if (myIsSelected == true)
			{
				mySelectionColor.w = 1.f;
			}
			else if (myIsHovered == true)
			{
				mySelectionColor.w = 0.5f;
			}
			else
			{
				mySelectionColor.w = 0.1f;
			}
			myProxy.myModel->GetEffect()->SetAmbientHue(mySelectionColor);
			myProxy.myModel->ActivateAlbedo(myOwnerType);
			myProxy.Render(myOrientation, aCamera.GetOrientation().GetPos());
		}
	}
}

void Prism::Instance::RenderModelAnimated(ModelAnimated* aModel, const CU::Matrix44<float>& aParent
	, const Camera& aCamera, TransformationNodeInstance& aHierarchy)
{
	if (aModel->myIsNULLObject == false)
	{
		aModel->Render(aHierarchy.GetTransformation() * aParent, aCamera.GetOrientation().GetPos());
	}

	for (int i = 0; i < aHierarchy.GetChildren().Size(); ++i)
	{
		DL_ASSERT_EXP(aModel->myChildren[i] != nullptr, "Missmatch number of TransformationNodes and number of Models");

		RenderModelAnimated(aModel->myChildren[i], aHierarchy.GetTransformation() * aParent, aCamera, *aHierarchy.GetChildren()[i]);
	}
}

CU::Vector3<float> Prism::Instance::GetPosition() const
{
	return myOrientation.GetPos();
}

void Prism::Instance::SetEffect(const std::string& aEffectFile)
{
	if (myProxy.IsLoaded())
	{
		myProxy.SetEffect(EffectContainer::GetInstance()->GetEffect(aEffectFile));
	}
}

void Prism::Instance::SetScale(const CU::Vector3<float>& aScaleVector)
{
	myScale = aScaleVector;
	
}

void Prism::Instance::UpdateDirectionalLights(
	const CU::StaticArray<DirectionalLightData, NUMBER_OF_DIRECTIONAL_LIGHTS>& someDirectionalLightData)
{
	if (myProxy.IsLoaded())
	{
		myProxy.GetEffect()->UpdateDirectionalLights(someDirectionalLightData);
	}
}

void Prism::Instance::UpdatePointLights(
	const CU::StaticArray<PointLightData, NUMBER_OF_POINT_LIGHTS>& somePointLightData)
{
	if (myProxy.IsLoaded())
	{
		myProxy.GetEffect()->UpdatePointLights(somePointLightData);
	}
}

void Prism::Instance::UpdateSpotLights(
	const CU::StaticArray<SpotLightData, NUMBER_OF_SPOT_LIGHTS>& someSpotLightData)
{
	if (myProxy.IsLoaded())
	{
		myProxy.GetEffect()->UpdateSpotLights(someSpotLightData);
	}
}

void Prism::Instance::SetAnimation(Animation* aAnimation)
{
	myAnimation = aAnimation;
}

bool Prism::Instance::IsAnimationDone()
{
	if (myAnimation == nullptr) return true;

	if (myTotalTime > myAnimation->GetAnimationLenght()) return true;

	return false;
}

void Prism::Instance::ResetAnimationTime(float aTime)
{
	myTotalTime = aTime;
}

void Prism::Instance::ActivateAlbedo(eOwnerType aOwner)
{
	myOwnerType = aOwner;
}

Prism::ModelProxy& Prism::Instance::GetModel()
{
	return myProxy;
}

void Prism::Instance::BuildHierarchy(TransformationNodeInstance& aHierarchy, ModelAnimated* aModel)
{
	aHierarchy.SetTransformationNode(aModel->myTransformation);

	for (int i = 0; i < aModel->myChildTransforms.Size(); ++i)
	{
		TransformationNodeInstance* newNode = new TransformationNodeInstance();
		newNode->SetTransformationNode(aModel->myChildTransforms[i]);
		aHierarchy.AddChildNode(newNode);

		BuildHierarchy(*newNode, aModel->myChildren[i]);
	}
}
