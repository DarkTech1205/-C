#include "ReversePortal.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool ReversePortal::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_alreadyActivated = false;
    return true;
}

void ReversePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("reverse-portal"_spr)
        .sprite("reverse-portal-icon.png"_spr)
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return ReversePortal::create(info, "reverse-portal-icon.png"_spr);
            })
            .build())
        .build();

    // ObjectInfo's copy constructor is explicitly deleted (confirmed) --
    // registerObject takes it by value, so this needs std::move, not a
    // plain pass, or it won't compile.
    ObjectAPI::registerObject(std::move(info), mod);
}

void ReversePortal::collidedByPlayer(PlayerObject* player) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    auto* layer = GJBaseGameLayer::get();
    if (!layer) return;

    // Spin up a real, vanilla Reverse trigger and fire its own activation
    // function rather than hand-rolling the direction-flip ourselves.
    // TODO: confirm the real Reverse trigger's object ID for GD 2.2081.
    constexpr int kReverseTriggerObjectID = 1917;

    auto* trigger = EffectGameObject::createWithKey(kReverseTriggerObjectID);
    trigger->triggerObject(layer, 0, nullptr);
    trigger->release();
}
