#include "ReversePortal.hpp"
#include "CustomSpeedPortal.hpp"
#include "PausePortal.hpp"
#include "PurpleOrb.hpp"
#include "GravityShiftOrb.hpp"
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// object-collab's CustomObjectInterface has a real, confirmed
// collidedByPlayer(PlayerObject*) hook -- called every frame the player is
// in contact with a Modifier-type custom object. ReversePortal,
// CustomSpeedPortal, and PausePortal (now redesigned to use the real speed
// system instead of position-pinning) all use that directly. PurpleOrb/
// GravityShiftOrb stay on this hand-rolled rect-check because they
// specifically need "the moment contact ENDS" (to let an orb re-arm), and
// no confirmed "just left" callback exists on CustomObjectInterface --
// keeping them here is the safer choice until that's confirmed one way or
// another.
class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        auto* layer = GJBaseGameLayer::get();
        if (!layer) return;

        auto playerRect = this->getObjectRect();

        // NOTE: iterating every object in the level every frame (as written
        // below) will not perform well on large levels. Swap m_objects for
        // whichever "objects currently active in this section" list/array
        // GJBaseGameLayer already maintains for collision checks once
        // you've confirmed its real name -- worth optimizing before shipping.
        for (auto* obj : CCArrayExt<GameObject*>(layer->m_objects)) {
            if (auto* orb = typeinfo_cast<PurpleOrb*>(obj)) {
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
//
// This hooks PlayLayer::resetLevel rather than GJBaseGameLayer::resetLevel --
// the base-class version turned out to be force-inlined in the bindings
// (the actual compiler error: "cannot be hooked due to an inline definition
// existing"), so it's not a valid Geode hook target at all. PlayLayer's own
// override is one of the most commonly hooked functions across published
// Geode mods for exactly this kind of per-attempt reset logic.
class $modify(MyPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        SpeedPortalManager::instance().resetForNewAttempt();
        for (auto* obj : CCArrayExt<GameObject*>(this->m_objects)) {
            if (auto* rp = typeinfo_cast<ReversePortal*>(obj)) rp->m_alreadyActivated = false;
            if (auto* sp = typeinfo_cast<CustomSpeedPortal*>(obj)) sp->m_alreadyActivated = false;
            if (auto* pp = typeinfo_cast<PausePortal*>(obj)) pp->m_alreadyActivated = false;
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
