#include "CustomSpeedPortal.hpp"
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <algorithm>

using namespace geode::prelude;
using namespace object_collab;

bool CustomSpeedPortal::init(const char* frame) {
    if (!CustomObject::init(frame)) return false;
    m_speedValue = 1.0f;
    m_alreadyActivated = false;
    return true;
}

void CustomSpeedPortal::registerObject(Mod* mod) {
    auto info = ObjectInfo::builder()
        .id("custom-speed-portal"_spr)
        .sprite("speed-portal-icon.png"_spr) // the ">" chevron
        .editorTab(EditorTab::PlayerModifiers) // confirmed enum, closest fit -- see ReversePortal.cpp
        .editorButtonColor(EditorButtonColor::Aqua) // confirmed real values: Green/Aqua/Pink/LightGray/DarkGray/Red
        // Now using the CONFIRMED real popup API from the full
        // EditorPopupConfig.hpp dump: NumericMenu is exactly the "type a
        // number" field we wanted, and applyValueToSelected/
        // getCommonValueOrDefault are real free functions built for exactly
        // this pointer-to-member pattern.
        .editSpecial([](Selected const& selected) -> PopupOptions {
            auto menu = editor_popup::NumericMenu::builder()
                .id("speed"_spr)
                .title("Speed")
                .onValue([](float value, const Selected& sel, geode::Popup*) {
                    applyValueToSelected(sel, &CustomSpeedPortal::m_speedValue, value);
                })
                .currentValue([](const Selected& sel, geode::Popup*) -> float {
                    return getCommonValueOrDefault(sel, &CustomSpeedPortal::m_speedValue);
                })
                .placeholder("Speed")
                .inputType(editor_popup::NumericMenu::InputType::TextBox)
                .precision(2)
                .stepSize(0.1f)
                .min(0.1f)
                .max(std::nullopt)
                .build();
            return editor_popup::PopupConfig::builder()
                .title("Custom Speed Portal")
                .menu(std::move(menu))
                .width(200.f)
                .height(150.f)
                .build();
        })
        .construction(ComplexObject::builder()
            .factory([](ObjectInfo* info) -> CustomObjectInterface* {
                return CustomSpeedPortal::create("speed-portal-icon.png"_spr);
            })
            .build())
        .build();

    // NOTE: ObjectTraits (isSpeedObject etc.) has no confirmed attachment
    // point on ObjectInfo::Builder -- see ReversePortal.cpp's note. Removed
    // rather than another wrong guess; our own touch detection in main.cpp
    // doesn't need it.

    ObjectAPI::registerObject(info, mod);
}

void CustomSpeedPortal::onPlayerTouch(GJBaseGameLayer* layer, PlayerObject* player, bool isPlayer1) {
    if (m_alreadyActivated) return;
    m_alreadyActivated = true;

    // Singleton bookkeeping -- see the class comment in CustomSpeedPortal.hpp
    // for why this isn't a real per-level Geode field.
    SpeedPortalManager::instance().registerSpeed(m_speedValue);

    float timeMod = SpeedPortalManager::speedToTimeMod(m_speedValue);
    player->updateTimeMod(timeMod, isPlayer1);
}

const std::vector<SpeedPortalManager::CurvePoint>& SpeedPortalManager::curve() {
    // Built once: raw (speed, timeMod) points from ReSpeed's about.md, then
    // Fritsch-Carlson tangents computed so the resulting cubic Hermite curve
    // is monotonic (never overshoots between two increasing points, which a
    // naive Catmull-Rom spline through these 5 unevenly-spaced points can do).
    static const std::vector<SpeedPortalManager::CurvePoint> pts = [] {
        std::vector<std::pair<float, float>> raw = {
            {0.5f, 0.7f}, {1.0f, 0.9f}, {2.0f, 1.1f}, {3.0f, 1.3f}, {4.0f, 1.6f}
        };
        size_t n = raw.size();
        std::vector<float> secants(n - 1);
        for (size_t i = 0; i + 1 < n; i++) {
            secants[i] = (raw[i + 1].second - raw[i].second) / (raw[i + 1].first - raw[i].first);
        }

        std::vector<float> tangents(n);
        tangents[0] = secants[0];
        tangents[n - 1] = secants[n - 2];
        for (size_t i = 1; i + 1 < n; i++) {
            if (secants[i - 1] * secants[i] <= 0.f) {
                tangents[i] = 0.f; // local extremum -> flatten to preserve monotonicity
            } else {
                tangents[i] = (secants[i - 1] + secants[i]) * 0.5f;
            }
        }
        // Fritsch-Carlson clamp: keeps each tangent from overshooting its
        // neighbouring secants.
        for (size_t i = 0; i + 1 < n; i++) {
            if (secants[i] == 0.f) {
                tangents[i] = 0.f;
                tangents[i + 1] = 0.f;
                continue;
            }
            float a = tangents[i] / secants[i];
            float b = tangents[i + 1] / secants[i];
            float h = std::hypot(a, b);
            if (h > 3.f) {
                float t = 3.f / h;
                tangents[i] = t * a * secants[i];
                tangents[i + 1] = t * b * secants[i];
            }
        }

        std::vector<SpeedPortalManager::CurvePoint> out;
        out.reserve(n);
        for (size_t i = 0; i < n; i++) out.push_back({raw[i].first, raw[i].second, tangents[i]});
        return out;
    }();
    return pts;
}

float SpeedPortalManager::speedToTimeMod(float speed) {
    auto const& pts = curve();

    if (speed <= pts.front().speed) {
        auto const& p0 = pts.front();
        return p0.timeMod + (speed - p0.speed) * p0.tangent; // linear off boundary tangent
    }
    if (speed >= pts.back().speed) {
        auto const& p1 = pts.back();
        return p1.timeMod + (speed - p1.speed) * p1.tangent;
    }

    for (size_t i = 0; i + 1 < pts.size(); i++) {
        auto const& p0 = pts[i];
        auto const& p1 = pts[i + 1];
        if (speed < p0.speed || speed > p1.speed) continue;

        float h = p1.speed - p0.speed;
        float t = (speed - p0.speed) / h;
        float t2 = t * t, t3 = t2 * t;
        // Standard cubic Hermite basis functions.
        float h00 = 2*t3 - 3*t2 + 1;
        float h10 = t3 - 2*t2 + t;
        float h01 = -2*t3 + 3*t2;
        float h11 = t3 - t2;
        return h00*p0.timeMod + h10*h*p0.tangent + h01*p1.timeMod + h11*h*p1.tangent;
    }
    return 0.9f; // fallback, shouldn't be reached
}

int SpeedPortalManager::registerSpeed(float speed) {
    auto it = std::find(m_orderedSpeeds.begin(), m_orderedSpeeds.end(), speed);
    if (it != m_orderedSpeeds.end()) {
        return static_cast<int>(it - m_orderedSpeeds.begin());
    }
    m_orderedSpeeds.push_back(speed);
    std::sort(m_orderedSpeeds.begin(), m_orderedSpeeds.end());
    auto newIt = std::find(m_orderedSpeeds.begin(), m_orderedSpeeds.end(), speed);
    return static_cast<int>(newIt - m_orderedSpeeds.begin());
}

void SpeedPortalManager::spawnMiniMarkers(GJBaseGameLayer* layer) {
    for (auto* obj : CCArrayExt<GameObject*>(layer->m_objects)) {
        auto* portal = typeinfo_cast<CustomSpeedPortal*>(obj);
        if (!portal) continue;

        float frac = portal->getSpeedValue() - std::floor(portal->getSpeedValue());
        if (std::fabs(frac - 0.5f) > 0.001f) continue; // only X.5 speeds get a marker

        // Cosmetic-only duplicate, offset a short distance ahead along the
        // portal's facing direction, scaled down to read as "mini" -- same
        // approach vanilla GD uses for its own mini portals (same sprite,
        // just smaller), rather than a separate icon asset. This is a
        // marker, not a second trigger -- it doesn't need its own touch
        // logic since the portal it belongs to already set the time mod.
        auto* marker = CCSprite::createWithSpriteFrameName("speed-portal-icon.png"_spr);
        if (!marker) continue;
        marker->setScale(0.55f);
        marker->setPosition(obj->getPosition() + CCPoint{20.f, 0.f});
        marker->setColor(obj->getColor());
        marker->setOpacity(obj->getOpacity());
        layer->m_objectLayer->addChild(marker);
    }
}
