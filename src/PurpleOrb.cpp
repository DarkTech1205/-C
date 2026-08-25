#include "PurpleOrb.hpp"
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool PurpleOrb::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_alreadyFired = false;
    // Confirmed real CustomObject<T> method -- gives it a circular hitbox
    // like vanilla orbs instead of whatever default shape the sprite's
    // bounding box implied. 0.5 units (half a grid tile) roughly matches
    // vanilla orb size; adjust to taste once you see it in-game.
    this->setRoundHitbox(0.5f);
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
                return PurpleOrb::create(info, "purple-orb-icon.png"_spr);
            })
            .build())
        .build();

    ObjectAPI::registerObject(std::move(info), mod);
}

void PurpleOrb::tryActivate(PlayerObject* player) {
    if (m_alreadyFired) return;

    // Confirmed real method (Geode's own PlayerObject docs): bool
    // buttonDown(PlayerButton button) -- replaces the earlier guessed bool
    // member, which didn't exist.
    bool jumpHeld = player->buttonDown(PlayerButton::Jump);
    if (!jumpHeld) return;

    m_alreadyFired = true;

    // boostPlayer(float yVelocity) is a real, confirmed PlayerObject method
    // -- this call was already correct.
    constexpr float kRedOrbBaseVelocity = 19.0f; // approximate, still unverified
    player->boostPlayer(kRedOrbBaseVelocity * kStrengthMultiplier);
}
