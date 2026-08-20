# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-17

### Added
- Initial release of AH Injector module for AzerothCore 3.3.5a
- Automated item injection into neutral Auction House
- Configurable injection interval, items per cycle, and item list
- Cross-faction Auction House support via `AllowTwoSide.Interaction.Auction = 1`
- Virtual injector character (GUID 1002000) as auction owner
- Duplicate prevention (skips items already listed by injector)
- Items persisted to `item_instance` table for searcher visibility
- Virtual items saved to database for server restart persistence
- Configurable injection interval, items per cycle, and item list
- Full configuration via `ahinjector.conf`
- Logging to `modules.ahinjector` category

### Configuration Options
- `AHInjector.Enabled` - Enable/disable module (default: 1)
- `AHInjector.UpdateInterval` - Injection cycle interval in ms (default: 300000)
- `AHInjector.ItemsPerCycle` - Max items per injection cycle (default: 10)
- `AHInjector.ItemList` - Comma-separated item definitions (default: empty)
- `AHInjector.InjectorGuid` - Virtual injector GUID (default: 1002000)
- `AHInjector.InjectorName` - Virtual injector name (default: "AHInjector")

### ItemList Format
```
entry:count:minBid:buyout:durationHours
```
Example: `19019:1:100000:200000:48, 44151:1:500000:1000000:48`

### Requirements
- AzerothCore 3.3.5a (Playerbot branch)
- `AllowTwoSide.Interaction.Auction = 1` in worldserver.conf
- All CharacterDatabase prepared statements must use `CONNECTION_BOTH`

### Database Fixes Required
All CharacterDatabase prepared statements must use `CONNECTION_BOTH` instead of `CONNECTION_SYNCH` or `CONNECTION_ASYNC` in:
- `src/server/database/Database/Implementation/CharacterDatabase.cpp`
- `src/server/database/Database/Implementation/LoginDatabase.cpp`
- `src/server/database/Database/Implementation/WorldDatabase.cpp`
- `src/server/database/Database/Implementation/PlayerbotsDatabase.cpp`

### SQL Setup
Virtual injector character created via `data/sql/characters/base/ah_injector_bot.sql`:
- GUID: 1002000
- Name: AHInjector
- Level: 80
- Gold: 100,000,000 copper

---

## [Unreleased]

### Fixed
- **Persistence**: injected auctions no longer disappear after a server restart when
  PlayerBots is enabled. `ah_injector_bot.sql` now requires a dedicated, non-PlayerBots
  account for the injector character. Previously the character was created on account
  `1`, which PlayerBots treats as a random-bot account: it logged the character in and
  reset its inventory, deleting the virtual auction items from `item_instance`. The
  auctions then failed to load after restart and were re-created each cycle, producing
  duplicate entries.
- **Docs**: added account setup + PlayerBots troubleshooting to `INSTALL.md` and README.

### Changed
- `ahinjector.conf.dist` now ships with a populated `ItemList` example so the module
  injects items out of the box instead of silently doing nothing.

### Planned
- Web UI for configuration management
- Item price history tracking
- Dynamic pricing based on market demand
- Per-faction injection options
- Auction expiration notifications
- Integration with AH Bot for market analysis