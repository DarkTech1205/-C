#include "PausePortal.hpp"
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool PausePortal::init(ObjectInfo* info) {
    if (!CustomObject::init(info)) return false;
    return true;
}

void PausePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("pause-portal"_spr)
        .sprite("pause-portal-icon.png"_spr) // the "||" bars
        .editorTab(EditorTab::Portals)
        .editorButtonColor(EditorButtonColor::Purple)
        .construction([](ObjectInfo* info) -> CustomObjectInterface* {
            return PausePortal::create(info);
        })
        .build();

    auto traits = ObjectTraits::builder()
        .gameObjectType(GameObjectType::Solid)
        .defaultZLayer(ZLayer::B4)
        .build();

    info.setTraits(traits);
    ObjectAPI::registerObject(info, mod);
}

void PausePortal::holdPlayer(PlayerObject* player) {
    // Simplest reliable "freeze X" approach: GD advances the player's X
    // position internally every frame based on level speed/timeMod rather
    // than a simple velocity variable we could just zero out, so instead of
    // fighting that internal math we pin the X coordinate back to where it
    // was the instant contact started -- applied AFTER PlayerObject::update
    // runs each frame (see the update() hook in main.cpp), so it's always
    // the final word on X position for that frame. Y (gravity, jumping,
    // rotation) is untouched and keeps behaving normally.
    if (m_holdX == std::nullopt) {
        m_holdX = player->getPositionX();
    }
    player->setPositionX(*m_holdX);
}
