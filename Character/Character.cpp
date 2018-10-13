
#include "Character.h"
#include "Race.h"
#include "Engine.h"
#include "Equipment.h"
#include "Faction.h"
#include "Talents.h"
#include "Target.h"
#include "CharacterStats.h"
#include "Stats.h"
#include "ActiveProcs.h"
#include "ProcInfo.h"
#include "ActiveBuffs.h"
#include "Spells.h"
#include "CombatRoll.h"
#include "MainhandAttack.h"
#include "OffhandAttack.h"
#include "Weapon.h"
#include "Rotation.h"
#include "PlayerAction.h"
#include <QDebug>

#include "Berserking.h"
#include "BerserkingBuff.h"
#include "BloodFury.h"
#include "BloodFuryBuff.h"

Character::Character(Race* race, EquipmentDb* equipment_db, QObject* parent) :
    QObject(parent)
{
    this->race = race;
    this->engine = new Engine();
    this->target = new Target(63);
    this->roll = new CombatRoll(this);
    this->faction = new Faction();
    this->talents = new Talents();
    this->cstats = new CharacterStats(this, equipment_db);
    this->active_procs = new ActiveProcs(this, faction);
    this->active_buffs = new ActiveBuffs(this, faction);
    this->statistics = nullptr;
    this->clvl = 1;
    this->melee_attacking = false;
    this->next_gcd = 0 - this->global_cooldown();
    this->ability_crit_dmg_mod = 2.0;
    this->spell_crit_dmg_mod = 1.5;
    this->current_rotation = nullptr;

    this->berserking_buff = new BerserkingBuff(this);
    this->blood_fury_buff = new BloodFuryBuff(this);
}

Character::~Character() {
    delete engine;
    delete target;
    delete roll;
    delete faction;
    delete talents;
    delete active_procs;
    delete active_buffs;
    delete berserking_buff;
    delete blood_fury_buff;
}

Race* Character::get_race() {
    return this->race;
}

bool Character::race_available(Race* race) const {
    return available_races.contains(race->get_name());
}

void Character::set_race(Race* race) {
    assert(race_available(race));

    remove_racial_effects();
    this->race = race;
    apply_racial_effects();
}

bool Character::set_rotation(Rotation* rotation) {
    if (!rotation->link_spells(this)) {
        qDebug() << "Failed to link spells for rotation" << rotation->get_name();
        return false;
    }

    current_rotation = rotation;
    qDebug() << "Successfully set rotation" << current_rotation->get_name();
    return true;
}

QString Character::get_current_rotation_name() const {
    return current_rotation != nullptr ? current_rotation->get_name() :
                                         "No rotation selected";
}

void Character::switch_faction() {
    active_procs->switch_faction();
    active_buffs->switch_faction();
}

void Character::perform_rotation() {
    if (current_rotation == nullptr)
        return;

    this->current_rotation->perform_rotation();
}

Rotation* Character::get_rotation() {
    return this->current_rotation;
}

void Character::change_target_creature_type(const QString &creature_type) {
    remove_racial_effects();
    target->set_creature_type(creature_type);
    apply_racial_effects();
}

void Character::apply_racial_effects() {
    switch (race->get_race_int()) {
    case Races::Orc:
        spells->get_blood_fury()->enable();
        break;
    case Races::Troll:
        spells->get_berserking()->enable();
        if (target->get_creature_type() == Target::CreatureType::Beast)
            cstats->increase_total_phys_dmg_mod(5);
        break;
    }
}

void Character::remove_racial_effects() {
    switch (race->get_race_int()) {
    case Races::Orc:
        spells->get_blood_fury()->disable();
        break;
    case Races::Troll:
        spells->get_berserking()->disable();
        if (target->get_creature_type() == Target::CreatureType::Beast)
            cstats->decrease_total_phys_dmg_mod(5);
        break;
    }
}

int Character::get_clvl() const {
    return this->clvl;
}

void Character::set_clvl(const int clvl) {
    this->clvl = clvl;
}

bool Character::is_dual_wielding() {
    return cstats->get_equipment()->is_dual_wielding();
}

bool Character::is_melee_attacking() const {
    return melee_attacking;
}

Engine* Character::get_engine() const {
    return this->engine;
}

Target* Character::get_target() const {
    return this->target;
}

CombatRoll* Character::get_combat_roll() const {
    return this->roll;
}

Faction* Character::get_faction() const {
    return this->faction;
}

Equipment* Character::get_equipment() const {
    return this->cstats->get_equipment();
}

Talents* Character::get_talents() const {
    return this->talents;
}

ActiveBuffs* Character::get_active_buffs() const {
    return this->active_buffs;
}

Spells* Character::get_spells() const {
    return this->spells;
}

CharacterStats* Character::get_stats() const {
    return this->cstats;
}

ClassStatistics* Character::get_statistics() const {
    return this->statistics;
}

ActiveProcs* Character::get_active_procs() const {
    return this->active_procs;
}

BerserkingBuff* Character::get_berserking_buff() const {
    return this->berserking_buff;
}

BloodFuryBuff* Character::get_blood_fury_buff() const {
    return this->blood_fury_buff;
}

void Character::add_player_reaction_event() {
    auto* new_event = new PlayerAction(current_rotation, engine->get_current_priority() + 0.1);
    engine->add_event(new_event);
}

void Character::start_attack() {
    this->melee_attacking = true;

    spells->start_attack();
}

void Character::stop_attack() {
    this->melee_attacking = false;
}

void Character::start_global_cooldown() {
    assert(action_ready());
    this->next_gcd = engine->get_current_priority() + global_cooldown();
}

double Character::global_cooldown() const {
    return 1.5;
}

bool Character::on_global_cooldown() const {
    return engine->get_current_priority() < this->next_gcd;
}

bool Character::action_ready() const {
    // Allow some rounding errors (this could effectively speed up gcd by 1/10000ths of a second).
    double delta = next_gcd - engine->get_current_priority();
    return delta < 0.0001;
}

void Character::melee_mh_white_hit_effect() {
    run_mh_white_specific_proc_effects();
}

void Character::melee_mh_yellow_hit_effect() {
    run_mh_yellow_specific_proc_effects();
}

void Character::melee_mh_white_critical_effect() {
    run_mh_white_specific_proc_effects();
}

void Character::melee_mh_yellow_critical_effect() {
    run_mh_yellow_specific_proc_effects();
}

void Character::melee_oh_white_hit_effect() {
    run_oh_white_specific_proc_effects();
}

void Character::melee_oh_yellow_hit_effect() {
    run_oh_yellow_specific_proc_effects();
}

void Character::melee_oh_white_critical_effect() {
    run_oh_white_specific_proc_effects();
}

void Character::melee_oh_yellow_critical_effect() {
    run_oh_yellow_specific_proc_effects();
}


void Character::spell_hit_effect() {

}

void Character::spell_critical_effect() {

}

void Character::run_mh_white_specific_proc_effects() {
    active_procs->run_proc_effects(ProcInfo::Source::MainhandSwing);
}

void Character::run_mh_yellow_specific_proc_effects() {
    active_procs->run_proc_effects(ProcInfo::Source::MainhandSpell);
}

void Character::run_oh_white_specific_proc_effects() {
    active_procs->run_proc_effects(ProcInfo::Source::OffhandSwing);
}

void Character::run_oh_yellow_specific_proc_effects() {
    active_procs->run_proc_effects(ProcInfo::Source::OffhandSpell);
}

void Character::run_extra_mh_attack() {
    spells->get_mh_attack()->extra_attack();
}

void Character::run_extra_oh_attack() {
    spells->get_oh_attack()->extra_attack();
}

double Character::get_ability_crit_dmg_mod() const {
    return ability_crit_dmg_mod;
}

double Character::get_spell_crit_dmg_mod() const {
    return spell_crit_dmg_mod;
}

double Character::get_random_normalized_mh_dmg() {
    Weapon* mh = cstats->get_equipment()->get_mainhand();
    return get_normalized_dmg(mh->get_random_dmg(), mh);
}

double Character::get_random_non_normalized_mh_dmg() {
    Weapon* mh = cstats->get_equipment()->get_mainhand();
    return get_non_normalized_dmg(mh->get_random_dmg(), mh->get_base_weapon_speed());
}

double Character::get_random_normalized_oh_dmg() {
    Weapon* oh = cstats->get_equipment()->get_offhand();
    return get_normalized_dmg(oh->get_random_dmg(), oh);
}

double Character::get_random_non_normalized_oh_dmg() {
    Weapon* oh = cstats->get_equipment()->get_offhand();
    return get_non_normalized_dmg(oh->get_random_dmg(), oh->get_base_weapon_speed());
}

unsigned Character::get_avg_mh_damage() {
    if (!has_mainhand())
        return static_cast<unsigned>(round(get_normalized_dmg(1, nullptr)));

    Weapon* mh = cstats->get_equipment()->get_mainhand();
    auto avg_dmg = static_cast<unsigned>(round(mh->get_min_dmg() + mh->get_max_dmg()) / 2);
    return static_cast<unsigned>(round(get_non_normalized_dmg(avg_dmg, mh->get_base_weapon_speed())));
}

double Character::get_normalized_dmg(const unsigned damage, const Weapon* weapon) {
    if (weapon == nullptr)
        return get_non_normalized_dmg(damage, 2.0);

    double normalized_wpn_speed = -1;

    switch (weapon->get_weapon_slot()) {
    case WeaponSlots::MAINHAND:
    case WeaponSlots::OFFHAND:
    case WeaponSlots::ONEHAND:
        normalized_wpn_speed = weapon->get_weapon_type() == WeaponTypes::DAGGER ? 1.7 : 2.4;
        break;
    case WeaponSlots::TWOHAND:
        normalized_wpn_speed = 3.3;
        break;
    case WeaponSlots::RANGED:
        normalized_wpn_speed = 2.8;
        break;
    default:
        assert(false);
    }

    return get_non_normalized_dmg(damage, normalized_wpn_speed);
}

double Character::get_non_normalized_dmg(const unsigned damage, const double wpn_speed) {
    return damage + (wpn_speed * cstats->get_melee_ap() / 14);
}

int Character::get_mh_wpn_skill() const {
    return cstats->get_mh_wpn_skill();
}

int Character::get_oh_wpn_skill() const {
    return cstats->get_oh_wpn_skill();
}

int Character::get_wpn_skill(Weapon* weapon) const {
    return cstats->get_wpn_skill(weapon);
}

void Character::increase_attack_speed(int increase) {
    cstats->increase_haste(increase);
    double increase_double = double(increase) / 100;

    spells->get_mh_attack()->update_next_expected_use(increase_double);
    spells->add_next_mh_attack();

    if (cstats->get_equipment()->is_dual_wielding()) {
        spells->get_oh_attack()->update_next_expected_use(increase_double);
        spells->add_next_oh_attack();
    }
}

void Character::decrease_attack_speed(int decrease) {
    cstats->decrease_haste(decrease);
    double decrease_double = double(decrease) / 100;

    spells->get_mh_attack()->update_next_expected_use(-decrease_double);
    spells->add_next_mh_attack();

    if (cstats->get_equipment()->is_dual_wielding()) {
        spells->get_oh_attack()->update_next_expected_use(-decrease_double);
        spells->add_next_oh_attack();
    }
}

void Character::increase_ability_crit_dmg_mod(double increase) {
    ability_crit_dmg_mod += increase;
}

void Character::decrease_ability_crit_dmg_mod(double decrease) {
    ability_crit_dmg_mod -= decrease;
}

void Character::increase_spell_crit_dmg_mod(double increase) {
    spell_crit_dmg_mod += increase;
}

void Character::decrease_spell_crit_dmg_mod(double decrease) {
    spell_crit_dmg_mod -= decrease;
}

bool Character::has_mainhand() const {
    return cstats->get_equipment()->get_mainhand() != nullptr;
}

bool Character::has_offhand() const {
    return cstats->get_equipment()->get_offhand() != nullptr;
}

unsigned Character::get_resource_level() const {
    return 0;
}

void Character::reset() {
    melee_attacking = false;
    next_gcd = 0 - this->global_cooldown();

    active_buffs->reset();
    reset_spells();
    active_procs->reset();

    reset_resource();
}

void Character::dump() {
    qDebug() << "--------------------------------------";
    qDebug() << get_name();
    qDebug() << "STRENGTH" << cstats->get_strength();
    qDebug() << "AGILITY" << cstats->get_agility();
    qDebug() << "STAMINA" << cstats->get_stamina();
    qDebug() << "INTELLECT" << cstats->get_intellect();
    qDebug() << "SPIRIT" << cstats->get_spirit();
    qDebug() << "MH Wpn skill" << cstats->get_mh_wpn_skill();
    qDebug() << "OH Wpn skill" << cstats->get_oh_wpn_skill();
    qDebug() << "MH DPS" << (cstats->get_equipment()->get_mainhand() != nullptr ? QString::number(cstats->get_equipment()->get_mainhand()->get_wpn_dps()) : "No MH");
    qDebug() << "OH DPS" << (cstats->get_equipment()->get_offhand() != nullptr ? QString::number(cstats->get_equipment()->get_offhand()->get_wpn_dps()) : "No OH");
    qDebug() << "Hit chance" << cstats->get_hit_chance();
    qDebug() << "Crit chance" << cstats->get_mh_crit_chance();
    qDebug() << "--------------------------------------";
}
