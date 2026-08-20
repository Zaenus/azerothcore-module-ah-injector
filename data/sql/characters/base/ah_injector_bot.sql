/*
 * AH Injector Module - Virtual Injector Character
 * This creates a virtual character that will be the owner of all injected auctions.
 *
 * IMPORTANT (persistence fix):
 * The injector account MUST NOT be a PlayerBots random-bot account. If the
 * character is created on an account that PlayerBots manages (account name
 * matching RandomBotAccountPrefix, or an account registered with type 1/2 in
 * `acore_playerbots.playerbots_account_type`), PlayerBots will register the
 * character as a random bot, log it in, and reset its inventory on login. That
 * deletes the virtual auction items from `item_instance`, so after the next
 * restart the auctions fail to load ("Loaded 0 auction items" / "Loaded 0
 * auctions") and the injector re-creates them, causing duplicate entries.
 *
 * Create a dedicated, non-bot account first (run against `acore_auth`):
 *
 *     INSERT INTO `account` (id, username, salt, verifier, session_key, email, reg_mail,
 *                            failed_logins, locked, online, expansion, Flags, locale, recruiter, totaltime)
 *     VALUES (200, 'AHInjector', UNHEX(REPEAT('00',32)), UNHEX(REPEAT('00',32)),
 *             UNHEX(REPEAT('00',40)), '', '', 0, 0, 0, 2, 0, 0, 0, 0);
 *
 * Adjust the `account` value in the INSERT below to match the account id you
 * created (the example uses 200). Do NOT use an account that PlayerBots uses.
 */

-- Delete existing injector character if exists
DELETE FROM `characters` WHERE `guid` = 1002000;

-- Insert the virtual injector character (only required columns, others use defaults)
INSERT INTO `characters` 
(`guid`, `account`, `name`, `race`, `class`, `gender`, `level`, `money`,
 `position_x`, `position_y`, `position_z`, `map`, `orientation`,
 `playerFlags`, `taximask`, `cinematic`, `logout_time`, `rest_bonus`,
 `trans_x`, `trans_y`, `trans_z`, `trans_o`, `transguid`,
 `extra_flags`, `stable_slots`, `at_login`, `zone`, `death_expire_time`,
 `arenaPoints`, `totalHonorPoints`, `todayHonorPoints`, `yesterdayHonorPoints`,
 `totalKills`, `todayKills`, `yesterdayKills`, `chosenTitle`, `knownCurrencies`,
 `watchedFaction`, `drunk`, `health`, `power1`, `power2`, `power3`, `power4`,
 `power5`, `power6`, `power7`, `latency`, `talentGroupsCount`, `activeTalentGroup`,
 `exploredZones`, `equipmentCache`, `ammoId`, `knownTitles`, `actionBars`,
 `grantableLevels`, `order`, `deleteInfos_Account`, `deleteInfos_Name`, `deleteDate`,
 `innTriggerId`, `extraBonusTalentCount`)
VALUES
(1002000, 200, 'AHInjector', 1, 1, 0, 80, 100000000,
 -8828.5, 624.3, 94.1, 0, 2.5,
 0, '', 0, UNIX_TIMESTAMP(), 0,
 0, 0, 0, 0, 0,
 0, 0, 0, 1, 0,
 0, 0, 0, 0,
 0, 0, 0, 0, 0,
 0, 0, 100, 100, 100, 0, 0,
 0, 0, 0, 0, 1, 0,
 '', '', 0, '', 0,
 0, 0, NULL, NULL, NULL,
 0, 0);