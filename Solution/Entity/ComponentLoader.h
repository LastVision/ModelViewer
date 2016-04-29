#pragma once

struct AnimationComponentData;
struct GraphicsComponentData;
struct PhysicsComponentData;
struct TriggerComponentData;
struct SoundComponentData;
class XMLReader;

namespace tinyxml2
{
	class XMLElement;
}

class ComponentLoader
{
public:
	void LoadAnimationComponent(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement, AnimationComponentData& aOutputData);
	void LoadGraphicsComponent(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement, GraphicsComponentData& aOutputData);
	void LoadPhysicsComponent(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement, PhysicsComponentData& aOutputData);
	void LoadTriggerComponent(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement, TriggerComponentData& aOutputData);
	void LoadSoundComponent(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement, SoundComponentData& aOutputData);
private:

	eObjectRoomType LoadRoomType(XMLReader& aDocument, tinyxml2::XMLElement* aSourceElement);

	int ConvertToTriggerEnum(std::string aName);
};