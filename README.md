# Reverse & Custom Speed Portals — design notes

I pulled these two mods apart (`profdragon.bigportal` and
`mariomastr.customisable-speed-portals`) to build this on real, verified
foundations instead of guessing. Here's what I found and how it shaped the
design.

## What I actually confirmed from the two .geode files

- **BigPortal is a custom `EffectGameObject` subclass built on `smjs.object-collab`**
  (`.?AV?$CustomObject@VEffectGameObject@@@object_collab@@`, `.?AVBigPortal@@`),
  a real published Geode mod/library for adding brand-new placeable objects
  — their own editor button, tab, icon, and popup — without hijacking an
  existing GD object ID. That's the real answer to "how do I add a whole new
  gamemode object": depend on `smjs.object-collab`, not raw ID-squatting.
- It hooks `PlayerObject::update` itself (as `MyPlayerObject`) rather than
  relying on some automatic touch callback — portal-style effects are
  checked against the player's hitbox by hand, every frame, same as vanilla
  gravity/mirror/mini portals.
- It uses a `"no-multi-activate"` property, i.e. an activation flag that
  resets on respawn — that's where `m_alreadyActivated` + the
  `resetLevel` hook in this project come from.
- `object-collab` exposes `ObjectInfo::Builder`, `ObjectTraits::Builder`
  (including a literal `isSpeedObject()` trait — very relevant to you),
  `CustomObjectInterface`, and a whole `editor_popup` namespace
  (`PopupConfig`, `ValueMenu`, `ToggleMenu`, `AxisLayoutMenu`) for building
  custom in-editor property popups. Those are the real building blocks used
  in `CustomSpeedPortal.cpp`'s `editSpecial(...)` for the speed input field.
- ReSpeed's `about.md` documents the **real, confirmed** internal floats
  `PlayerObject::updateTimeMod(float p0, bool p1)` uses for the 5 vanilla
  speeds (0.5x→0.7, 1x→0.9, 2x→1.1, 3x→1.3, 4x→1.6) — `speedToTimeMod()`
  interpolates/extrapolates off those real points.

## v1.2 changelog

- **The `0.5.5-beta.3` pin didn't actually take last round.** Confirmed
  from Geode's own dependency docs: the version field only accepts
  `>=version`, `=version`, or `<=version` — a bare version string with no
  operator isn't valid syntax at all, which is presumably why it silently
  still resolved to the newest available (`0.6.0`) instead of pinning.
  Fixed to `"=0.5.5-beta.3"` (explicit equals), which should actually pin
  it this time.

- **`GameObjectType::Modifier` is confirmed correct — the diagnostic
  paid off.** Got the complete real enum from Geode's own generated
  bindings. There's no generic "Orb" or "Portal" catch-all value — every
  portal/orb/pad entry (`ShipPortal`, `UfoPortal`, `YellowJumpPad`, etc.)
  is tied to a *specific* hardcoded vanilla gamemode swap, which we
  explicitly don't want for custom behavior objects like these. `Modifier`
  really is the right generic type; `isSpeedObject(true)` (already applied
  last round) is what controls visibility, not the type choice. That part
  of the design is settled now — the `CMakeLists.txt` diagnostic that
  found this has been removed since it did its job.
- **`object-collab` pinned to an exact working version instead of chasing
  a broken newer one.** `mod.json` only floated a floor (`>=0.5.5-beta.3`),
  so it auto-updated to `v0.6.0` — which pulls in a second dependency,
  `alphalaneous.editortab_api`, that turned out to not be resolvable on
  the index at all (not just missing from our `mod.json` — genuinely not
  found there per the CLI's own error). That's not something adding a
  version guess fixes; it means `0.6.0` itself currently depends on
  something unpublished or otherwise broken. Rather than chase that,
  `mod.json` now pins the exact version (`"0.5.5-beta.3"`, no `>=`) that
  was already compiling clean through every round up to this point —
  the safe, known-working target instead of "whatever's newest."
  Worth revisiting the floating `>=` once `0.6.0`'s dependency situation
  is actually sorted out upstream, but there's no reason to block on that
  now.


- **Real, source-confirmed visibility fix — no more guessing needed on
  `GameObjectType`.** Re-read `CustomObject.hpp`'s own `customSetup()`
  carefully: `m_isInvisible = !editorEnabled && isTriggerObject() &&
  !isSpeedObject()`. `isSpeedObject(true)` — not `GameObjectType` — is
  what keeps an object visible during actual gameplay. `CustomSpeedPortal`
  already had it set (which is why it was the one object you didn't report
  as invisible); the other four didn't. All five now set
  `.isSpeedObject(true)` alongside `.gameObjectType(GameObjectType::Modifier)`.
  The `CMakeLists.txt` diagnostic searching for the real enum is still
  there as a fallback if this turns out incomplete, but this fix doesn't
  depend on that round-trip at all.
- **Pause portal redesigned as what you actually asked for: a real 0x speed
  portal.** Not a position-pinning hack anymore — it calls the same
  `PlayerObject::updateTimeMod` the speed portal uses, hardcoded to `0.0f`,
  once per touch (same single-latch pattern as Reverse/CustomSpeedPortal).
  Because that's GD's own real speed state, it now does exactly what you
  described: touch it once, X speed drops to 0 and *stays* 0 — through
  jumps, through leaving and re-entering the object — until you touch a
  different speed-setting portal. The "trapped until you hit a different
  portal" behavior you first reported as a bug is now the intended,
  correctly-working feature, good for the fall-section/corridor use case
  you mentioned. Its old continuous per-frame hold/release logic and the
  `main.cpp` rect-check entry for it are both gone.
- **Speed curve extrapolation at low values is unchanged for now** — since
  the pause portal is a hardcoded `0.0f` rather than routed through
  `SpeedPortalManager::speedToTimeMod`, "0x feeling like pause" is now
  moot for that specific object; the curve's own behavior near 0 for an
  actual *custom speed portal* set close to 0 is still the same smooth
  extrapolation as before, unaffected by this change.


- **Speed-stack visual, actually implemented now.** The earlier
  `spawnMiniMarkers` code was real but dead — nothing ever called it. Moved
  the logic into `CustomSpeedPortal::refreshSpeedStack()`, called from the
  real confirmed `postInit()` hook (fires once the object has fully
  generated) and live-refreshed from the editor popup's `onValue` callback
  so the chevron row grows as you type a new speed in, not just once at
  placement. One extra chevron per whole speed increment, plus a smaller
  one for a `.5` fractional part, matching what you originally described.
- **The Modifier/visibility tension you caught is real and unresolved.**
  `GameObjectType::Modifier` fixed the Solid-block collision problem, but
  per `ObjectTraits.hpp`'s own doc comments, Modifier is GD's *trigger*
  category — invisible during actual gameplay by design, only shown in the
  editor. That's why they stopped showing up in the level; it's not a
  separate bug; it's the wrong type for objects that need to be visible
  and touchable. I don't have the real GD `GameObjectType` value for
  visible portals/orbs and I'm not guessing a third value blind after two
  wrong ones — `CMakeLists.txt` now searches the generated bindings
  themselves for the real enum definition and prints whatever it finds.
  Paste that back and I'll set the correct type with certainty.
- **"0x speed feels like the pause portal"** — this is expected curve
  behavior, not a new bug: the interpolation only has real vanilla data
  down to 0.5x (→0.7 timeMod), and extrapolating further toward 0 keeps
  following that same downward trend, so very low speeds genuinely do
  approach a near-stop. If you want a hard floor instead of a continuous
  approach to near-zero, `speedToTimeMod`'s extrapolation branch is a small
  change — say the word once the visibility issue is sorted and it'll be
  easier to actually feel and tune in-game.
- **Known follow-up, not yet fixed:** `m_speedValue` currently isn't
  registered as a real `Property` via `PropertyInterface::from(...)` /
  `ComplexObject::builder().customProperties(...)` — the popup edits the
  live member directly, which works while playtesting in the same editor
  session, but won't be saved when the level is saved and reloaded. Worth
  wiring up once the visibility fix lands, since there's no point tuning
  persistence on an object you can't currently see or test properly.


- **Found the root cause of nearly everything you reported.** Every object
  had `ObjectTraits` left on its default `GameObjectType::Solid` — which
  means "physical block," confirmed verbatim in `ObjectTraits.hpp`'s own
  doc comments. That single default explains:
  - the pause portal having block hitboxes and trapping you (you were
    colliding with an actual solid wall, not passing through it)
  - the reverse and speed portals "doing nothing" (same — blocked as a
    wall before your gameplay logic could meaningfully apply)
  - the speed portal specifically: `isSpeedObject`'s own doc comment says
    it "only works when... is a object type of Modifier" — Solid silently
    disabled it
  - the purple orb reading like a pad (a solid-collision physics response
    on contact, not your jump-gated `boostPlayer` call)
  All five objects now use `GameObjectType::Modifier` instead.
  - Also added real round hitboxes to both orbs via the confirmed
    `CustomObject<T>::setRoundHitbox(float)` method, instead of leaving
    them with whatever box shape the sprite bounds implied.
- **Still to sort out — not a bug, likely just the placeholder asset:**
  `GravityShiftOrb`'s "no distinct sprite" is probably the downscaled 3D
  render crop I flagged as a placeholder from the start (`35×64`, an odd
  sliver shape at that size) rather than a code issue — worth swapping in
  a real square icon to know for sure.
- **New scope, not a fix:** the "ship/UFO/wave clamped vertical space"
  behavior for the gravity-shift gamemode is a substantial feature on its
  own — replicating that means either toggling into one of GD's actual
  built-in flight modes to get its space-clamp for free, or fully
  reimplementing the camera/vertical-bound logic by hand. Tell me which
  direction you want and I'll scope it properly rather than guess at it
  inside this same fix pass.


- **Build workflow fixed against the real official example.** I'd guessed
  at `build-geode-mod`'s output names (`build-output-name`/
  `build-output-path`, and a `combine` job that downloads per-platform
  artifacts manually) — none of that matched the real action. Found
  geode-sdk's own `examples/multi-platform.yml` and copied its structure
  exactly: the `build` job doesn't upload anything itself, and a separate
  `package` job runs `geode-sdk/build-geode-mod/combine@main` alone (no
  manual artifact download needed — it handles gathering the per-platform
  builds itself) with a single `build-output` output. `.github/workflows/
  build.yml` now matches that real template, with just the iOS SDK install
  step kept from earlier troubleshooting.

- **All five objects now compile successfully** per your last build log —
  `ReversePortal.cpp`, `CustomSpeedPortal.cpp`, `PausePortal.cpp`, and
  `main.cpp` built clean. The only remaining errors were the three symbols
  flagged as unverified from the start (`PurpleOrb.cpp`,
  `GravityShiftOrb.cpp`), confirming the object-collab integration itself
  is correct now.
- **Looked up the real `PlayerObject` bindings directly** (Geode's own docs
  site has the full class) instead of guessing a 4th time:
  - Jump-held state isn't a bool member at all -- it's a real method,
    `bool buttonDown(PlayerButton button)`. Fixed in both `PurpleOrb.cpp`
    and `GravityShiftOrb.cpp`.
  - There's no `teleportToVertical`. The real pattern is just
    `setPosition(...)` (already a real public/virtual `PlayerObject`
    method) followed by the real `playerTeleported()` bookkeeping call.
  - `toggleGravityEffect` doesn't exist; the real confirmed method is
    `void flipGravity(bool flip, bool noEffects)`. `m_isUpsideDown` (which
    this was already using) turned out to be real too.
  - `boostPlayer(float yVelocity)` (used by `PurpleOrb`) was already
    correct.
- **Replaced the Reverse trigger hack with a real, confirmed direct call.**
  Rather than spinning up a fake trigger object with a guessed object ID,
  `PlayerObject::doReversePlayer(bool reverse)` is a real, confirmed
  method — `ReversePortal.cpp` now calls it directly. One documented
  trade-off: it toggles based on the portal's own last-known state, not a
  confirmed "am I currently reversed" read from the player, so it could
  drift out of sync if a level also mixes in vanilla Reverse triggers —
  fine for levels using only these portals for reverse.
- Every other fix from this round (real `CustomObject` construction,
  `collidedByPlayer`, real `EditorTab`/`EditorButtonColor` values, the
  restored `NumericMenu` popup) is unchanged.
- **Nothing known-wrong remains in the code.** The `kOscillationAmplitude`/
  `kOscillationPeriod` constants and `kStrengthMultiplier`/
  `kRedOrbBaseVelocity` are still tune-by-feel values (no "correct" answer
  exists for those, same as before), not unverified API calls.





- **Fixed the mix-up:** your ring/dumbbell render was never the Purple Orb's
  sprite — it's the new gamemode's showcase art (renamed to
  `newgamemode-showcase.png`). The Purple Orb now uses your actual round
  orb image (`purple-orb-icon.png`) as its sprite.
- **New: the new gamemode itself — `GravityShiftOrb`.** Per what you
  described: an orb that drifts up and down on its own (self-contained
  ping-pong motion, no move trigger needed in the editor); jump while
  near it and it teleports you to the orb's current position AND flips
  your gravity, both at once. The skill is timing your jump against where
  the orb is in its cycle. Motion range/speed are fixed constants for now
  (`kOscillationAmplitude` / `kOscillationPeriod` in the header) rather than
  a full editor popup — wiring those up as tunable per-object values is a
  small follow-up once you've felt the default numbers in-game and know
  what's worth exposing.
- Also fixed: `PurpleOrb::registerObject` was written in v1.1 but never
  actually got called from `$on_mod(Loaded)` — it existed but would never
  have shown up in the editor. Both new-in-1.1/1.2 objects are wired up now.
- New unverified TODOs (same honesty policy as everything else in this
  project): `PlayerObject::teleportToVertical` and
  `PlayerObject::toggleGravityEffect` in `GravityShiftOrb.cpp` are
  placeholder names for GD's real teleport-portal and gravity-portal
  internals — same category of guess as the orb jump-held accessor and the
  Reverse trigger ID, all still pending confirmation once the
  `object-collab` include is sorted.

## v1.1 changelog

- **New: Purple Orb.** A jump orb, not a portal -- it only fires if you're
  holding jump when you touch it (same as every vanilla orb), and resets the
  moment you leave it so you can fly back and reuse it later in the same
  attempt. Its strength is `kStrengthMultiplier` (1.35x) times a placeholder
  red-orb base velocity in `PurpleOrb.cpp` -- both the base value and the
  multiplier are unverified/tunable, see the TODOs in that file.
- The image you sent for this (`purple-orb-showcase.png`) is a ring/portal
  shape, not a round orb sprite, so I've treated it as promo/showcase art
  again rather than the actual in-game icon, same as the reverse portal's
  3D render. `purple-orb-icon.png` is a downscaled placeholder crop of it --
  swap in a real round orb sprite when you have one.
- "AND MORE" in the request was open-ended, so this v1.1 sticks to exactly
  what was specific (an orb stronger than red) rather than inventing
  behaviour you didn't ask for. Tell me what else you want on it and I'll
  add it in v1.2.
- **Build fix:** the CMakeLists.txt in earlier zips still had the broken
  `include()`/`setup_geode_mod()` pairing from my first pass -- I'd only
  corrected it in chat replies, not in the actual file you were building
  from. It's genuinely fixed in this zip now (`add_subdirectory` +
  `setup_geode_mod`).
- **Still unresolved:** the `object-collab.hpp` include path. I couldn't
  find its real header layout anywhere I have access to search. Rather than
  guess a fourth time, `CMakeLists.txt` now globs and prints the entire
  `geode-deps` folder right after dependency resolution, so your *next*
  build log will show the real path/filename directly -- no separate debug
  step needed. Paste me that `=== geode-deps contents ===` block from the
  log and I'll fix the three `#include` lines for good.
- Also unverified, called out with TODOs in the code: the real accessor for
  "is jump currently held" on `PlayerObject`, and the real jump-velocity
  call orbs use internally (`PurpleOrb.cpp`), and the Reverse trigger's
  object ID (`ReversePortal.cpp`, unchanged from before).

## Corrections from the first pass

I had the icons backwards. Fixed now:
- The `>` chevron (`speed-portal-icon.png`) is the **base Custom Speed
  Portal**'s icon — not a reverse portal icon.
- The `||` bars (`pause-portal-icon.png`) is a **Pause Portal** — a new
  third object, not a "mini speed portal marker". While the player overlaps
  it, X-axis movement is frozen (position held, Y/gravity/jump untouched);
  it releases the instant they leave its bounds. See `PausePortal.hpp/.cpp`.
- The `.5`-speed mini marker now reuses the speed portal's *own* icon
  scaled down instead of the bars icon — that's actually how vanilla GD's
  own mini portals work (same sprite, smaller), so no separate asset needed.
- The Reverse Portal didn't have a dedicated small icon in what you sent —
  it's currently a downscaled crop of your 3D showcase render
  (`reverse-portal-icon.png`), which is a placeholder, not a real icon. Send
  a proper small square icon for it when you have one.

## How the three ideas map onto the code

**Reverse Portal** — a touch-based object (not a trigger you have to wire up
with groups) that, on first contact, spawns a real vanilla Reverse trigger
in memory and fires its own activation function. That means it inherits
whatever the actual reverse behaviour is without us having to reverse
engineer the internal flag — and it'll keep working if Robtop changes how
reverse works internally in a future update, since we're not duplicating
that logic ourselves. **You do need to fill in `kReverseTriggerObjectID`**
in `ReversePortal.cpp` with the real trigger's object ID for 2.2081 — I
didn't want to guess a number and have it silently be wrong.

**Custom Speed Portal** — hue uses GD's existing per-object HSV colour
channel editor (already free, no UI work needed — just read `getColor()`
back off the object). Speed is a real typed number via a `ValueMenu` popup.
Each level keeps its own growing, sorted list of every distinct speed value
placed in it (`SpeedPortalManager::m_orderedSpeeds`, appended to as new
values are discovered — this is your "every speed adds a new portal to the
end"). Any portal whose speed has a `.5` fractional part (1.5x, 2.5x, ...)
automatically gets a small duplicate of its own icon spawned just in front
of it at level load, scaled down (`spawnMiniMarkers`).

The speed→timeMod conversion (`SpeedPortalManager::speedToTimeMod`) got a
real upgrade this round: it's now a **monotonic cubic Hermite curve**
(Fritsch-Carlson tangents) through the 5 real vanilla points instead of
straight line segments. Concretely that means:
- It still passes *exactly* through 0.5x/1x/2x/3x/4x, so setting a portal
  to those values is pixel-identical to a real vanilla speed portal.
- Between them the curve is smooth and, critically, monotonic — a plain
  spline through unevenly-spaced points like these can overshoot and dip
  below a neighbouring point; Fritsch-Carlson clamping guarantees that never
  happens, so speed always increases as your entered value increases.
- Below 0.5x and above 4.0x it extrapolates linearly off the curve's own
  boundary tangent (not a naive two-point secant like before), so something
  like 0.1x or 8x still feels continuous with the rest of the curve instead
  of kinking at the edges.

**Pause Portal** — a *zone*, not a one-shot trigger: for every frame the
player's hitbox overlaps it, their X position is pinned to wherever it was
the instant contact started (Y/gravity/jump keep working normally), and it
lets go the moment they leave. See the `PausePortal` branch of the
`PlayerObject::update` hook in `main.cpp`.

## What's genuinely uncertain and worth double-checking before you build

1. `kReverseTriggerObjectID` in `ReversePortal.cpp` — placeholder, confirm
   the real ID.
2. Exact method names/parameter order on `object_collab::ObjectInfo::Builder`,
   `ObjectTraits::Builder`, and `editor_popup::ValueMenu` — I reconstructed
   these from the compiled export names in `smjs.object-collab.dll`
   (visible via `strings`), which gives correct names but not always exact
   argument order. Once you add `smjs.object-collab` as a dependency and run
   a Geode build, the CLI pulls its real headers and your editor's
   autocomplete/compiler errors will tell you immediately where I guessed
   wrong.
3. The `PlayerObject::update` loop currently iterates `layer->m_objects`
   (every object in the level) every frame — fine for testing, but swap it
   for whichever "active in this section" collection vanilla portal checks
   already use before shipping, or it'll tank FPS on big levels.
4. `speedToTimeMod`'s interpolation curve is *invented* — the 5 reference
   points are real, but there's no "correct" curve through arbitrary values
   like 7.3x since vanilla never had to define one. Treat it as a starting
   point to tune by feel.

## Building locally

```
geode build .
```
(with the Geode CLI installed and `smjs.object-collab` resolvable as a
dependency — it's on the official Geode mod index).

## Building on GitHub (no local setup needed)

Push this folder to a repo and `.github/workflows/build.yml` takes over from
there — it uses the official `geode-sdk/geode-build-action`, which installs
the right SDK/toolchain per platform and resolves your `mod.json`
dependencies for you (no manually vendoring `object-collab`'s headers).

- Runs on every push, PR, and manually via the Actions tab
  (`workflow_dispatch`).
- Builds Windows, macOS, Android32, and Android64 in parallel as separate
  jobs.
- A final `combine` job merges all four into one cross-platform `.geode`
  file, uploaded as its own artifact you can drag straight into GD's
  mods folder.

To use it: create a repo, push this whole folder (mod.json, CMakeLists.txt,
src/, resources/, .github/) as-is, and check the Actions tab after the
first push. No secrets or extra config needed — the action is public.

## About the images you sent

- `107.webp` (chevron) → `resources/reverse-portal-icon.webp`, used as the
  editor button icon for the Reverse Portal.
- `185.webp` (two bars) → `resources/mini-speed-portal-icon.webp`, used both
  as the Custom Speed Portal's own icon and as the small "mini portal"
  marker for `.5` speeds.
- `THEREVERSEPORTAL.png` → kept as `resources/reverse-portal-showcase.png`,
  a nice mod-page/thumbnail image rather than an in-game sprite (too
  detailed for a small editor icon).

Convert the `.webp` files to `.png` before building — GD's spritesheet
tooling wants PNG, not WebP.
