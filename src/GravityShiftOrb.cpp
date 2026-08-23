#include "GravityShiftOrb.hpp"
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool GravityShiftOrb::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_wasJumpHeldLastFrame = false;
    return true;
}

void GravityShiftOrb::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("gravity-shift-orb"_spr)
        .sprite("gravity-shift-orb-icon.png"_spr) // TODO: use the teal ring render's actual game sprite once you export one
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::Aqua)
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                auto* orb = GravityShiftOrb::create(info, "gravity-shift-orb-icon.png"_spr);
                if (orb) orb->startOscillating();
                return orb;
            })
            .build())
        .build();

    ObjectAPI::registerObject(std::move(info), mod);
}

void GravityShiftOrb::startOscillating() {
    // Simple self-contained ping-pong motion -- doesn't need a move trigger
    // wired up in the editor, the orb just drifts on its own the instant
    // it's placed. Half the amplitude up, then a full amplitude down, then
    // half back to center, repeated forever, so it starts at its own spawn
    // point instead of jumping there on the first frame.
    float halfPeriod = kOscillationPeriod / 2.f;
    auto up = CCMoveBy::create(halfPeriod, {0.f, kOscillationAmplitude});
    auto down = CCMoveBy::create(kOscillationPeriod, {0.f, -kOscillationAmplitude * 2.f});
    auto backToCenter = CCMoveBy::create(halfPeriod, {0.f, kOscillationAmplitude});
    auto easedUp = CCEaseSineInOut::create(up);
    auto easedDown = CCEaseSineInOut::create(down);
    auto easedBack = CCEaseSineInOut::create(backToCenter);
    auto cycle = CCSequence::create(easedUp, easedDown, easedBack, nullptr);
    this->runAction(CCRepeatForever::create(cycle));
}

void GravityShiftOrb::tryActivate(PlayerObject* player) {
    // TODO verify: same unverified accessor as PurpleOrb.cpp for "is jump
    // currently held" -- fix both call sites together once confirmed.
    bool jumpHeldNow = player->m_isJumpButtonPressed;
    bool freshPress = jumpHeldNow && !m_wasJumpHeldLastFrame;
    m_wasJumpHeldLastFrame = jumpHeldNow;

    if (!freshPress) return;

    // Teleport first, then flip gravity, so the flip applies from the
    // orb's position rather than wherever the player was an instant before
    // (matters for how the post-flip velocity reads to the player).
    // TODO verify both real symbol names -- placeholders based on common
    // GD/Geode mod naming for portal-style teleport and gravity toggle;
    // confirm against the bindings once object-collab's include is sorted.
    player->teleportToVertical(this->getPosition(), false);
    player->toggleGravityEffect(!player->m_isUpsideDown, true);
}
