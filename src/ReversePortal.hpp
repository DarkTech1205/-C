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
