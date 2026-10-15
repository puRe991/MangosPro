-- MangosPro: Bloodlust (2825) and Heroism (32182) hit the whole raid within range
-- instead of the shaman's party only.
-- Target B: 33 (TARGET_ENUM_UNITS_PARTY_AOE_AT_SRC_LOC) -> 56 (TARGET_ENUM_UNITS_RAID_WITHIN_CASTER_RANGE)
-- Apply to the world database (tbcmangos) and restart mangosd.
-- Revert: set the three EffectImplicitTargetB columns back to 33.

UPDATE `spell_template`
SET `EffectImplicitTargetB1` = 56,
    `EffectImplicitTargetB2` = 56,
    `EffectImplicitTargetB3` = 56
WHERE `Id` IN (2825, 32182);
