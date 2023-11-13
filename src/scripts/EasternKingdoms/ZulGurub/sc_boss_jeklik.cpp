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

enum Says
{
    // Jeklik
    SAY_AGGRO                           = 0,
    SAY_CALL_RIDERS                     = 1,
    SAY_DEATH                           = 2,
    EMOTE_SUMMON_BATS                   = 3,
    EMOTE_GREAT_HEAL                    = 4,

    // Bat Rider
    EMOTE_BATRIDER_LOW_HEALTH            = 0
};

enum Spells
{
    // Intro
    SPELL_GREEN_CHANNELING              = 13540,
    SPELL_BAT_FORM                      = 23966,

    // Phase one
    SPELL_PIERCE_ARMOR                  = 12097,
    SPELL_BLOOD_LEECH                   = 22644,
    SPELL_CHARGE                        = 22911,
    SPELL_SONIC_BURST                   = 23918,
    SPELL_SWOOP                         = 23919,

    // Phase two
    SPELL_CURSE_OF_BLOOD                = 16098,
    SPELL_PSYCHIC_SCREAM                = 22884,
    SPELL_SHADOW_WORD_PAIN              = 23952,
    SPELL_MIND_FLAY                     = 23953,
    SPELL_GREATER_HEAL                  = 23954,

    // Bat Rider (Boss)
    SPELL_BATRIDER_THROW_LIQUID_FIRE    = 23970,
    SPELL_BATRIDER_SUMMON_LIQUID_FIRE   = 23971,

    // Bat Rider (Trash)
    SPELL_BATRIDER_DEMO_SHOUT           = 23511,
    SPELL_BATRIDER_BATTLE_COMMAND       = 5115,
    SPELL_BATRIDER_INFECTED_BITE        = 16128,
    SPELL_BATRIDER_PASSIVE_THRASH       = 8876,
    SPELL_BATRIDER_UNSTABLE_CONCOCTION  = 24024
};

enum BatIds
{
    NPC_BLOODSEEKER_BAT                 = 11368,
    NPC_BATRIDER                        = 14750
};

enum Phase
{
    PHASE_ONE                           = 1,
    PHASE_TWO                           = 2
};

Position const SpawnBat[6] =
{
    { -12291.6220f, -1380.2640f, 144.8304f, 5.483f },
    { -12289.6220f, -1380.2640f, 144.8304f, 5.483f },
    { -12293.6220f, -1380.2640f, 144.8304f, 5.483f },
    { -12291.6220f, -1380.2640f, 144.8304f, 5.483f },
    { -12289.6220f, -1380.2640f, 144.8304f, 5.483f },
    { -12293.6220f, -1380.2640f, 144.8304f, 5.483f }
};

Position const SpawnBatRider = { -12301.689, -1371.2921, 145.09244 };
Position const JeklikCaveHomePosition = { -12291.9f, -1380.08f, 144.902f, 2.28638f };

enum PathID
{
    PATH_JEKLIK_INTRO                   = 145170,
    PATH_BATRIDER_LOOP                  = 147500
};

enum BatRiderMode
{
    BATRIDER_MODE_TRASH                 = 1,
    BATRIDER_MODE_BOSS
};

/**
 * @brief SmallCraft AI for the High Priestess Jeklik (14517) creature.
 */
struct sc_boss_jeklik : public BossAI
{
    sc_boss_jeklik(Creature* creature) : BossAI(creature, DATA_JEKLIK) { }

    void InitializeAI() override
    {
        BossAI::InitializeAI();
    }

    void Reset() override
    {
        BossAI::Reset();

        me->SetHomePosition(JeklikCaveHomePosition);

        me->SetDisableGravity(false);
        me->SetReactState(REACT_PASSIVE);
        BossAI::SetCombatMovement(false);

        DoCastSelf(SPELL_GREEN_CHANNELING, true);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);

        Talk(SAY_AGGRO);
        DoZoneInCombat();

        me->RemoveAurasDueToSpell(SPELL_GREEN_CHANNELING);
        me->SetDisableGravity(true);
        DoCastSelf(SPELL_BAT_FORM, true);

        me->GetMotionMaster()->MovePath(PATH_JEKLIK_INTRO, false);
    }

    void PathEndReached(uint32 pathId) override
    {
        BossAI::PathEndReached(pathId);

        me->SetDisableGravity(false);
        SetCombatMovement(true);
        me->SetReactState(REACT_AGGRESSIVE);

        //
        // Phase 1
        //
        LOG_DEBUG("module.Smallcraft.ai", "sc_boss_jeklik:: PHASE ONE");
        // Charge
        scheduler.Schedule(10s, 20s, PHASE_ONE, [this](TaskContext context)
        {
            // charge the nearest player that is at least 8 yards away (charge min distance)
            if (Unit* target = SelectTarget(SelectTargetMethod::MinDistance, 0, -8.0f, false, false))
            {
                LOG_DEBUG("module.Smallcraft.ai", "sc_boss_jeklik::UpdateAI:: Charge successful (target: {})", target->GetName());
                DoCast(target, SPELL_CHARGE);
                AttackStart(target);
            }
            else
            {
                LOG_DEBUG("module.Smallcraft.ai", "sc_boss_jeklik::UpdateAI:: Charge failed (no target available)");
            }
            context.Repeat(15s, 30s);
        // Pierce Armor
        }).Schedule(5s, 15s, PHASE_ONE, [this](TaskContext context)
        {
            DoCastVictim(SPELL_PIERCE_ARMOR);
            context.Repeat(20s, 30s);
        // Blood Leech
        }).Schedule(5s, 15s, PHASE_ONE, [this](TaskContext context)
        {
            DoCastVictim(SPELL_BLOOD_LEECH);
            context.Repeat(10s, 20s);
        // Sonic Burst
        }).Schedule(5s, 15s, PHASE_ONE, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SONIC_BURST);
            context.Repeat(20s, 30s);
        // Swoop
        }).Schedule(20s, PHASE_ONE, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SWOOP);
            context.Repeat(20s, 30s);
        // Spawn Cave Bats
        }).Schedule(30s, PHASE_ONE, [this](TaskContext context)
        {
            Talk(EMOTE_SUMMON_BATS);
            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0))
            {
                for (uint8 i = 0; i < 6; ++i)
                {
                    if (Creature* bat = me->SummonCreature(NPC_BLOODSEEKER_BAT, SpawnBat[i], TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 15000))
                    {
                        bat->AI()->AttackStart(target);
                    }
                }
            }
            context.Repeat(30s);
        });

        //
        // Phase 2 (@ 50% health)
        //
        ScheduleHealthCheckEvent(50, [&]
        {
            LOG_DEBUG("module.Smallcraft.ai", "sc_boss_jeklik:: PHASE TWO");
            me->RemoveAurasDueToSpell(SPELL_BAT_FORM);
            DoResetThreatList();

            scheduler.CancelGroup(PHASE_ONE);

            // Curse of Blood
            scheduler.Schedule(5s, 15s, PHASE_TWO, [this](TaskContext context)
            {
                DoCastSelf(SPELL_CURSE_OF_BLOOD);
                context.Repeat(25s, 30s);
            // Psychic Scream
            }).Schedule(25s, 35s, PHASE_TWO, [this](TaskContext context)
            {
                DoCastVictim(SPELL_PSYCHIC_SCREAM);
                context.Repeat(35s, 45s);
            // Shadow Word: Pain
            }).Schedule(10s, 15s, PHASE_TWO, [this](TaskContext context)
            {
                DoCastRandomTarget(SPELL_SHADOW_WORD_PAIN, 0, true);
                context.Repeat(12s, 18s);
            // Mind Flay
            }).Schedule(10s, 30s, PHASE_TWO, [this](TaskContext context)
            {
                DoCastVictim(SPELL_MIND_FLAY);
                context.Repeat(20s, 40s);
            // Greater Heal
            }).Schedule(25s, PHASE_TWO, [this](TaskContext context)
            {
                Talk(EMOTE_GREAT_HEAL);
                me->InterruptNonMeleeSpells(false);
                DoCastSelf(SPELL_GREATER_HEAL);
                context.Repeat(25s);
            // Spawn Flying Bat
            }).Schedule(8s, PHASE_TWO, [this](TaskContext /*context*/)
            {
                if (me->GetThreatMgr().GetThreatListSize())
                {
                    LOG_DEBUG("module.Smallcraft.ai", "boss_jeklik:: Spawn Flying Bat");
                    Talk(SAY_CALL_RIDERS);
                    me->SummonCreature(NPC_BATRIDER, SpawnBatRider, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT);
                }
            });

        });
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (why != EvadeReason::EVADE_REASON_NO_PATH)
        {
            me->DespawnOnEvade(5s);
        }

        BossAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        BossAI::JustDied(killer);
        Talk(SAY_DEATH);
    }
};

/**
 * @brief SmallCraft AI for the Gurubashi Bat Rider (14750) creature.
 *        The same creature ID is used for both Jeklik and the trash around her.
 */
struct sc_npc_batrider : public CreatureAI
{
    BatRiderMode _mode; // the version of this creature (trash or boss)
    TaskScheduler _scheduler;
    Unit* _lastTarget = nullptr;
    Unit* _nextTarget = nullptr;

    sc_npc_batrider(Creature* creature) : CreatureAI(creature) { }

    void InitializeAI() override
    {
        CreatureAI::InitializeAI();

        // if this is a summon of Jeklik, it is in boss mode
        if
        (
            me->GetEntry() == NPC_BATRIDER &&
            me->IsSummon() &&
            me->ToTempSummon() &&
            me->ToTempSummon()->GetSummoner() &&
            me->ToTempSummon()->GetSummoner()->GetEntry() == NPC_PRIESTESS_JEKLIK
        )
        {
            LOG_DEBUG("module.Smallcraft.ai", "sc_npc_batrider::InitializeAI: BATRIDER_MODE_BOSS");
            _mode = BATRIDER_MODE_BOSS;

            // make the bat rider unattackable
            me->SetUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
            me->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
            me->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);

            // keep the bat from attacking players directly
            me->SetReactState(REACT_PASSIVE);

            // make the bat rider move the correct speed
            me->SetSpeed(MOVE_WALK, 5.0f, true);

            // start the flight loop
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->GetMotionMaster()->MoveSplinePath(PATH_BATRIDER_LOOP);

            // throw bomb
            _scheduler.Schedule(2s, [this](TaskContext context)
            {
                // If there is more than one threat target, don't throw the bomb at the same target twice in a row
                do
                {
                    _nextTarget = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);  // pick a new random target
                }
                while
                (
                    _lastTarget &&                                          // _lastTarget has been set
                    me->GetThreatMgr().GetThreatListSize() > 1 &&           // there is more than one threat target
                    _nextTarget->GetGUID() == _lastTarget->GetGUID()        // the next target is the same as the last target
                );

                if (_nextTarget)
                {
                    LOG_DEBUG("module.Smallcraft.ai", "sc_npc_batrider:: throwing bomb at {}", _nextTarget->GetName());
                    DoCast(_nextTarget, SPELL_BATRIDER_THROW_LIQUID_FIRE);
                    _lastTarget = _nextTarget;                              // save the next target as the last target
                }

                context.Repeat(4s, 6s);
            });
        }
        // otherwise, trash mode
        else
        {
            LOG_DEBUG("module.Smallcraft.ai", "sc_npc_batrider::InitializeAI: BATRIDER_MODE_TRASH");
            _mode = BATRIDER_MODE_TRASH;

            me->SetReactState(REACT_DEFENSIVE);

            // don't interrupt casting
            _scheduler.SetValidator([this]
            {
                return !me->HasUnitState(UNIT_STATE_CASTING);
            });
        }
    }

    void Reset() override
    {
        CreatureAI::Reset();

        _scheduler.CancelAll();

        if (_mode == BATRIDER_MODE_BOSS)
        {
            me->GetMotionMaster()->Clear();
        }
        else if (_mode == BATRIDER_MODE_TRASH)
        {
            // apply the Thrash (8876) spell to the bat rider (passive ability)
            me->CastSpell(me, SPELL_BATRIDER_PASSIVE_THRASH);
        }
    }

    void JustEngagedWith(Unit* who) override
    {
        CreatureAI::JustEngagedWith(who);

        if (_mode == BATRIDER_MODE_TRASH)
        {
            // Demo Shout
            _scheduler.Schedule(1s, [this](TaskContext /*context*/)
            {
                DoCastSelf(SPELL_BATRIDER_DEMO_SHOUT);
            // Battle Command
            }).Schedule(8s, [this](TaskContext context)
            {
                DoCastSelf(SPELL_BATRIDER_BATTLE_COMMAND);
                context.Repeat(25s);
            // Infected Bite
            }).Schedule(6500ms, [this](TaskContext context)
            {
                DoCastVictim(SPELL_BATRIDER_INFECTED_BITE);
                context.Repeat(8s);
            });
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType, SpellSchoolMask) override
    {
        if (_mode == BATRIDER_MODE_TRASH)
        {
            // if health goes below 30%, cast unstable concoction
            if (me->HealthBelowPctDamaged(30, damage))
            {
                _scheduler.CancelAll();
                DoCastSelf(SPELL_BATRIDER_UNSTABLE_CONCOCTION);
            }
        }
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (_mode == BATRIDER_MODE_BOSS)
        {
            // if the creature isn't moving, run the loop
            if (!me->isMoving())
            {
                LOG_DEBUG("module.Smallcraft.ai", "sc_npc_batrider::UpdateAI: not moving, running loop");
                // enable flying
                me->SetCanFly(true);
                // send the rider on its loop
                me->GetMotionMaster()->MoveSplinePath(PATH_BATRIDER_LOOP);
            }
        }
        else if (_mode == BATRIDER_MODE_TRASH)
        {
            if (!UpdateVictim())
            {
                return;
            }

            DoMeleeAttackIfReady();
        }

        _scheduler.Update();
    }
};

class sc_boss_jeklik_DatabaseScript : public DatabaseScript
{
public:
    sc_boss_jeklik_DatabaseScript() : DatabaseScript("sc_boss_jeklik_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        // Alter the AIs of creatures to use the SmallCraft AI without updating the DB
        // High Priestess Jeklik (14517)
        creatureTemplates[14517]->ScriptID = sObjectMgr->GetScriptId("sc_boss_jeklik");

        // Gurubashi Bat Rider (14750)
        creatureTemplates[14750]->ScriptID = sObjectMgr->GetScriptId("sc_npc_batrider");
    }

};

void load_sc_boss_jeklik()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Zul'Gurub/Jeklik is enabled.");

    // High Priestess Jeklik (14517)
    RegisterSmallcraftCreatureAI(sc_boss_jeklik);

    // Gurubashi Bat Rider (14750)
    RegisterSmallcraftCreatureAI(sc_npc_batrider);

    new sc_boss_jeklik_DatabaseScript();
}

