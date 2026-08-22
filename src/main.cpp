#include "ReversePortal.hpp"
#include "CustomSpeedPortal.hpp"
#include "PausePortal.hpp"
#include "PurpleOrb.hpp"
#include "GravityShiftOrb.hpp"
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

// BigPortal hooks PlayerObject::update itself rather than relying on
// object-collab to call something automatically -- portal-type effects
// (gravity/mirror/mini/speed) are checked directly against the player's
// hitbox every frame in the base game, and custom objects need to do the
// same thing by hand. This mirrors that pattern for both of our objects.
class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        auto* layer = GJBaseGameLayer::get();
        if (!layer) return;

        auto playerRect = this->getObjectRect();
        bool isP1 = (this == layer->m_player1);

        // NOTE: iterating every object in the level every frame (as written
        // below) will not perform well on large levels. Swap m_objects for
        // whichever "objects currently active in this section" list/array
        // GJBaseGameLayer already maintains for collision checks (the same
        // one vanilla portal detection uses) once you've confirmed its real
        // name against the Geode bindings -- this is the one piece of this
        // file worth optimizing before shipping.
        for (auto* obj : CCArrayExt<GameObject*>(layer->m_objects)) {
            if (auto* reversePortal = typeinfo_cast<ReversePortal*>(obj)) {
                if (playerRect.intersectsRect(reversePortal->getObjectRect())) {
                    reversePortal->onPlayerTouch(layer, this);
                }
            } else if (auto* speedPortal = typeinfo_cast<CustomSpeedPortal*>(obj)) {
                if (playerRect.intersectsRect(speedPortal->getObjectRect())) {
                    speedPortal->onPlayerTouch(layer, this, isP1);
                }
            } else if (auto* pausePortal = typeinfo_cast<PausePortal*>(obj)) {
                // Continuous, not single-activate: hold while overlapping,
                // release the moment contact ends so it doesn't leave the
                // player stuck to a stale X after they've left the zone.
                bool overlapping = playerRect.intersectsRect(pausePortal->getObjectRect());
                if (overlapping) {
                    pausePortal->holdPlayer(this);
                } else if (pausePortal->isHolding()) {
                    pausePortal->releasePlayer();
                }
            } else if (auto* orb = typeinfo_cast<PurpleOrb*>(obj)) {
                if (playerRect.intersectsRect(orb->getObjectRect())) {
                    orb->tryActivate(this);
                } else {
                    orb->release();
                }
            } else if (auto* gravOrb = typeinfo_cast<GravityShiftOrb*>(obj)) {
                if (playerRect.intersectsRect(gravOrb->getObjectRect())) {
                    gravOrb->tryActivate(this);
                }
            }
        }
    }
};

// Resets every custom portal's "already activated" flag on respawn, the same
// way vanilla non-multi-activate triggers reset on checkpoint/death, so a
// portal fires again next attempt instead of staying spent forever.
class $modify(MyGJBaseGameLayer, GJBaseGameLayer) {
    void resetLevel() {
        GJBaseGameLayer::resetLevel();
        for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
            if (auto* rp = typeinfo_cast<ReversePortal*>(obj)) rp->m_alreadyActivated = false;
            if (auto* sp = typeinfo_cast<CustomSpeedPortal*>(obj)) sp->m_alreadyActivated = false;
            if (auto* orb = typeinfo_cast<PurpleOrb*>(obj)) orb->resetForRespawn();
            if (auto* gravOrb = typeinfo_cast<GravityShiftOrb*>(obj)) gravOrb->resetForRespawn();
        }
    }
};

$on_mod(Loaded) {
    auto mod = Mod::get();
    ReversePortal::registerObject(mod);
    CustomSpeedPortal::registerObject(mod);
    PausePortal::registerObject(mod);
    PurpleOrb::registerObject(mod);
    GravityShiftOrb::registerObject(mod);
}
