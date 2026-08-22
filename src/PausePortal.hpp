#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/ObjectCollab.hpp>

using namespace geode::prelude;

// The "||" icon -- a zone-style portal, not a one-shot activation like the
// reverse or speed portals. While the player's hitbox overlaps it, their
// X-axis movement is frozen every frame (position held, X velocity zeroed);
// gravity/jumping/rotation on the Y axis keep working normally. The instant
// they leave its bounds, normal X movement resumes on its own -- no matching
// "unpause" object needed.
class PausePortal : public object_collab::CustomObject<EffectGameObject> {
protected:
    bool init(object_collab::ObjectInfo* info) override;

public:
    static void registerObject(geode::Mod* mod);

    // Called every frame the player's rect overlaps this object (NOT a
    // single-activate latch like ReversePortal/CustomSpeedPortal -- freezing
    // has to be continuously re-applied for as long as contact holds).
    void holdPlayer(PlayerObject* player);

    // Cleared whenever the player is no longer overlapping this portal (see
    // the update() hook in main.cpp) so the next contact re-anchors fresh
    // instead of snapping back to a stale position from last time.
    void releasePlayer() { m_holdX = std::nullopt; }
    bool isHolding() const { return m_holdX.has_value(); }

private:
    std::optional<float> m_holdX;
};
