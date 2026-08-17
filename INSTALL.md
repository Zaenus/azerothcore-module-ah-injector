# Installation Guide

## Prerequisites

- **AzerothCore** 3.3.5a (Playerbot branch recommended)
- **CMake** 3.20+
- **MySQL/MariaDB** 8.0+ / 10.5+
- **C++20** compatible compiler (GCC 11+, Clang 13+, MSVC 19.30+)

## Step 1: Prepare Database

### Required Database Fixes

The module requires all CharacterDatabase prepared statements to support both synchronous and asynchronous connections. Apply these changes to your AzerothCore source:

**File: `src/server/database/Database/Implementation/CharacterDatabase.cpp`**
```cpp
// Change all CONNECTION_SYNCH and CONNECTION_ASYNC to CONNECTION_BOTH
// Find/replace: CONNECTION_SYNCH -> CONNECTION_BOTH
// Find/replace: CONNECTION_ASYNC -> CONNECTION_BOTH
```

**File: `src/server/database/Database/Implementation/LoginDatabase.cpp`**
```cpp
// Same: CONNECTION_SYNCH -> CONNECTION_BOTH
```

**File: `src/server/database/Database/Implementation/WorldDatabase.cpp`**
```cpp
// Same: CONNECTION_SYNCH -> CONNECTION_BOTH
```

**File: `src/server/database/Database/Implementation/PlayerbotsDatabase.cpp`**
```cpp
// Same: CONNECTION_SYNCH -> CONNECTION_BOTH
```

> **Why?** The auction house searcher runs in worker threads and needs synchronous database access. Without this fix, you'll get "Could not fetch prepared statement X on database acore_characters, connection type: synchronous" errors.

## Step 2: Install Module

### Option A: Using create_module.sh (Recommended)

```bash
cd /path/to/azerothcore/modules
./create_module.sh mod-ah-injector
# Enter "mod-ah-injector" when prompted
```

### Option B: Manual Copy

```bash
cd /path/to/azerothcore/modules
git clone https://github.com/azerothcore/azerothcore-module-ah-injector.git mod-ah-injector
# Or copy the module directory manually
```

## Step 3: Build

```bash
cd /path/to/azerothcore
mkdir -p build && cd build
cmake .. -DMODULES=static -DSCRIPTS=static -DCMAKE_INSTALL_PREFIX=/path/to/azeroth-server
make -j$(nproc)
make install
```

### Build Options

| Option | Description |
|--------|-------------|
| `-DMODULES=static` | Build modules statically (recommended) |
| `-DMODULES=dynamic` | Build as shared libraries |
| `-DSCRIPTS=static` | Build scripts statically |
| `-DBUILD_TESTING=ON` | Enable unit tests |
| `-DNOPCH=1` | Disable precompiled headers (for debugging) |

## Step 4: Database Setup

The module includes SQL to create the virtual injector character. Run it on your `acore_characters` database:

```bash
mysql -u acore -p acore_characters < /path/to/azeroth-server/etc/modules/ah_injector_bot.sql
```

Or manually execute the SQL from `data/sql/characters/base/ah_injector_bot.sql`:

```sql
DELETE FROM `characters` WHERE `guid` = 1002000;

INSERT INTO `characters` 
(`guid`, `account`, `name`, `race`, `class`, `gender`, `level`, `money`, ...)
VALUES
(1002000, 1, 'AHInjector', 1, 1, 0, 80, 100000000, ...);
```

## Step 5: Configuration

### 1. Copy Config Template

```bash
cp /path/to/azeroth-server/etc/modules/ahinjector.conf.dist \
   /path/to/azeroth-server/etc/modules/ahinjector.conf
```

### 2. Edit Configuration

Edit `/path/to/azeroth-server/etc/modules/ahinjector.conf`:

```ini
[AHInjector]
Enabled = 1
UpdateInterval = 300000        # 5 minutes (milliseconds)
ItemsPerCycle = 10             # Max items per injection cycle
ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24"
InjectorGuid = 1002000
InjectorName = "AHInjector"
```

### 3. Enable Cross-Faction AH (Required)

Edit `/path/to/azeroth-server/etc/worldserver.conf`:

```ini
AllowTwoSide.Interaction.Auction = 1
```

> **Important**: Without this setting, items will only be visible in the neutral AH (Gadgetzan/Booty Bay/Everlook) but not in faction-specific AHs. With this enabled, ALL auction houses become neutral and show the same items.

## Step 6: Start Server

```bash
cd /path/to/azeroth-server
./bin/worldserver
```

## Step 7: Verify Installation

### Check Logs

Look for these messages in the console/logs:

```
AH Injector initialized: Injector GUID=1002000, Name=AHInjector
AH Injector config loaded: Enabled=true, Interval=300000ms, ItemsPerCycle=10, ItemCount=3
AH Injector: Injected item 19019 x1 (bid=100000, buyout=200000, duration=48h)
AH Injector: Injected item 44151 x1 (bid=500000, buyout=1000000, duration=48h)
```

### Verify in Database

```sql
-- Check injector character exists
SELECT guid, name, money FROM characters WHERE guid = 1002000;

-- Check auctions in neutral AH
SELECT id, houseid, itemguid, itemowner, startbid, buyoutprice 
FROM auctionhouse WHERE houseid = 7;

-- Check items in item_instance
SELECT guid, owner_guid, itemEntry FROM item_instance WHERE owner_guid = 1002000;
```

### In-Game Verification

1. Go to any Auction House (Exodar, Orgrimmar, Booty Bay, etc.)
2. Search for injected items: "Thunderfury", "Azure Netherwing Drake", "Titansteel Destroyer"
3. Items should appear with the configured prices

## Troubleshooting

### "Could not fetch prepared statement X on database acore_characters"

**Cause**: Prepared statements not prepared for synchronous connections.

**Fix**: Apply the database fixes in Step 1 (change all `CONNECTION_SYNCH` and `CONNECTION_ASYNC` to `CONNECTION_BOTH`).

### Items not appearing in Auction House

1. **Check cross-faction AH is enabled**: `AllowTwoSide.Interaction.Auction = 1`
2. **Check items exist in DB**: Query `auctionhouse` table for `houseid=7`
3. **Check items in item_instance**: Items must exist in `item_instance` with correct owner
4. **Check logs**: Look for "AH Injector: Injected item..." messages

### Items disappear after server restart

Virtual items are now saved to `item_instance` table, so they persist across restarts. If still missing:
1. Check `item_instance` table for items with `owner_guid=1002000`
2. Verify `LoadAuctionItems` runs at startup (check logs)

### "Assertion failed: m_mStmt" / "Could not fetch prepared statement"

This indicates a synchronous query using a statement only prepared for async. Ensure ALL CharacterDatabase statements use `CONNECTION_BOTH`.

## Upgrading

```bash
cd /path/to/azerothcore/modules/mod-ah-injector
git pull
cd /path/to/azerothcore/build
make -j$(nproc)
make install
```

Then restart worldserver.

## Uninstalling

1. Remove module from build: `cd build && cmake .. -DMODULES=static -DDISABLED_AC_MODULES="mod-ah-injector" && make install`
2. Remove config: `rm /path/to/azeroth-server/etc/modules/ahinjector.conf`
3. Remove injector character: `DELETE FROM characters WHERE guid = 1002000;`
3. Remove auctions: `DELETE FROM auctionhouse WHERE itemowner = 1002000;`
4. Remove items: `DELETE FROM item_instance WHERE owner_guid = 1002000;`