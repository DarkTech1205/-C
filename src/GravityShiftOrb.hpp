#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;

// Your idea: an orb that drifts up and down on its own. Jump while near it
// and two things happen at once: your gravity flips, AND you're teleported
// to the orb's current position. So the skill is timing your jump for
// *where the orb is in its cycle*, not just whether you're touching it --
// jump when it's near the top and you get flung upward post-flip, jump
// near the bottom and you don't.
//
// This is a genuinely new mechanic (not a reskin of an existing gamemode),
// so rather than build a full custom-popup editor UI for tuning its motion
// (like the speed portal's ValueMenu) before you've felt it in-game, v1.2
// ships it with fixed, tunable-in-code defaults. Once you've played with it
// and know what range/speed feels right, wiring up a popup for those two
// numbers is a small follow-up, not a redesign.
class GravityShiftOrb : public object_collab::CustomObject<EffectGameObject> {
public:
    using CustomObject::CustomObject;

protected:
    bool init(const char* frame) override;

public:
    static GravityShiftOrb* create(object_collab::ObjectInfo* info, const char* frame) {
        auto* ret = new GravityShiftOrb(info, object_collab::ObjectTraits::builder()
            .gameObjectType(GameObjectType::Modifier) // NOT Solid -- Solid is a physical block, which was causing every symptom reported
            .build());
        if (ret->init(frame)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void registerObject(geode::Mod* mod);

    // Starts the orb's own up/down drift. Called once when the object is
    // set up, independent of any player -- it moves whether or not anyone's
    // near it, same as vanilla moving objects do.
    void startOscillating();

    // Called every frame the player overlaps this orb. Only fires on the
    // *moment* jump is freshly pressed (edge-triggered), not every frame
    // jump happens to be held, or it'd fire repeatedly while you sit inside
    // it holding jump.
    void tryActivate(PlayerObject* player);

    void resetForRespawn() { m_wasJumpHeldLastFrame = false; }

    // Tuning defaults -- see the class comment above.
    static constexpr float kOscillationAmplitude = 60.f; // units up/down from spawn position
    static constexpr float kOscillationPeriod = 1.5f;    // seconds for one full up-down-up cycle

private:
    bool m_wasJumpHeldLastFrame = false;
};
