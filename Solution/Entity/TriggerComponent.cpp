#include "stdafx.h"
#include "CollisionNote.h"
#include <GameStateMessage.h>
#include "PhysicsComponent.h"
#include <PostMaster.h>
#include "TriggerComponent.h"
#include "TriggerComponentData.h"

TriggerComponent::TriggerComponent(Entity& anEntity, const TriggerComponentData& someData)
	: Component(anEntity)
	, myData(someData)
	, myTriggerType(eTriggerType(someData.myTriggerType))
	, myHasTriggered(false)
	, myRespawnValue(-1)
	, myPlayersInside(0)
	, myRespawnTime(0.f)
	, myPickupTextRows(0)
{
	if (myData.myPickupText.size() > 0)
	{
		for each (char letter in myData.myPickupText)
		{
			if (letter == '\n')
			{
				myPickupTextRows++;
			}
		}
	}
}

TriggerComponent::~TriggerComponent()
{
}

void TriggerComponent::Update(float aDelta)
{
	
}

void TriggerComponent::ReceiveNote(const CollisionNote& aNote)
{
	
}

void TriggerComponent::Activate()
{
}

int TriggerComponent::GetValue() const
{
	return myData.myValue;
}

bool TriggerComponent::IsClientSide() const
{
	return myData.myIsClientSide;
}

bool TriggerComponent::GetIsActiveFromStart() const
{
	return myData.myIsActiveFromStart;
}

bool TriggerComponent::IsPressable() const
{
	return myData.myIsPressable;
}