#include "ReversePortal.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool ReversePortal::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    // Same flag BigPortal's "no-multi-activate" property backs -- stops the
    // portal re-triggering every single frame the player overlaps it.
    m_alreadyActivated = false;
    return true;
}

void ReversePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("reverse-portal"_spr)
        // Placeholder: downscaled from your 3D showcase render. That render
        // reads great as a big promo image but is too busy at editor-icon
        // size -- worth swapping for a purpose-drawn small icon later.
        .sprite("reverse-portal-icon.png"_spr)
        // Confirmed real EditorTab enum has no dedicated Portals/Orbs entry
        // (None/Solids/TransparentSolids/Slopes/Hazards/ThreeDimensionals/
        // PlayerModifiers/Animated/Pixels/Collectables/Particles/
        // Decorations/Saws/Triggers) -- PlayerModifiers is the closest
        // conceptual fit for an object that changes player state on touch.
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::Pink) // confirmed real values: Green/Aqua/Pink/LightGray/DarkGray/Red
        // .construction() needs an actual ObjectConstruction (variant of
        // QuickObject or ComplexObject), not a raw lambda -- confirmed by
        // compiler. ComplexObject::builder().factory(...) is the path for a
        // real custom C++ class like this one; QuickObject is for objects
        // that don't need their own class.
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return ReversePortal::create(info, "reverse-portal-icon.png"_spr);
            })
            .build())
        .build();

    // NOTE: ObjectInfo::Builder's confirmed full method list is id/sprite/
    // construction/editorTab/editorButtonColor/editObject/editSpecial/build
    // -- there's no method here to attach an ObjectTraits. Wherever that
    // actually plugs in (CustomObject.hpp, not yet dumped) is still unknown,
    // so the traits/z-layer/game-object-type setup from earlier drafts is
    // removed rather than another wrong guess. Our own hand-rolled touch
    // detection in main.cpp doesn't depend on it either way.

    ObjectAPI::registerObject(info, mod);
}

void ReversePortal::onPlayerTouch(GJBaseGameLayer* layer, PlayerObject* player) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    // Spin up a real, vanilla Reverse trigger and fire its own activation
    // function rather than hand-rolling the direction-flip ourselves.
    // TODO: confirm the real Reverse trigger's object ID for GD 2.2081.
    constexpr int kReverseTriggerObjectID = 1917;

    auto* trigger = EffectGameObject::createWithKey(kReverseTriggerObjectID);
    // m_notActive isn't a real GameObject member -- confirmed by compiler,
    // removed. triggerObject's real confirmed signature:
    //   virtual void triggerObject(GJBaseGameLayer* layer, int uniqueID, gd::vector<int> const* remapKeys);
    trigger->triggerObject(layer, 0, nullptr);
    trigger->release();
}
