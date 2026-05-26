# Atmospheric Liquid Pad — Project Context

Use this file when starting a new Cursor chat on this repo.

## What This Is

**Atmospheric Liquid Pad** — a polyphonic cinematic Liquid DnB pad synth.

- **Template:** [Pamplejuce](https://github.com/sudara/pamplejuce) (JUCE + CMake + C++23)
- **Company:** TRAUMASPIRAL
- **CMake target:** `AtmosphericLiquidPad`
- **Product name (DAW):** `Atmospheric Liquid Pad`
- **Formats:** Standalone, AU, VST3, AUv3
- **Build dir (Windows):** `build/` (Visual Studio generator)
- **Plugin logic:** `source/`

## Design Goals

- Warm, submerged atmospheric tone (Liquid DnB / cinematic ambient)
- Emotional pad movement, soft stereo clouding
- Macro-driven workflow (Serum-style) — not a knob-per-parameter synth yet
- No harsh EDM supersaw behavior
- DAW-native: APVTS, automation, state save/load

## Control Architecture

```
APVTS → MacroManager → ModulationMatrix → DSP (AIR/MOTION/WIDTH/WARMTH)
APVTS → MacroManager → MacroMapping glue → matrix accumulators
APVTS → MacroManager → computeEffectiveSpaceAmount() → SpaceReverb
```

Per-block: macro skip → matrix evaluate (primary + glue) → apply modulations → SPACE amount → render voices → effect chain.

**Rules for agents:**
- No direct GUI → DSP writes; use APVTS + attachments
- Do not change DSP architecture unless explicitly asked
- Bipolar macros (AIR/MOTION/WIDTH/WARMTH): **0.5 = neutral** (zero matrix contribution)
- SPACE: **0.0 = off**, unidirectional 0→1, ease-out shaping
- Audio thread must stay realtime-safe (no allocations/locks in `processBlock`)

## Audio Signal Chain

```
Per voice: UnisonSection (default 3 voices) → Filter → ADSR
Bus: Chorus → Main Reverb (Reverb.h) → SpaceReverb → Output
```

## Macros (5)

| Macro   | Default | Neutral / off | Shaping | Primary effect |
|---------|---------|---------------|---------|----------------|
| AIR     | 0.5     | 0.5           | Bipolar piecewise (MacroMapping) | Filter cutoff (dominant), minimal reverb trim |
| MOTION  | 0.5     | 0.5           | Bipolar | LFO depth, chorus depth |
| WIDTH   | 0.5     | 0.5           | Bipolar | Chorus mix + depth |
| WARMTH  | 0.5     | 0.5           | Bipolar | Filter resonance softening + cutoff darkening, reverb damping |
| SPACE   | 0.0     | 0.0           | Unidirectional ease-out `1-(1-x)^1.92` | Filtered atmospheric reverb (~10s tail at max) |

Default matrix routing: `MacroMapping::configureDefaultRouting()` in `source/Control/MacroMapping.cpp`.

Also routed: **Envelope → FilterCutoff** (audio-rate swell on attack).

### Perceptual glue (MacroMapping)

Soft cross-domain bleed (~5–12% of primary depths) applied after primary routing via `accumulateGlueModulations()`. Zero at bipolar 0.5; activation begins outside ±0.10 of center with `pow(t, 2.8)` scaling. SPACE glue is unidirectional (onset above ~0.20 raw SPACE).

| Source | Bleed target | Intent |
|--------|--------------|--------|
| AIR | Reverb wet, chorus mix, SPACE amount | Openness ↔ width ↔ cloud |
| SPACE | Reverb wet, chorus depth | Brightness halo + motion diffusion |
| WIDTH | LFO depth | Wide → slightly more animated |
| MOTION | Reverb damping | Movement → spatial smear |
| WARMTH | Filter resonance, reverb damping | Darkening ↔ reduced cloud clarity |

## Key DSP Defaults

**Filter** (`source/DSP/Filter.h`):
- Cutoff: 400–2000 Hz, default **1050 Hz**
- Resonance: 0.5–1.15, default 0.85

**Unison** (`source/DSP/UnisonConfig.h`):
- Default **3 voices** (1–4), subtle detune + pan + analog drift
- `unisonCount == 1` uses legacy dual-saw `OscillatorSection`

**SPACE reverb** (`source/DSP/SpaceReverb.h`):
- Wet-only parallel mix; bypass when smoothed amount &lt; ~0.002
- Send path: tap → **HPF 800 Hz** → **LPF 5000 Hz** → pre-delay → reverb → wet return
- Filters use `StateVariableTPTFilter` (~12 dB/oct), resonance 0.62 — dry and main reverb untouched
- Macro smoothing: 225 ms; internal param smoothing: 250 ms
- Pre-delay lines **must** call `prepare()` (fixed crash if missing)
- Tail reported as ~10 s for host

## Factory Presets

- **9 embedded presets** in `assets/presets/` (APVTS XML macro snapshots)
- Loaded at startup by `source/Control/PresetManager.h` via BinaryData
- JUCE program API wired in `PluginProcessor` (`getNumPrograms`, `setCurrentProgram`, etc.)
- GUI selector in `MainEditor` — Presets panel with ComboBox + prev/next buttons

| Preset | Hero macro(s) | Character |
|--------|---------------|-----------|
| Init | — | Neutral baseline (0.5 / SPACE 0.0) |
| Submerged | WARMTH, low AIR | Dark, close, no cloud |
| Liquid Halo | SPACE, WIDTH | Airy halo, moderate width |
| Cinematic Cloud | SPACE | Maximum filtered cloud; restrained AIR |
| Drifting Motion | MOTION | Swell and animation dominant |
| Wide Mist | WIDTH, SPACE | Stereo field + atmospheric cloud |
| Deep Warmth | WARMTH | Dense, soft, minimal space |
| Shimmer Air | AIR | Bright openness, light cloud |
| Amber Drift | WARMTH, MOTION | Warm animated drift |

## Source Layout

| Area | Files |
|------|-------|
| Processor | `source/PluginProcessor.h/.cpp` |
| Engine | `source/DSP/SynthEngine.h` |
| Voice chain | `source/DSP/Voice.cpp`, `UnisonSection.h`, `Filter.h` |
| FX | `source/DSP/EffectChain.h`, `Reverb.h`, `SpaceReverb.h`, `ChorusSection.h` |
| Control | `source/Control/MacroManager.h`, `MacroMapping.*`, `ModulationMatrix.*`, `Parameters.h`, `PresetManager.h` |
| GUI | `source/GUI/MainEditor.*`, `MacroKnob.h` |
| Presets | `assets/presets/*.xml` |
| Dead code | `source/PluginEditor.*` (unused) |

## GUI Philosophy

- Minimal cinematic interface
- Large macro-centric controls
- Emotional / atmospheric aesthetic
- Avoid crowded technical layouts
- Macros remain visually dominant over engine/preset controls

## Non-Goals

- Not an EDM supersaw synth
- Not a modular workstation
- Not a knob-per-parameter interface
- Avoid harsh top-end brightness
- Avoid aggressive transient/pluck behavior
- Avoid overstacked unison density

## Current Development Status

Completed:
- APVTS integration
- State save/load
- Modulation matrix
- Cinematic unison
- SPACE reverb system (filtered atmospheric send)
- Envelope → filter routing
- Factory presets + preset browser
- Macro contrast refinement
- Perceptual glue layer
- Musical product pass (preset re-tuning + macro micro-tweaks)

Current focus:
- DAW validation / listening sessions
- Additional factory presets or user save/load (if needed)

Future:
- Expanded engine controls (APVTS)
- User preset save/load to disk
- Performance optimization
- Visual polish
