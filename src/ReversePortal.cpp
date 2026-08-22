#include "ReversePortal.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool ReversePortal::init(ObjectInfo* info) {
    if (!CustomObject::init(info)) return false;
    // Same flag BigPortal's "no-multi-activate" property backs -- stops the
    // portal re-triggering every single frame the player overlaps it.
    m_alreadyActivated = false;
    return true;
}

void ReversePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("reverse-portal"_spr)
        // Placeholder: downscaled from your 3D showcase render. That render
        // reads great as a big promo image but is too busy at editor-icon
        // size — worth swapping for a purpose-drawn small icon later.
        .sprite("reverse-portal-icon.png"_spr)
        .editorTab(EditorTab::Portals)
        .editorButtonColor(EditorButtonColor::Green)
        .construction([](ObjectInfo* info) -> CustomObjectInterface* {
            return ReversePortal::create(info);
        })
        .build();

    auto traits = ObjectTraits::builder()
        .gameObjectType(GameObjectType::Solid) // touch-based, not a trigger box
        .defaultZLayer(ZLayer::B4)
        .build();

    info.setTraits(traits);
    ObjectAPI::registerObject(info, mod);
}

void ReversePortal::onPlayerTouch(GJBaseGameLayer* layer, PlayerObject* player) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    // Spin up a real, vanilla Reverse trigger and fire its own activation
    // function rather than hand-rolling the direction-flip ourselves. This
    // guarantees identical behaviour to the base game's trigger (same speed
    // handling, same camera behaviour, same everything) and survives future
    // GD updates that change how reverse actually works internally.
    // TODO: confirm the real Reverse trigger's object ID for the installed
    // GD version (look it up in the object ID list / GDShare -- it moves
    // between updates, unlike the stable-since-2.2 `updateTimeMod` symbol
    // ReSpeed's about.md documents for speed portals).
    constexpr int kReverseTriggerObjectID = 1917;

    auto trigger = EffectGameObject::createWithKey(kReverseTriggerObjectID);
    trigger->m_notActive = false;
    trigger->triggerObject(layer, /*p1*/ std::nullopt, /*p2*/ nullptr);
    trigger->release();
}
