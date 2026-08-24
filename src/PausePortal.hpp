#pragma once
#include <Geode/Geode.hpp>
#include <Geode/binding/EffectGameObject.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;

// Redesigned per your call: this isn't a special "freeze X" hack anymore --
// it's literally a speed portal fixed at 0x, using the real GD speed system
// (PlayerObject::updateTimeMod) the same way CustomSpeedPortal does, instead
// of manually pinning position every frame. That means it behaves exactly
// like you wanted: touch it once and X speed drops to 0 and STAYS there --
// through jumps, through leaving and re-entering the object, all the way
// until the player touches a different speed-setting portal (which calls
// updateTimeMod again with a new value) -- great for fall sections and
// transitions in corridors, and it sidesteps the "trapped until you hit a
// different portal" bug that the old position-pinning approach had, since
// that's now the actual intended behavior rather than an accident.
class PausePortal : public object_collab::CustomObject<EffectGameObject> {
public:
    using CustomObject::CustomObject;

protected:
    bool init(const char* frame) override;

public:
    static PausePortal* create(object_collab::ObjectInfo* info, const char* frame) {
        auto* ret = new PausePortal(info, object_collab::ObjectTraits::builder()
            .gameObjectType(GameObjectType::Modifier) // NOT Solid -- Solid is a physical block
            .isSpeedObject(true) // required both for real speed behavior AND to stay visible -- see ReversePortal.hpp's note
            .build());
        if (ret->init(frame)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void registerObject(geode::Mod* mod);

    // Single-latch touch, same pattern as ReversePortal/CustomSpeedPortal --
    // not a continuous per-frame hold anymore.
    void collidedByPlayer(PlayerObject* player) override;

    bool m_alreadyActivated = false;
};
