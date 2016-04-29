#include "stdafx.h"

#include <AudioInterface.h>
#include "Level.h"
#include "Console.h"
#include <CommonHelper.h>
#include <GameStateMessage.h>
#include <EffectContainer.h>
#include "InGameState.h"
#include <InputWrapper.h>
#include <ModelLoader.h>
#include <MurmurHash3.h>
#include <PostMaster.h>
#include <ScriptSystem.h>
#include <VTuneApi.h>
#include <TextProxy.h>
#include <SpriteProxy.h>
#include <Cursor.h>

InGameState::InGameState(int aLevelID)
	: myGUIManager(nullptr)
	, myLevelToLoad(aLevelID)
	, myState(eInGameState::LEVEL)
	, myLevel(nullptr)
	, myFailedLevelHash(false)
	, myHasStartedMusicBetweenLevels(false)
	, myLastLevel(aLevelID)
{
	myIsActiveState = false;

	myLevelCompleteSprite = Prism::ModelLoader::GetInstance()->LoadSprite(
		"Data/Resource/Texture/Menu/T_background_story01.dds", { 1920.f, 1080.f });
	myLevelFailedSprite = Prism::ModelLoader::GetInstance()->LoadSprite(
		"Data/Resource/Texture/Menu/T_background_gameover.dds", { 1920.f, 1080.f });
	myLoadingScreenSprite = Prism::ModelLoader::GetInstance()->LoadSprite(
		"Data/Resource/Texture/Menu/T_background_story01.dds", { 1920.f, 1080.f });
	myRotatingThing = Prism::ModelLoader::GetInstance()->LoadSprite(
		"Data/Resource/Texture/Menu/T_rotating_thing.dds", { 128.f, 128.f }, { 64.f, 64.f });
	myPressToStart = Prism::ModelLoader::GetInstance()->LoadSprite(
		"Data/Resource/Texture/Menu/T_press_to_start.dds", { 512.f, 64.f });
	CU::Vector2<int> windowSize = Prism::Engine::GetInstance()->GetWindowSizeInt();
	OnResize(windowSize.x, windowSize.y);
}

InGameState::~InGameState()
{
	PostMaster::GetInstance()->UnSubscribe(eMessageType::GAME_STATE, this);
	Console::Destroy();
	SAFE_DELETE(myLevelCompleteSprite);
	SAFE_DELETE(myLevelFailedSprite);
	SAFE_DELETE(myLoadingScreenSprite);
	SAFE_DELETE(myRotatingThing);
	SAFE_DELETE(myPressToStart);
	SAFE_DELETE(myLevel);
	SAFE_DELETE(myLevelFactory);
	SAFE_DELETE(myText);
}

void InGameState::InitState(StateStackProxy* aStateStackProxy, GUI::Cursor* aCursor)
{
	myIsLetThrough = false;
	myStateStack = aStateStackProxy;
	myStateStatus = eStateStatus::eKeepState;
	myCursor = aCursor;
	myCursor->SetShouldRender(false);

	myIsActiveState = true;

	myText = Prism::ModelLoader::GetInstance()->LoadText(Prism::Engine::GetInstance()->GetFont(Prism::eFont::CONSOLE));
	myText->SetPosition(CU::Vector2<float>(800.f, 800.f));

	PostMaster::GetInstance()->Subscribe(eMessageType::GAME_STATE, this);
}

void InGameState::EndState()
{
}

const eStateStatus InGameState::Update(const float& aDeltaTime)
{
	
	return myStateStatus;
}

void InGameState::Render()
{

}

void InGameState::ResumeState()
{
	myIsActiveState = true;
	myLevelToLoad = -1;
}

void InGameState::ReceiveMessage(const GameStateMessage& aMessage)
{
	if (myState != eInGameState::LEVEL)
	{
		switch (aMessage.myGameState)
		{
		case eGameState::LOAD_LEVEL:
			myLevelToLoad = aMessage.myID;
			myState = eInGameState::LOAD_LEVEL;
			break;
		}
	}
}

void InGameState::OnResize(int aWidth, int aHeight)
{

}