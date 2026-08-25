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
// - ".5 extra speed spawns a mini portal in front": built per-object by
//   CustomSpeedPortal::refreshSpeedStack(), called from the real confirmed
//   postInit() hook and live-refreshed from the editor popup as you type.

class CustomSpeedPortal : public object_collab::CustomObject<EffectGameObject> {
public:
    using CustomObject::CustomObject; // see ReversePortal.hpp for why

protected:
    bool init(const char* frame) override;

public:
    static CustomSpeedPortal* create(object_collab::ObjectInfo* info, const char* frame) {
        auto* ret = new CustomSpeedPortal(info, object_collab::ObjectTraits::builder()
            // The doc comment on isSpeedObject explicitly says it only works
            // when the object type is Modifier -- leaving gameObjectType on
            // its Solid default (as earlier drafts did) is almost certainly
            // why isSpeedObject was silently doing nothing.
            .gameObjectType(GameObjectType::Modifier)
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

    void collidedByPlayer(PlayerObject* player) override;

    // Real confirmed CustomObject<T> hook: "Runs after the object has fully
    // generated... highly recommended to use this if you want to alter
    // default GameObject properties." Used here for the initial speed-stack
    // build instead of the old dead spawnMiniMarkers approach, which needed
    // an external call site that never actually existed anywhere.
    void postInit() override;

    float getSpeedValue() const { return m_speedValue; }
    void setSpeedValue(float v) { m_speedValue = v; }

    // Rebuilds the row of chevron sprites in front of this portal so it
    // reads as "more speed = more portal icons stacked ahead of it" --
    // clears any previously-spawned stack first. Public so the editor
    // popup's onValue callback (in CustomSpeedPortal.cpp) can call it live
    // while you're still typing a new speed value in, not just once at
    // placement time.
    void refreshSpeedStack();

    float m_speedValue = 1.0f; // e.g. 1.5 = "1.5x", editable via the popup
    bool m_alreadyActivated = false;

private:
    cocos2d::CCNode* m_stackContainer = nullptr;
};

// Per-level bookkeeping. Simplified to a singleton rather than a real
// per-level Geode field: attaching custom data to GJBaseGameLayer through
// Geode's field system means writing our own $modify(GJBaseGameLayer) class
// with a Fields struct (the documented, real pattern), which only the class
// that declares Fields can read via m_fields -- not something an unrelated
// class like this one can fetch generically. A singleton avoids guessing at
// an API surface we're not sure exists, at the honest cost of being reset
// explicitly on level (re)start rather than automatically scoped per-level
// (see resetForNewAttempt(), called from the PlayLayer::resetLevel hook in
// main.cpp) -- fine for one level played at a time, but doesn't multiplex if
// two levels' custom speed lists needed to coexist simultaneously.
class SpeedPortalManager {
public:
    static SpeedPortalManager& instance() {
        static SpeedPortalManager s_instance;
        return s_instance;
    }

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

    // Registers `speed` in this level's ordered list if it's new, returns
    // its index (position it was appended at, i.e. "which portal in the
    // list this is"). This is separate bookkeeping from the visual stack
    // (CustomSpeedPortal::refreshSpeedStack) -- this tracks distinct speed
    // values across the whole level for the timeMod curve; the visual
    // stack is purely per-object, based on that object's own speed.
    int registerSpeed(float speed);

    // Clears the discovered-speeds list. Call this when a level restarts (or
    // a different level loads) so one level's custom speeds don't leak into
    // another's -- see the PlayLayer::resetLevel hook in main.cpp.
    void resetForNewAttempt() { m_orderedSpeeds.clear(); }

private:
    struct CurvePoint { float speed, timeMod, tangent; };
    static const std::vector<CurvePoint>& curve();

    std::vector<float> m_orderedSpeeds; // grows as new values are discovered
};
