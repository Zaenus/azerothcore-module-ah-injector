# AzerothCore Module: AH Injector

[![Build Status](https://github.com/azerothcore/azerothcore-module-ah-injector/actions/workflows/build.yml/badge.svg)](https://github.com/azerothcore/azerothcore-module-ah-injector/actions/workflows/build.yml)
[![Code Style](https://github.com/azerothcore/azerothcore-module-ah-injector/actions/workflows/codestyle.yml/badge.svg)](https://github.com/azerothcore/azerothcore-module-ah-injector/actions/workflows/codestyle.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![AzerothCore](https://img.shields.io/badge/AzerothCore-3.3.5a-blue.svg)](https://www.azerothcore.org)

## Overview

**AH Injector** is an AzerothCore module that automatically injects specified items into the neutral Auction House at configurable intervals. Perfect for populating empty auction houses on low-population realms or maintaining a baseline economy.

### Features

- **Automated Injection**: Injects configured items into the neutral Auction House at regular intervals
- **Cross-Faction Support**: Works with `AllowTwoSide.Interaction.Auction = 1` for cross-faction visibility
- **Configurable Items**: Define items via simple `entry:count:minBid:buyout:durationHours` format
- **Duplicate Prevention**: Skips items already listed by the injector
- **Flexible Scheduling**: Configurable interval and items per cycle
- **Virtual Injector Character**: Uses a dedicated character (GUID 1002000) as the auction owner

### Requirements

- **AzerothCore** 3.3.5a (Playerbot branch or newer)
- **Cross-Faction AH Enabled**: `AllowTwoSide.Interaction.Auction = 1` in `worldserver.conf`
- **Database Fixes**: All prepared statements must use `CONNECTION_BOTH` (see [INSTALL.md](INSTALL.md))
- **Dedicated Injector Account**: The injector character (GUID 1002000) must be on a
  non-PlayerBots account. On a random-bot account, PlayerBots will log the character
  in and reset its inventory, deleting the virtual auction items from `item_instance`
  so the injected auctions disappear after restart (see [INSTALL.md](INSTALL.md))

### Quick Start

1. **Install Module** (see [INSTALL.md](INSTALL.md))
2. **Configure** `etc/modules/ahinjector.conf`:
   ```ini
   AHInjector.Enabled = 1
   AHInjector.UpdateInterval = 300000        # 5 minutes
   AHInjector.ItemsPerCycle = 10
   AHInjector.ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48"
   AHInjector.InjectorGuid = 1002000
   ```
3. **Enable Cross-Faction AH** in `worldserver.conf`:
   ```ini
   AllowTwoSide.Interaction.Auction = 1
   ```
4. **Run** worldserver - items will inject every 5 minutes

### Configuration

See [CONFIG.md](CONFIG.md) for complete configuration reference.

### How It Works

1. Module loads on server startup, creates virtual injector character (GUID 1002000)
2. Every `UpdateInterval` milliseconds, injects up to `ItemsPerCycle` items
3. For each item in `ItemList`:
   - Checks if item already listed by injector (by itemEntry + owner GUID)
   - If not listed: creates auction with configured bid/buyout/duration
   - Saves virtual item to `item_instance` table (for searcher visibility)
   - Adds auction to neutral AH (houseId=7)
3. Logs each injection to console/logs

### ItemList Format

```
entry:count:minBid:buyout:durationHours
```

Example:
```
19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24
```

| Field | Description | Example |
|-------|-------------|---------|
| entry | Item template ID | `19019` (Thunderfury) |
| count | Stack size | `1` |
| minBid | Starting bid (copper) | `100000` (10g) |
| buyout | Buyout price (copper) | `200000` (20g) |
| durationHours | Auction duration | `48` (48 hours) |

### Building

```bash
cd /path/to/azerothcore
./modules/create_module.sh mod-ah-injector
# (or copy module to modules/mod-ah-injector)
cd build
cmake .. -DMODULES=static -DSCRIPTS=static
make -j$(nproc)
make install
```

### Links

- [Installation Guide](INSTALL.md)
- [Configuration Reference](CONFIG.md)
- [Changelog](CHANGELOG.md)
- [AzerothCore Module Catalogue](https://www.azerothcore.org/wiki/Module-Catalogue)
- [Report Issues](https://github.com/azerothcore/azerothcore-module-ah-injector/issues)

### License

MIT License - see [LICENSE](LICENSE) for details.