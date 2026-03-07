# World Editor Roadmap

## Goal

Move the current sandbox/editor shell from a physics-demo-oriented tool into a stable world editor with clear authoring, persistence, and runtime separation.

The current codebase already has:

- editor shell UI
- hierarchy / inspector / debug panels
- PIE lifecycle
- scene switching and scene-level metadata editing
- command/history infrastructure
- project/workspace/prefab/asset pipeline foundations

The next phase should focus on turning those foundations into a real editing workflow rather than continuing to add isolated shortcuts or one-off sandbox behaviors.

## Current Position

The repository is no longer just a physics sandbox.

It already behaves like an early-stage world editor in these areas:

- editor shell exists under `apps/sandbox_dwrite/`
- runtime/editor separation has started
- PIE enter / pause / step / exit loop exists
- scene rename / ordering / asset GUID editing exists
- undo/redo infrastructure exists
- project workspace, package/plugin, prefab semantics, and asset pipeline systems exist

What is still missing is the core authoring loop expected from a real editor:

- persistent scene editing
- full command-driven editing
- viewport authoring tools
- prefab instance workflow
- asset browsing and binding workflow

## Priorities

### P0: Must Do First

These items should be completed before expanding the editor surface further.

1. Unify the editing data model

- Define explicit editor-side concepts for:
  - `World`
  - `Scene`
  - `Entity` / `Body`
  - `Constraint`
  - `AssetRef`
  - `PrefabInstance`
- Reduce direct coupling between UI state and runtime objects.
- Make editor state the source of truth and runtime state the execution copy.

2. Complete scene persistence workflow

- Add:
  - new scene
  - save
  - save as
  - dirty flag
  - confirm-before-switch when modified
- Move the current preset-scene flow toward real editable scene files.

3. Route all edit operations through command/history

- Ensure all user-facing edits go through command bus + history transaction:
  - create
  - delete
  - rename
  - reorder
  - parameter edits
  - asset binding
- Avoid direct mutation paths that bypass undo/redo.

4. Align docs and runtime behavior

- Keep help text, quickstart docs, and actual keybindings in sync.
- Treat editor interaction documentation as part of the feature, not optional follow-up.

### P1: High-Value Editor Features

These features turn the current shell into a usable authoring tool.

1. Viewport authoring tools

- translation / rotation gizmos
- snapping
- box selection
- multi-select
- duplicate / copy / paste

2. Hierarchy panel upgrade

- search/filter polish
- drag reorder
- context menu
- bulk operations
- type/group filters

3. Inspector upgrade

- component-style grouped presentation
- stable numeric editing
- reset-to-default
- mixed-value support for multi-selection

4. Prefab workflow

- instantiate prefab
- show override state
- apply overrides
- revert overrides
- jump to prefab source

5. Asset browser

- browse project assets without manually typing GUIDs
- bind assets from UI selection
- display basic metadata and dependency hints

### P2: Later Expansion

These items matter, but should not block the core editor loop.

1. Large-world support

- chunking
- streaming
- scene partitioning
- layered visibility

2. Scripted tooling

- batch edit tools
- migration helpers
- auto-fix tools

3. Validation and diagnostics

- scene validation panel
- authoring-time error reporting
- stronger recovery flows

4. Release workflow

- standalone runtime/player
- project templates
- packaging profiles

## Recommended Execution Order

1. Scene save/load/dirty workflow
2. Full command-driven edit path
3. Viewport gizmos and multi-selection
4. Prefab instance workflow
5. Asset browser and binding UX

## Why This Order

The current repository does not mainly lack more panels.

It mainly lacks a fully reliable editing loop:

- author changes
- persist them
- undo/redo them
- enter PIE safely
- exit PIE and continue editing

Without that loop, adding more editor surface area will increase complexity faster than capability.

## Definition Of Progress

The editor can be considered to have moved into a stronger “world editor” phase once all of the following are true:

- scenes are real editable assets, not only preset switches
- all major edits are undoable/redone through one command path
- viewport editing is practical without relying on console/debug-only flows
- prefab instances have a visible and stable authoring workflow
- assets can be assigned through an editor UI rather than raw GUID entry

## Immediate Recommendation

The best next implementation step is:

1. define the editor-side scene/world model explicitly
2. complete scene save/load/dirty handling
3. remove remaining direct-edit paths that bypass command/history

That sequence will create the foundation needed for all later world-editor features.
