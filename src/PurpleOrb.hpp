#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;

// Orbs behave differently from portals: a portal changes state the instant
// you touch it, but an orb only fires when you touch it AND press/hold jump
// -- that's what lets you choose whether to take an orb or fly past it.
// Vanilla's strongest orb is the red orb (the big upward kick). This one is
// deliberately stronger than that.
//
// "AND MORE" in the request was open-ended, so rather than invent extra
// behaviour that wasn't asked for, this v1.1 keeps it to exactly what was
// specific (stronger-than-red jump) plus the one natural extra that fits
// the rest of this mod's pattern: hue is controllable the same way the
// speed portal's is, via the object's own colour channel, so the orb can be
// recoloured without needing a new asset.
class PurpleOrb : public object_collab::CustomObject<EffectGameObject> {
public:
    using CustomObject::CustomObject;

protected:
    bool init(const char* frame) override;

public:
    static PurpleOrb* create(object_collab::ObjectInfo* info, const char* frame) {
        auto* ret = new PurpleOrb(info, object_collab::ObjectTraits::builder()
            .gameObjectType(GameObjectType::Modifier) // NOT Solid -- Solid is a physical block
            // Confirmed from CustomObject.hpp's own customSetup() source: m_isInvisible
            // = !editorEnabled && isTriggerObject() && !isSpeedObject() -- isSpeedObject(true)
            // is what keeps a Modifier-type object VISIBLE during actual gameplay,
            // not GameObjectType. CustomSpeedPortal already had this (why it was
            // the one object you didn't report as invisible); the others didn't.
            .isSpeedObject(true)
            .build());
        if (ret->init(frame)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void registerObject(geode::Mod* mod);

    // Kept on the hand-rolled PlayerObject::update rect-check in main.cpp
    // (not switched to the confirmed collidedByPlayer() hook like
    // ReversePortal/CustomSpeedPortal) because orbs specifically need
    // "reset the moment you leave" -- collidedByPlayer only fires while
    // touching, with no confirmed "just left" callback, so a manual
    // per-frame check is the safer way to get that leave-to-reset behavior
    // right rather than risk losing it on an unconfirmed assumption.
    // Called every frame the player overlaps this orb. Only actually fires
    // the boost if the jump button is currently held, same as vanilla orbs
    // -- otherwise the player just passes through it doing nothing, letting
    // them choose to skip it.
    // TODO verify: real member/accessor for "is the jump button currently
    // held" on PlayerObject -- ReversePortal.cpp and this file are the two
    // spots in this project still carrying an unverified GD symbol name.
    void tryActivate(PlayerObject* player);

    // Orbs reset the moment you're no longer overlapping them, not just on
    // respawn -- that's what lets you fly away and come back to hit the
    // same orb again later in the same attempt, matching vanilla orb feel.
    void release() { m_alreadyFired = false; }
    void resetForRespawn() { m_alreadyFired = false; }

    // How much stronger than the vanilla red orb this is. Red orb's jump
    // velocity magnitude isn't independently confirmed the way the 5 speed
    // portal timeMod floats are (ReSpeed's about.md only documents speed,
    // not orb strength) -- so this multiplier is a starting point to tune
    // by feel in-game, not a verified constant.
    static constexpr float kStrengthMultiplier = 1.35f;

private:
    bool m_alreadyFired = false;
};
