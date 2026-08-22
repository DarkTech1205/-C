#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/object_collab.hpp>
#include <vector>

using namespace geode::prelude;

// Design, matching what you described:
// - Hue: reuse GD's own per-object HSV colour channel editor -- no custom UI
//   needed, you already get a full hue/saturation/brightness picker for free
//   whenever an object has a main colour channel. We just read that colour
//   back out to tint the portal sprite.
// - Speed: a real editable number, entered through a small popup built with
//   object-collab's ValueMenu (the same building block ReSpeed and other
//   trigger-style mods use for their "enter a number" fields).
// - "every speed adds a new portal to the end": the vanilla game only has 5
//   fixed speed steps (0.5x/1x/2x/3x/4x -> the 0.7/0.9/1.1/1.3/1.6 timeMod
//   floats ReSpeed's about.md documents). We don't touch that fixed table --
//   instead each level keeps its OWN ordered list of every distinct custom
//   speed value placed in it, sorted ascending, built the first time the
//   level is played. New distinct values get appended to the end of that
//   list as they're discovered, and each entry's PlayerObject::updateTimeMod
//   float is derived from its position via speedToTimeMod() below.
// - ".5 extra speed spawns a mini portal in front": purely a level-load-time
//   cosmetic step -- see SpeedPortalManager::spawnMiniMarkers.

class CustomSpeedPortal : public object_collab::CustomObject<EffectGameObject> {
protected:
    bool init(object_collab::ObjectInfo* info) override;

public:
    static void registerObject(geode::Mod* mod);

    void onPlayerTouch(GJBaseGameLayer* layer, PlayerObject* player, bool isPlayer1);

    float getSpeedValue() const { return m_speedValue; }
    void setSpeedValue(float v) { m_speedValue = v; }

    float m_speedValue = 1.0f; // e.g. 1.5 = "1.5x", editable via the popup
    bool m_alreadyActivated = false;
};

// Per-level bookkeeping: one of these lives on the PlayLayer/LevelEditorLayer
// (attached via a Geode field on our GJBaseGameLayer modify class in main.cpp).
class SpeedPortalManager {
public:
    // Converts an arbitrary speed value (e.g. 1.5, 7.3, 0.2) into the float
    // GD's PlayerObject::updateTimeMod(float p0, bool p1) expects.
    //
    // Uses monotonic cubic Hermite interpolation (Fritsch-Carlson tangents)
    // through the 5 real vanilla reference points ReSpeed's about.md
    // documents:
    //   0.5x -> 0.7f   1.0x -> 0.9f   2.0x -> 1.1f   3.0x -> 1.3f   4.0x -> 1.6f
    // This passes exactly through every known vanilla speed (so a portal set
    // to exactly 1.0x, 2.0x etc. behaves pixel-identical to the real thing),
    // is smooth and *guaranteed monotonic* between them (no dips/overshoots
    // a plain cubic spline could introduce), and extrapolates outside
    // [0.5, 4.0] linearly off the boundary tangent instead of off a single
    // secant line, so speeds like 0.1x or 8x still feel continuous with the
    // curve rather than kinking at the edges.
    static float speedToTimeMod(float speed);

private:
    struct CurvePoint { float speed, timeMod, tangent; };
    static const std::vector<CurvePoint>& curve();

    // Registers `speed` in this level's ordered list if it's new, returns
    // its index (position it was appended at, i.e. "which portal in the
    // list this is").
    int registerSpeed(float speed);

    // Call once after a level's objects are all loaded. Walks every placed
    // CustomSpeedPortal; for any whose m_speedValue has a .5 fractional part
    // (1.5, 2.5, ...), duplicates a small marker sprite positioned a short
    // distance in front of it along the portal's facing direction.
    void spawnMiniMarkers(GJBaseGameLayer* layer);

    std::vector<float> m_orderedSpeeds; // grows as new values are discovered
};
