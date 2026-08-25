#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;

// A portal-style version of the Reverse trigger: instead of firing once when
// a spawn/touch group activates it, it flips the level's reverse state the
// moment the player's hitbox touches it -- exactly like gravity/mirror/size
// portals already behave, just for direction.
class ReversePortal : public object_collab::CustomObject<EffectGameObject> {
public:
    // CustomObject<T>'s own constructor takes (ObjectInfo*, ObjectTraits&&)
    // -- confirmed from the real CustomObject.hpp dump. There's no default
    // constructor to inherit implicitly, so we inherit this one explicitly
    // rather than writing a forwarding constructor by hand.
    using CustomObject::CustomObject;

protected:
    bool init(const char* frame) override;

public:
    // Two real, confirmed-from-source facts changed this from earlier
    // drafts:
    // 1. Static factories aren't virtual, so create() has to be declared
    //    here to actually allocate a ReversePortal (confirmed by an
    //    earlier compiler error).
    // 2. CustomObject<T>'s constructor needs traits at construction time,
    //    before init() runs -- confirmed by the real header. So create()
    //    now takes the ObjectInfo* the factory callback receives, and
    //    builds the ObjectTraits itself.
    static ReversePortal* create(object_collab::ObjectInfo* info, const char* frame) {
        auto* ret = new ReversePortal(info, object_collab::ObjectTraits::builder()
            // Third attempt at this value, and worth being honest about the
            // history: Solid (default) physically blocked the player like a
            // wall. Modifier fixed that, and per source m_isInvisible should
            // have gone false once isSpeedObject(true) was set below -- but
            // it's still reportedly invisible/inert in-game, which suggests
            // Modifier (GD's trigger category) likely never gets real
            // per-frame collidedByPlayer() checks at all, regardless of
            // visibility -- triggers are probably detected by
            // position/zone, not hitbox collision. Special is a genuine
            // non-solid, non-trigger, non-hardcoded-portal-swap category --
            // the safest remaining guess. If this still doesn't work, the
            // last real option is one of the confirmed vanilla portal/orb
            // types (TeleportPortal, RegularSizePortal, etc.), accepting
            // the risk that they may carry their own hardcoded vanilla
            // side effects alongside our custom logic.
            .gameObjectType(GameObjectType::Special)
            .isSpeedObject(true) // keeps it visible per the confirmed m_isInvisible formula -- unrelated to the type question above
            .build());
        if (ret->init(frame)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void registerObject(geode::Mod* mod);

    // CustomObjectInterface's real confirmed touch hook -- called every
    // frame the player is in contact with a Solid-type custom object. This
    // replaces the hand-rolled PlayerObject::update rect-intersection
    // check earlier drafts used; the framework already does that detection
    // for us.
    void collidedByPlayer(PlayerObject* player) override;

    bool m_alreadyActivated = false; // backs the "no-multi-activate" property

private:
    // Own local toggle state -- see the note in ReversePortal.cpp on why
    // this doesn't read the player's true current reverse state.
    bool m_currentlyReversed = false;
};
