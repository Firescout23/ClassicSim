#pragma once

#include <QString>

#include "Utils/Check.h"

enum class ItemStats: int {
    Agility = 0,
    Intellect,
    Spirit,
    Stamina,
    Strength,
    Armor,
    Defense,
    DodgeChance,
    ParryChance,
    ResistanceArcane,
    ResistanceFire,
    ResistanceFrost,
    ResistanceHoly,
    ResistanceNature,
    ResistanceShadow,
    ResistanceAll,
    SkillAxe,
    SkillDagger,
    SkillFist,
    SkillMace,
    SkillSword,
    Skill2hAxe,
    Skill2hMace,
    Skill2hSword,
    SkillBow,
    SkillCrossbow,
    SkillGun,
    HitChance,
    CritChance,
    AttackSpeedPercent,
    MeleeAttackSpeedPercent,
    RangedAttackSpeedPercent,
    CastingSpeedPercent,
    AttackPower,
    MeleeAttackPower,
    RangedAttackPower,
    APVersusBeast,
    APVersusDemon,
    APVersusDragonkin,
    APVersusElemental,
    APVersusGiant,
    APVersusHumanoid,
    APVersusMechanical,
    APVersusUndead,
    FlatWeaponDamage,
    ManaPer5,
    HealthPer5,
    ManaSkillReduction,
    SpellDamage,
    SpellDamageArcane,
    SpellDamageFire,
    SpellDamageFrost,
    SpellDamageHoly,
    SpellDamageNature,
    SpellDamageShadow,
    SpellDamageVersusBeast,
    SpellDamageVersusDemon,
    SpellDamageVersusDragonkin,
    SpellDamageVersusElemental,
    SpellDamageVersusGiant,
    SpellDamageVersusHumanoid,
    SpellDamageVersusMechanical,
    SpellDamageVersusUndead,
    SpellCrit,
    SpellHit,
    SpellPenetration,
};

ItemStats get_item_stats_from_string(const QString& item_stat);
