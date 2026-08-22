#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>

// smjs.object-collab is what BigPortal uses to add genuinely new, fully custom
// placeable objects (its own editor button/tab/popup, its own sprite, its own
// touch behaviour) without stealing an existing GD object ID. Declaring it as
// a "required" dependency in mod.json makes the Geode CLI fetch its real
// public headers at build time -- confirm the exact class/method names below
// against those headers (they were reconstructed from BigPortal's compiled
// symbols, so names are right but a couple of parameter orders may drift
// between object-collab versions).
#include <smjs.object-collab/include/ObjectCollab.hpp>

using namespace geode::prelude;

// A portal-style version of the Reverse trigger: instead of firing once when
// a spawn/touch group activates it, it flips the level's reverse state the
// moment the player's hitbox touches it -- exactly like gravity/mirror/size
// portals already behave, just for direction.
class ReversePortal : public object_collab::CustomObject<EffectGameObject> {
protected:
    bool init(object_collab::ObjectInfo* info) override;

public:
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
