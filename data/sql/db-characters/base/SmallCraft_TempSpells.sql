CREATE TABLE IF NOT EXISTS `custom_smallcraft_tempspells` (
	`ID` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT 'ID for this assignment',
	`GUID` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Player\'s GUID',
	`SpellID` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Granted SpellID',
	PRIMARY KEY (`ID`) USING BTREE,
	INDEX `GUID` (`GUID`) USING BTREE
)
COMMENT='Stores any temporary spells that have been granted to characters by the Smallcraft TempSpells feature.'
COLLATE='utf8mb4_unicode_ci'
ENGINE=InnoDB
;
