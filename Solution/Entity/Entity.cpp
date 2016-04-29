#include "stdafx.h"

#include "AnimationComponent.h"
#include "GraphicsComponent.h"
#include "PhysicsComponent.h"
#include "PollingStation.h"
#include <Scene.h>
#include <Instance.h>
#include <EmitterMessage.h>
#include <PostMaster.h>
#include "TriggerComponent.h"
#include "SoundComponent.h"

Entity::Entity(unsigned int aGID, const EntityData& aEntityData, Prism::Scene* aScene, bool aClientSide, const CU::Vector3<float>& aStartPosition,
	const CU::Vector3f& aRotation, const CU::Vector3f& aScale, const std::string& aSubType)
	: myGID(aGID)
	, myScene(aScene)
	, myEntityData(aEntityData)
	, myEmitterConnection(nullptr)
	, myIsClientSide(aClientSide)
	, mySubType(aSubType)
	, myIsEnemy(false)
	, myIsActive(true)
	, myTimeActiveBeforeKill(10.f)
	, myTimeActiveBeforeKillTimer(10.f)
	, myDelayAddToSceneTimer(0.f)
	, myDelayedAddToScene(false)
{
	for (int i = 0; i < static_cast<int>(eComponentType::_COUNT); ++i)
	{
		myComponents[i] = nullptr;
	}

	myOrientation.SetPos(aStartPosition);

	SetRotation(aRotation);


	//has to be here fro press E-text
	if (aEntityData.myTriggerData.myExistsInEntity == true)
	{
		myComponents[static_cast<int>(eComponentType::TRIGGER)] = new TriggerComponent(*this, aEntityData.myTriggerData);
	}


	myRoomType = eObjectRoomType::NOT_USED_ON_SERVER;
	if (myScene != nullptr)
	{
		myRoomType = eObjectRoomType::NONE;
		if (aEntityData.myAnimationData.myExistsInEntity == true)
		{
			myComponents[static_cast<int>(eComponentType::ANIMATION)] = new AnimationComponent(*this, aEntityData.myAnimationData, aScene);
			//GetComponent<AnimationComponent>()->SetRotation(aRotation);
			GetComponent<AnimationComponent>()->SetScale(aScale);
			myRoomType = aEntityData.myAnimationData.myRoomType;
		}
		else if (aEntityData.myGraphicsData.myExistsInEntity == true)
		{
			myComponents[static_cast<int>(eComponentType::GRAPHICS)] = new GraphicsComponent(*this, aEntityData.myGraphicsData);
			//GetComponent<GraphicsComponent>()->SetRotation(aRotation);
			GetComponent<GraphicsComponent>()->SetScale(aScale);
			myRoomType = aEntityData.myGraphicsData.myRoomType;
		}
	}

	if (aEntityData.mySoundData.myExistsInEntity == true && myIsClientSide == true)
	{
		myComponents[static_cast<int>(eComponentType::SOUND)] = new SoundComponent(*this);
	}

	if (aEntityData.myPhysicsData.myExistsInEntity == true)
	{
		if (aEntityData.myAnimationData.myExistsInEntity == true)
		{
			myComponents[static_cast<int>(eComponentType::PHYSICS)] = new PhysicsComponent(*this, aEntityData.myPhysicsData
				, aEntityData.myAnimationData.myModelPath);
		}
		else if (aEntityData.myGraphicsData.myExistsInEntity == true)
		{
			myComponents[static_cast<int>(eComponentType::PHYSICS)] = new PhysicsComponent(*this, aEntityData.myPhysicsData
				, aEntityData.myGraphicsData.myModelPath);
		}
		else
		{
			if (myIsClientSide == true && aEntityData.myTriggerData.myExistsInEntity == true && aEntityData.myTriggerData.myIsClientSide == false)
			{
			}
			else
			{
				DL_ASSERT_EXP(myIsClientSide == false, "Can't create PhysicsComponent on client without graphics.");
			}

			myComponents[static_cast<int>(eComponentType::PHYSICS)] = new PhysicsComponent(*this, aEntityData.myPhysicsData
				, "no path");
		}
	}
	Reset();

};

Entity::~Entity()
{
	if (GetComponent<PhysicsComponent>() != nullptr)
	{
		SET_RUNTIME(false);
		GetComponent<PhysicsComponent>()->RemoveFromScene();
	}
	for (int i = 0; i < static_cast<int>(eComponentType::_COUNT); ++i)
	{
		delete myComponents[i];
		myComponents[i] = nullptr;
	}
}

void Entity::Reset()
{
	myAlive = true;
	myState = eEntityState::IDLE;

	for (int i = 0; i < static_cast<int>(eComponentType::_COUNT); ++i)
	{
		if (myComponents[i] != nullptr)
		{
			myComponents[i]->Reset();
		}
	}
	
	myDelayAddToSceneTimer = 0.f;
	myDelayedAddToScene = false;
}

void Entity::Update(float aDeltaTime)
{
	for each (Component* component in myComponents)
	{
		if (component != nullptr)
		{
			component->Update(aDeltaTime);
		}
	}

	if (myEntityData.myPhysicsData.myPhysicsType == ePhysics::DYNAMIC)
	{
		if (myComponents[static_cast<int>(eComponentType::NETWORK)] == nullptr)
		{
			memcpy(&myOrientation.myMatrix[0], GetComponent<PhysicsComponent>()->GetOrientation(), sizeof(float) * 16);
		}
	}

	if (myIsClientSide == false && myIsActive == false)
	{
		myTimeActiveBeforeKillTimer -= aDeltaTime;

		if (myTimeActiveBeforeKillTimer <= 0.f)
		{
			myTimeActiveBeforeKillTimer = myTimeActiveBeforeKill;
			myIsActive = true;
			Kill(false);
		}
	}

	if (myDelayedAddToScene == true)
	{
		myDelayAddToSceneTimer -= aDeltaTime;
		if (myDelayAddToSceneTimer <= 0.f)
		{
			myDelayedAddToScene = false;
			AddToScene();
		}
	}
}

void Entity::AddComponent(Component* aComponent)
{
	DL_ASSERT_EXP(myComponents[int(aComponent->GetType())] == nullptr, "Tried to add component several times");
	myComponents[int(aComponent->GetType())] = aComponent;
}

void Entity::RemoveComponent(eComponentType aComponent)
{
	DL_ASSERT_EXP(myComponents[int(aComponent)] != nullptr, "Tried to remove an nonexisting component");
	delete myComponents[int(aComponent)];
	myComponents[int(aComponent)] = nullptr;
}

void Entity::AddToScene()
{
	DL_ASSERT_EXP(myIsInScene == false, "Tried to add Entity to scene twice");
	DL_ASSERT_EXP(myIsClientSide == true, "You can't add Entity to scene on server side.");

	if (myIsInScene == true || myIsClientSide == false)
	{
		return;
	}

	if (GetComponent<GraphicsComponent>() != nullptr && GetComponent<GraphicsComponent>()->GetInstance() != nullptr)
	{
		myScene->AddInstance(GetComponent<GraphicsComponent>()->GetInstance(), myRoomType);
	}
	else if (GetComponent<AnimationComponent>() != nullptr && GetComponent<AnimationComponent>()->GetInstance() != nullptr)
	{
		myScene->AddInstance(GetComponent<AnimationComponent>()->GetInstance(), myRoomType);
		GetComponent<AnimationComponent>()->AddWeaponToScene(myScene);
	}

	myIsInScene = true;
}

void Entity::RemoveFromScene()
{
	DL_ASSERT_EXP(myIsInScene == true, "Tried to remove Entity not in scene");
	DL_ASSERT_EXP(myIsClientSide == true, "You can't remove Entity to scene on server side.");

	if (myIsInScene == false || myIsClientSide == false)
	{
		return;
	}

	if (GetComponent<GraphicsComponent>() != nullptr)
	{
		myScene->RemoveInstance(GetComponent<GraphicsComponent>()->GetInstance());
	}
	else if (GetComponent<AnimationComponent>() != nullptr)
	{
		myScene->RemoveInstance(GetComponent<AnimationComponent>()->GetInstance());
		GetComponent<AnimationComponent>()->RemoveWeaponFromScene(myScene);
	}

	myIsInScene = false;
}

eEntityType Entity::GetType() const
{
	return myEntityData.myType;
}

void Entity::AddEmitter(Prism::ParticleEmitterInstance* anEmitterConnection)
{
	myEmitterConnection = anEmitterConnection;
}

Prism::ParticleEmitterInstance* Entity::GetEmitter()
{
	if (myEmitterConnection == nullptr)
		return nullptr;
	return myEmitterConnection;
}

void Entity::Kill(bool aRemoveFromPhysics)
{
	myAlive = false;

	if (myIsInScene == true)
	{
		RemoveFromScene();
		myIsInScene = false;
	}
	
	if (aRemoveFromPhysics == true && myEntityData.myPhysicsData.myExistsInEntity == true)
	{
		GetComponent<PhysicsComponent>()->RemoveFromScene();
	}
}

bool Entity::GetIsClient()
{
	return myIsClientSide;
}