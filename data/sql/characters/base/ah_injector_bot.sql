/*
 * AH Injector Module - Virtual Injector Character
 * This creates a virtual character that will be the owner of all injected auctions
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
(1002000, 1, 'AHInjector', 1, 1, 0, 80, 100000000,
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