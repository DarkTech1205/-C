#include "PausePortal.hpp"
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>

using namespace geode::prelude;
using namespace object_collab;

bool PausePortal::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_alreadyActivated = false;
    return true;
}

void PausePortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("pause-portal"_spr)
        .sprite("pause-portal-icon.png"_spr) // the "||" bars
        .editorTab(EditorTab::PlayerModifiers)
        .editorButtonColor(EditorButtonColor::DarkGray)
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return PausePortal::create(info, "pause-portal-icon.png"_spr);
            })
            .build())
        .build();

    ObjectAPI::registerObject(std::move(info), mod);
}

void PausePortal::collidedByPlayer(PlayerObject* player) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    auto* layer = GJBaseGameLayer::get();
    bool isPlayer1 = layer && layer->m_player1 == player;

    // Literally 0x speed -- not derived from SpeedPortalManager's curve
    // (that curve extrapolates smoothly toward near-zero, it doesn't
    // guarantee an exact stop), a hard 0.0f so this is unambiguously "no
    // horizontal movement," matching what you actually want here.
    player->updateTimeMod(0.0f, isPlayer1);
}
