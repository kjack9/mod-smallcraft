#include "GameObjectAI.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Smallcraft.h"
#include "SmartAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"

namespace sc
{
    enum Says
{
    SAY_AGGRO                   = 0,
    SAY_FEAST_PROWLER           = 1,
    SAY_DEATH                   = 2
};

enum Spells
{
    SPELL_SHADOW_WORD_PAIN      = 24212, // Corrected
    SPELL_GOUGE                 = 12540, // Corrected
    SPELL_MARK_OF_ARLOKK        = 24210, // triggered spell 24211 Added to spell_dbc
    SPELL_RAVAGE                = 24213, // Corrected
    SPELL_CLEAVE                = 25174, // Searching for right spell
    SPELL_PANTHER_TRANSFORM     = 24190, // Transform to panther now used
    SPELL_SUMMON_PROWLER        = 24246, // Added to Spell_dbc
    SPELL_VANISH_VISUAL         = 24222, // Added
    SPELL_VANISH                = 24223, // Added
    SPELL_SUPER_INVIS           = 24235  // Added to Spell_dbc
};

enum Events
{
    EVENT_SHADOW_WORD_PAIN      = 1,
    EVENT_GOUGE                 = 2,
    EVENT_MARK_OF_ARLOKK        = 3,
    EVENT_RAVAGE                = 4,
    EVENT_TRANSFORM             = 5,
    EVENT_VANISH                = 6,
    EVENT_VANISH_2              = 7,
    EVENT_TRANSFORM_BACK        = 8,
    EVENT_VISIBLE               = 9,
    EVENT_SUMMON_PROWLERS       = 10,
    EVENT_ADD_IMMUNITIES        = 11
};

enum Phases
{
    PHASE_ALL                   = 0,
    PHASE_ONE                   = 1,
    PHASE_TWO                   = 2
};

enum Weapon
{
    WEAPON_DAGGER               = 10616
};

enum Misc
{
    MAX_PROWLERS_PER_SIDE       = 15
};

Position const PosMoveOnSpawn[1] =
{
    { -11561.9f, -1627.868f, 41.29941f, 0.0f }
};

struct boss_arlokk : public BossAI
{
    boss_arlokk(Creature* creature) : BossAI(creature, DATA_ARLOKK) { }

    void InitializeAI() override
    {
        LOG_DEBUG("module.SmallCraft.ai", "SmallCraft:boss_arlokk:InitializeAI");

        BossAI::InitializeAI();

        _addImmunities();
    }

    void Reset() override
    {
        if (events.IsInPhase(PHASE_TWO))
            me->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, 35.0f, false); // hack
        _Reset();
        _summonCountA = 0;
        _summonCountB = 0;
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(WEAPON_DAGGER));
        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(WEAPON_DAGGER));
        me->SetWalk(false);
        me->SetHomePosition(PosMoveOnSpawn[0]);
        me->GetMotionMaster()->MoveTargetedHome();

        _addImmunities();

    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);
    }

    void JustEngagedWith(Unit* who) override
    {
        LOG_DEBUG("module.SmallCraft.ai", "SmallCraft:boss_arlokk:JustEngagedWith {}", who->GetName());

        _JustEngagedWith();

        events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 7s, 9s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_GOUGE, 12s, 15s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_SUMMON_PROWLERS, 7s, 0, PHASE_ALL);
        events.ScheduleEvent(EVENT_MARK_OF_ARLOKK, 9s, 11s, 0, PHASE_ALL);
        events.ScheduleEvent(EVENT_TRANSFORM, 30s, 0, PHASE_ONE);
        Talk(SAY_AGGRO);

        // Sets up list of Panther spawners to cast on
        std::list<Creature*> triggerList;
        GetCreatureListWithEntryInGrid(triggerList, me, NPC_PANTHER_TRIGGER, 100.0f);
        if (!triggerList.empty())
        {
            uint8 sideA = 0;
            uint8 sideB = 0;
            for (auto const& trigger : triggerList)
            {
                if (trigger->GetPositionY() < -1625.0f)
                {
                    _triggersSideAGUID[sideA] = trigger->GetGUID();
                    ++sideA;
                }
                else
                {
                    _triggersSideBGUID[sideB] = trigger->GetGUID();
                    ++sideB;
                }
            }
        }
    }

    void JustReachedHome() override
    {
        if (GameObject* object = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(GO_GONG_OF_BETHEKK)))
            object->RemoveGameObjectFlag(GO_FLAG_NOT_SELECTABLE);
        me->DespawnOrUnsummon();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        BossAI::EnterEvadeMode(why);

        std::list<Creature*> panthers;
        GetCreatureListWithEntryInGrid(panthers, me, NPC_ZULIAN_PROWLER, 200.f);
        for (auto const& panther : panthers)
            panther->DespawnOrUnsummon();
    }

    void SetData(uint32 id, uint32 /*value*/) override
    {
        if (id == 1)
            --_summonCountA;
        else if (id == 2)
            --_summonCountB;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SHADOW_WORD_PAIN:
                    DoCastVictim(SPELL_SHADOW_WORD_PAIN, true);
                    events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 5s, 7s, 0, PHASE_ONE);
                    break;
                case EVENT_GOUGE:
                    DoCastVictim(SPELL_GOUGE, true);
                    _removeImmunities(); // remove immunities for 4 seconds
                    if (me->GetVictim())
                    {
                        sc::Talk(me, "Arlokk is distracted, stop her!", CHAT_MSG_RAID_BOSS_EMOTE, 1000.0f);
                    }
                    events.ScheduleEvent(EVENT_ADD_IMMUNITIES, 4s);
                    break;
                case EVENT_ADD_IMMUNITIES:
                    _addImmunities();
                    break;
                case EVENT_SUMMON_PROWLERS:
                    if (_summonCountA < MAX_PROWLERS_PER_SIDE)
                    {
                        if (Unit* trigger = ObjectAccessor::GetUnit(*me, _triggersSideAGUID[urand(0, 4)]))
                        {
                            trigger->CastSpell(trigger, SPELL_SUMMON_PROWLER);
                            ++_summonCountA;
                        }
                    }
                    if (_summonCountB < MAX_PROWLERS_PER_SIDE)
                    {
                        if (Unit* trigger = ObjectAccessor::GetUnit(*me, _triggersSideBGUID[urand(0, 4)]))
                        {
                            trigger->CastSpell(trigger, SPELL_SUMMON_PROWLER);
                            ++_summonCountB;
                        }
                    }
                    events.ScheduleEvent(EVENT_SUMMON_PROWLERS, 7s, 0, PHASE_ALL);
                    break;
                case EVENT_MARK_OF_ARLOKK:
                    {
                        Unit* target = SelectTarget(SelectTargetMethod::MinDistance, 0, 0.0f, false, false, -SPELL_MARK_OF_ARLOKK);
                        if (!target)
                            target = me->GetVictim();
                        if (target)
                        {
                            DoCast(target, SPELL_MARK_OF_ARLOKK, true);
                            Talk(SAY_FEAST_PROWLER, target);
                        }
                        events.ScheduleEvent(EVENT_MARK_OF_ARLOKK, 120s, 130s);
                        break;
                    }
                case EVENT_TRANSFORM:
                    {
                        DoCastSelf(SPELL_PANTHER_TRANSFORM); // SPELL_AURA_TRANSFORM
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(EQUIP_UNEQUIP));
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(EQUIP_UNEQUIP));
                        me->AttackStop();
                        DoResetThreatList();
                        me->SetReactState(REACT_PASSIVE);
                        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        DoCastSelf(SPELL_VANISH_VISUAL);
                        DoCastSelf(SPELL_VANISH);
                        events.ScheduleEvent(EVENT_VANISH, 1s, 0, PHASE_ONE);
                        break;
                    }
                case EVENT_VANISH:
                    DoCastSelf(SPELL_SUPER_INVIS);
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(0, frand(-11551.0f, -11508.0f), frand(-1638.0f, -1617.0f), me->GetPositionZ());
                    events.ScheduleEvent(EVENT_VANISH_2, 9s, 0, PHASE_ONE);
                    break;
                case EVENT_VANISH_2:
                    DoCastSelf(SPELL_VANISH);
                    DoCastSelf(SPELL_SUPER_INVIS);
                    events.ScheduleEvent(EVENT_VISIBLE, 41s, 47s, 0, PHASE_ONE);
                    break;
                case EVENT_VISIBLE:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveUnitFlag(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
                        AttackStart(target);
                    me->RemoveAura(SPELL_SUPER_INVIS);
                    me->RemoveAura(SPELL_VANISH);
                    events.ScheduleEvent(EVENT_RAVAGE, 10s, 14s, 0, PHASE_TWO);
                    events.ScheduleEvent(EVENT_TRANSFORM_BACK, 30s, 40s, 0, PHASE_TWO);
                    events.SetPhase(PHASE_TWO);
                    me->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, 35.0f, true); // hack
                    break;
                case EVENT_RAVAGE:
                    DoCastVictim(SPELL_RAVAGE, true);
                    events.ScheduleEvent(EVENT_RAVAGE, 10s, 14s, 0, PHASE_TWO);
                    break;
                case EVENT_TRANSFORM_BACK:
                    {
                        me->RemoveAura(SPELL_PANTHER_TRANSFORM); // SPELL_AURA_TRANSFORM
                        DoCast(me, SPELL_VANISH_VISUAL);
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 0, uint32(WEAPON_DAGGER));
                        me->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_ID + 1, uint32(WEAPON_DAGGER));
                        me->HandleStatModifier(UNIT_MOD_DAMAGE_MAINHAND, TOTAL_PCT, 35.0f, false); // hack
                        events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, 4s, 7s, 0, PHASE_ONE);
                        events.ScheduleEvent(EVENT_GOUGE, 12s, 15s, 0, PHASE_ONE);
                        events.ScheduleEvent(EVENT_TRANSFORM, 30s, 0, PHASE_ONE);
                        events.SetPhase(PHASE_ONE);
                        break;
                    }
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    void _addImmunities()
    {
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, true);
    }

    void _removeImmunities()
    {
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISTRACT, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SNARE, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, false);
        me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, false);
    }

    uint8 _summonCountA;
    uint8 _summonCountB;
    ObjectGuid _triggersSideAGUID[5];
    ObjectGuid _triggersSideBGUID[5];
};

/*######
## npc_zulian_prowler
######*/

enum ZulianProwlerSpells
{
    SPELL_SNEAK_RANK_1_1         = 22766,
    SPELL_SNEAK_RANK_1_2         = 7939,  // Added to Spell_dbc
    SPELL_MARK_OF_ARLOKK_TRIGGER = 24211  // Added to Spell_dbc
};

enum ZulianProwlerEvents
{
    EVENT_ATTACK                 = 1
};

struct npc_zulian_prowler : public ScriptedAI
{
    npc_zulian_prowler(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()) { }

    void Reset() override
    {
        if (me->GetPositionY() < -1625.0f)
            _sideData = 1;
        else
            _sideData = 2;

        DoCast(me, SPELL_SNEAK_RANK_1_1);
        DoCast(me, SPELL_SNEAK_RANK_1_2);

        if (Unit* arlokk = ObjectAccessor::GetUnit(*me, _instance->GetGuidData(NPC_ARLOKK)))
            me->GetMotionMaster()->MovePoint(0, arlokk->GetPositionX(), arlokk->GetPositionY(), arlokk->GetPositionZ());
        _events.ScheduleEvent(EVENT_ATTACK, 6000);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->GetMotionMaster()->Clear(false);
        me->RemoveAura(SPELL_SNEAK_RANK_1_1);
        me->RemoveAura(SPELL_SNEAK_RANK_1_2);
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_MARK_OF_ARLOKK_TRIGGER) // Should only hit if line of sight
        {
            AttackStart(caster);
            me->GetThreatMgr().ClearAllThreat();
            me->GetThreatMgr().AddThreat(caster, 100.0f);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Unit* arlokk = ObjectAccessor::GetUnit(*me, _instance->GetGuidData(NPC_ARLOKK)))
        {
            if (arlokk->IsAlive())
                arlokk->GetAI()->SetData(_sideData, 0);
        }
        me->DespawnOrUnsummon(4000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim())
        {
            DoMeleeAttackIfReady();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ATTACK:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0.0f, 100, false))
                    {
                        AttackStart(target);
                    }
                    break;
                default:
                    break;
            }
        }
    }

private:
    int32 _sideData;
    EventMap _events;
    InstanceScript* _instance;
};

/*######
## go_gong_of_bethekk
######*/

Position const PosSummonArlokk[1] =
{
    { -11507.22f, -1628.062f, 41.38264f, 3.159046f }
};

class go_gong_of_bethekk : public GameObjectScript
{
public:
    go_gong_of_bethekk() : GameObjectScript("go_gong_of_bethekk") { }

    bool OnGossipHello(Player* /*player*/, GameObject* go) override
    {
        if (go->GetInstanceScript() && !go->FindNearestCreature(NPC_ARLOKK, 25.0f))
        {
            go->SetGameObjectFlag(GO_FLAG_NOT_SELECTABLE);
            go->SendCustomAnim(0);
            go->SummonCreature(NPC_ARLOKK, PosSummonArlokk[0], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 600000);
        }

        return true;
    }
};

class boss_arlokk_GlobalScript : public GlobalScript
{
public:
    boss_arlokk_GlobalScript() : GlobalScript("boss_arlok_GlobalScript") { }

    void OnLoadSpellCustomAttr(SpellInfo* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            // Mark of Arlokk (24210)
            // Pulse every 8 seconds
            case (SPELL_MARK_OF_ARLOKK):
                LOG_DEBUG("module.SmallCraft", "sc::boss_arlok_GlobalScript:: Modifying spell {} ({})...", spellInfo->Id, spellInfo->SpellName[0]);
                spellInfo->Effects[0].Amplitude = 8000;
                break;
            default:
                break;
        }
    }
};

class boss_arlokk_DatabaseScript : public DatabaseScript
{
public:
    boss_arlokk_DatabaseScript() : DatabaseScript("boss_arlokk_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        // High Priestess Arlokk (14515) - Panther Boss
        creatureTemplates[14515]->MechanicImmuneMask = 550183697; // can't be CC'd, but can be disorient/distract/root/snare/stun/freeze/daze
                                                                  // these other effects will be enabled/disabled by the boss script
    }
};


void load_boss_arlokk()
{
    RegisterCreatureAI(boss_arlokk);
    RegisterCreatureAI(npc_zulian_prowler);

    new go_gong_of_bethekk();

    new boss_arlokk_GlobalScript();
    new boss_arlokk_DatabaseScript();
}
} // namespace sc