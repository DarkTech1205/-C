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

- **The include is fixed for real.** Your `geode-deps` diagnostic log gave
  the actual answer: the header is
  `smjs.object-collab/include/object_collab.hpp` (underscore, not hyphen,
  and nested under `include/` — my three earlier guesses all missed one of
  those two details). All five `.hpp` files now use the confirmed path, and
  the diagnostic block in `CMakeLists.txt` has been removed since it did
  its job.
- **Still unverified:** the actual method names on `ObjectInfo::Builder` /
  `ObjectTraits::Builder` / `editor_popup::ValueMenu` etc. Now that we have
  the real header filenames (`ObjectInfo.hpp`, `ObjectTraits.hpp`,
  `DetailsBuilder.hpp`, `EditorPopupConfig.hpp`, `Property.hpp`,
  `ObjectIDSwap.hpp`, `CustomLevelData.hpp`, `CustomObject.hpp`) I tried
  searching for the source publicly to confirm signatures and came up
  empty — those headers aren't indexed anywhere I can reach. The include
  will resolve now; expect the *next* errors (if any) to be about specific
  method names/argument order inside those headers rather than "file not
  found." If that happens, your editor's autocomplete once the SDK is set
  up locally, or IntelliSense in VS Code with the Geode extension, will
  show you the real signatures directly from the header — much faster than
  me guessing blind from outside.

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
