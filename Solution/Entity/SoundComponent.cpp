#include "stdafx.h"
#include <AudioInterface.h>
#include "SoundComponent.h"
#include <PostMaster.h>



SoundComponent::SoundComponent(Entity& aEntity)
	: Component(aEntity)
	, myAudioSFXID(-1)
{
	myAudioSFXID = Prism::Audio::AudioInterface::GetInstance()->GetUniqueID();

}

SoundComponent::~SoundComponent()
{
	if (!Prism::Audio::AudioInterface::GetInstance())
	{
		return;
	}
	Prism::Audio::AudioInterface::GetInstance()->UnRegisterObject(myAudioSFXID);

}

void SoundComponent::Update(float)
{
	Prism::Audio::AudioInterface::GetInstance()->SetPosition(myEntity.GetOrientation().GetPos().x
		, myEntity.GetOrientation().GetPos().y, myEntity.GetOrientation().GetPos().z, myAudioSFXID);
}

