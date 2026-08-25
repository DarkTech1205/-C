#include "ReversePortal.hpp"
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool ReversePortal::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_alreadyActivated = false;
    return true;
}

void ReversePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("reverse-portal"_spr)
        .sprite("reverse-portal-icon.png"_spr)
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return ReversePortal::create(info, "reverse-portal-icon.png"_spr);
            })
            .build())
        .build();

    // ObjectInfo's copy constructor is explicitly deleted (confirmed) --
    // registerObject takes it by value, so this needs std::move, not a
    // plain pass, or it won't compile.
    ObjectAPI::registerObject(std::move(info), mod);
}

void ReversePortal::collidedByPlayer(PlayerObject* player) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    // doReversePlayer(bool reverse) is a real, confirmed PlayerObject
    // method (Geode's own PlayerObject docs) -- much more solid than the
    // earlier approach of spinning up a fake Reverse trigger with a
    // guessed object ID, and now removed entirely.
    //
    // Known limitation: this toggles based on this *portal's own* last
    // state, not the player's true current reverse status (no confirmed
    // public "am I currently reversed" getter was found) -- so if a
    // vanilla Reverse trigger elsewhere in the level also changes reverse
    // state, this portal's toggle could drift out of sync with it. Fine
    // for levels that only use these portals for reverse, worth revisiting
    // if mixing with vanilla reverse triggers turns out to matter.
    m_currentlyReversed = !m_currentlyReversed;
    player->doReversePlayer(m_currentlyReversed);
}
