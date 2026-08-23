#include "PurpleOrb.hpp"
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool PurpleOrb::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_alreadyFired = false;
    return true;
}

void PurpleOrb::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("purple-orb"_spr)
        .sprite("purple-orb-icon.png"_spr) // the actual round purple orb sprite
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return PurpleOrb::create("purple-orb-icon.png"_spr);
            })
            .build())
        .build();

    ObjectAPI::registerObject(info, mod);
}

void PurpleOrb::tryActivate(PlayerObject* player) {
    if (m_alreadyFired) return;

    // TODO verify real accessor -- placeholder name based on how other GD
    // orb-touch mods commonly refer to it. If this doesn't compile, search
    // the bindings for PlayerObject's jump-button-held state (something
    // Geode's bindings expose as a bool member or a small accessor).
    bool jumpHeld = player->m_isJumpButtonPressed;
    if (!jumpHeld) return;

    m_alreadyFired = true;

    // Vanilla red orb's jump call -- TODO verify real name/signature. Most
    // GD orb-hook mods call something along these lines; if the symbol
    // differs, grep the bindings for "redJump" / "Orb" in PlayerObject.
    constexpr float kRedOrbBaseVelocity = 19.0f; // approximate, unverified
    player->boostPlayer(kRedOrbBaseVelocity * kStrengthMultiplier);
}
