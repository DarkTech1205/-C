#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>

// smjs.object-collab is what BigPortal uses to add genuinely new, fully custom
// placeable objects (its own editor button/tab/popup, its own sprite, its own
// touch behaviour) without stealing an existing GD object ID.
// Confirmed real path (from the geode-deps diagnostic build log):
// build/geode-deps/smjs.object-collab/include/object_collab.hpp -- this is
// the umbrella header; ObjectInfo, ObjectTraits, CustomObject etc. each also
// have their own standalone headers alongside it if you ever want a lighter
// include.
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;

// A portal-style version of the Reverse trigger: instead of firing once when
// a spawn/touch group activates it, it flips the level's reverse state the
// moment the player's hitbox touches it -- exactly like gravity/mirror/size
// portals already behave, just for direction.
class ReversePortal : public object_collab::CustomObject<EffectGameObject> {
protected:
    bool init(const char* frame) override;

public:
    // Static factory methods aren't virtual in C++, so the inherited
    // EffectGameObject::create(frame) would allocate a plain EffectGameObject,
    // never a ReversePortal -- confirmed by a compiler error ("cannot
    // initialize CustomObjectInterface* with EffectGameObject*"), not a
    // guess. This is the standard cocos2d-x create() pattern (the same shape
    // as the CREATE_FUNC macro, just parameterized): declaring our own
    // create() here hides the inherited one and actually allocates the
    // right type.
    static ReversePortal* create(const char* frame) {
        auto* ret = new ReversePortal();
        if (ret->init(frame)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    // Registers the object with object-collab: editor tab, icon, colour, and
    // marks it as a trigger-style object so it gets touch collision instead
    // of a hitbox the player collides with physically.
    static void registerObject(geode::Mod* mod);

    // Called from our PlayerObject::update hook when the player's rect
    // overlaps this portal. Re-uses the real vanilla Reverse trigger's own
    // activation logic instead of us reimplementing the direction-flip
    // internals, so behaviour stays perfectly in sync with the base game.
    void onPlayerTouch(GJBaseGameLayer* layer, PlayerObject* player);

    bool m_alreadyActivated = false; // backs the "no-multi-activate" property
};
