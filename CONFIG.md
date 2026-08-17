# Configuration Reference

## Config File Location

`/path/to/azeroth-server/etc/modules/ahinjector.conf`

## Configuration Options

### AHInjector.Enabled

| Property | Value |
|----------|-------|
| **Type** | Boolean |
| **Default** | `1` (Enabled) |
| **Valid Values** | `0` = Disabled, `1` = Enabled |

Enables or disables the entire AH Injector module.

```ini
AHInjector.Enabled = 1
```

### AHInjector.UpdateInterval

| Property | Value |
|----------|-------|
| **Type** | Integer (milliseconds) |
| **Default** | `300000` (5 minutes) |
| **Minimum** | `1000` (1 second) |
| **Recommended Range** | `60000` - `3600000` (1 min - 1 hour) |

How often the injector runs its injection cycle.

```ini
# 1 minute
AHInjector.UpdateInterval = 60000

# 5 minutes (default)
AHInjector.UpdateInterval = 300000

# 15 minutes
AHInjector.UpdateInterval = 900000

# 1 hour
AHInjector.UpdateInterval = 3600000
```

### AHInjector.ItemsPerCycle

| Property | Value |
|----------|-------|
| **Type** | Integer |
| **Default** | `10` |
| **Minimum** | `1` |
| **Maximum** | `100` |

Maximum number of items to inject per cycle. The injector processes items in the order they appear in `ItemList` and stops after this many successful injections.

```ini
# Inject up to 5 items per cycle
AHInjector.ItemsPerCycle = 5

# Default: 10 items per cycle
AHInjector.ItemsPerCycle = 10

# Aggressive: 20 items per cycle
AHInjector.ItemsPerCycle = 20
```

### AHInjector.ItemList

| Property | Value |
|----------|-------|
| **Type** | String (comma-separated) |
| **Default** | `""` (empty - no items injected) |
| **Format** | `entry:count:minBid:buyout:durationHours` |

Comma-separated list of items to inject. Each item entry has 5 colon-separated values.

```ini
# Format: entry:count:minBid:buyout:durationHours
AHInjector.ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24"
```

#### ItemList Format Details

| Position | Field | Type | Description | Example |
|----------|-------|------|-------------|---------|
| 1 | `entry` | Integer | Item template ID (from `item_template` table) | `19019` |
| 2 | `count` | Integer | Stack size (1-200) | `1` |
| 3 | `minBid` | Integer (copper) | Starting bid price | `100000` (10g) |
| 4 | `buyout` | Integer (copper) | Buyout price | `200000` (20g) |
| 5 | `durationHours` | Integer | Auction duration in hours | `48` |

#### ItemList Examples

```ini
# Single item
AHInjector.ItemList = "19019:1:100000:200000:48"

# Multiple items
AHInjector.ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24"

# With spaces (allowed)
AHInjector.ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48"

# Empty (no items injected)
AHInjector.ItemList = ""
```

#### Common Item Entries

| Item | Entry | Description |
|------|-------|-------------|
| Thunderfury, Blessed Blade of the Windseeker | 19019 | Legendary sword |
| Reins of the Azure Netherwing Drake | 44151 | Mount |
| Titansteel Destroyer | 43954 | Epic weapon |
| Grand Black War Mammoth | 44231 | Mount |
| Reins of the Swift Spectral Tiger | 49283 | Mount |

> Find more item entries in `item_template` table or use `.lookup item <name>` in-game.

### AHInjector.InjectorGuid

| Property | Value |
|----------|-------|
| **Type** | Integer |
| **Default** | `1002000` |
| **Requirement** | Must match the GUID in `ah_injector_bot.sql` |

The GUID of the virtual injector character. Must match the SQL that creates the injector character.

```ini
AHInjector.InjectorGuid = 1002000
```

> **Important**: If you change this, you must also update the SQL file and re-run it.

### AHInjector.InjectorName

| Property | Value |
|----------|-------|
| **Type** | String |
| **Default** | `AHInjector` |
| **Max Length** | 12 characters |

The character name of the virtual injector.

```ini
AHInjector.InjectorName = "AHInjector"
```

### AHInjector.InjectorAccount

| Property | Value |
|----------|-------|
| **Type** | String |
| **Default** | `AHInjector` |

Account name for the virtual injector (currently unused, reserved for future use).

```ini
AHInjector.InjectorAccount = "AHInjector"
```

---

## WorldServer Configuration (Required)

### AllowTwoSide.Interaction.Auction

**File**: `/path/to/azeroth-server/etc/worldserver.conf`

```ini
AllowTwoSide.Interaction.Auction = 1
```

| Value | Effect |
|-------|--------|
| `0` (Default) | Separate Alliance/Horde/Neutral AHs |
| `1` | All AHs become Neutral (cross-faction) |

> **Required for cross-faction visibility**. Without this, injected items only appear in neutral AHs (Gadgetzan, Booty Bay, Everlook).

---

## Database Configuration (Required)

All CharacterDatabase prepared statements must use `CONNECTION_BOTH` instead of `CONNECTION_SYNCH` or `CONNECTION_ASYNC`.

See [INSTALL.md](INSTALL.md#step-1-prepare-database) for the complete fix.

---

## Example Configurations

### Minimal Configuration

```ini
[AHInjector]
Enabled = 1
ItemList = "19019:1:100000:200000:48"
```

### Production Configuration

```ini
[AHInjector]
Enabled = 1
UpdateInterval = 300000          # 5 minutes
ItemsPerCycle = 10
ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24, 44231:1:1000000:2000000:72"
InjectorGuid = 1002000
InjectorName = "AHInjector"
```

### Aggressive Economy Seeding

```ini
[AHInjector]
Enabled = 1
UpdateInterval = 60000           # 1 minute
ItemsPerCycle = 20
ItemList = "19019:1:100000:200000:48, 44151:1:500000:1000000:48, 43954:1:200000:400000:24, 44231:1:1000000:2000000:72, 49283:1:5000000:10000000:48"
```

### Testing Configuration (Fast Cycle)

```ini
[AHInjector]
Enabled = 1
UpdateInterval = 10000           # 10 seconds
ItemsPerCycle = 5
ItemList = "19019:1:10000:20000:1"  # Cheap, short duration for testing
```

---

## Environment Variables (Alternative)

All config options can also be set via environment variables:

| Config Option | Environment Variable |
|---------------|---------------------|
| `AHInjector.Enabled` | `AC_AHINJECTOR_ENABLED` |
| `AHInjector.UpdateInterval` | `AC_AHINJECTOR_UPDATE_INTERVAL` |
| `AHInjector.ItemsPerCycle` | `AC_AHINJECTOR_ITEMS_PER_CYCLE` |
| `AHInjector.ItemList` | `AC_AHINJECTOR_ITEM_LIST` |
| `AHInjector.InjectorGuid` | `AC_AHINJECTOR_INJECTOR_GUID` |
| `AHInjector.InjectorName` | `AC_AHINJECTOR_INJECTOR_NAME` |

Example:
```bash
export AC_AHINJECTOR_ENABLED=1
export AC_AHINJECTOR_UPDATE_INTERVAL=60000
export AC_AHINJECTOR_ITEM_LIST="19019:1:100000:200000:48"
./bin/worldserver
```

---

## Logging

The module logs to the `modules.ahinjector` category. Configure log level in `worldserver.conf`:

```ini
Logger.modules.ahinjector=4,Console Server
```

Log levels:
- `0` = Disabled
- `1` = Error only
- `2` = Warning
- `3` = Info (default)
- `4` = Debug
- `5` = Trace