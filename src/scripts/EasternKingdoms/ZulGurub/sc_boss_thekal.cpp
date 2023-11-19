#include "Log.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "Smallcraft.h"
#include "TaskScheduler.h"
#include "../../src/server/scripts/EasternKingdoms/ZulGurub/zulgurub.h"

enum Says
{
    SAY_AGGRO                      = 0,
    SAY_DEATH                      = 1,
    EMOTE_DIES                     = 2,

    EMOTE_ZEALOT_DIES              = 0
};

enum Spells
{
    // Boss - pre-fight
    SPELL_SUMMONTIGERS                  = 24183,

    // Boss
    SPELL_CHARGE                        = 24193,
    SPELL_ENRAGE                        = 8269,
    SPELL_FORCEPUNCH                    = 24189,
    SPELL_FRENZY                        = 8269,
    SPELL_MORTALCLEAVE                  = 22859,
    SPELL_RESURRECTION_IMPACT_VISUAL    = 24171,
    SPELL_SILENCE                       = 22666,
    SPELL_TIGER_FORM                    = 24169,

    // Zealot Lor'Khan Spells
    SPELL_SHIELD                        = 20545,
    SPELL_BLOODLUST                     = 24185,
    SPELL_GREATERHEAL                   = 24208,
    SPELL_DISARM                        = 6713,

    // Zealot Zath Spells
    SPELL_SWEEPINGSTRIKES               = 18765,
    SPELL_SINISTERSTRIKE                = 15581,
    SPELL_GOUGE                         = 12540,
    SPELL_KICK                          = 15614,
    SPELL_BLIND                         = 21060
};

enum Actions
{
    ACTION_RESSURRECT         = 1
};

// High-Priest Thekal (14509)
struct boss_thekal : public BossAI
{
    boss_thekal(Creature* creature) : BossAI(creature, DATA_THEKAL)
    {
        Initialize();
    }

    void Initialize()
    {
        _enraged = false;
        _wasDead = false;
        _lorkhanDied = false;
        _zathDied = false;
    }

    void Reset() override
    {
        _Reset();
        Initialize();

        scheduler.CancelAll();

        me->SetStandState(UNIT_STAND_STATE_STAND);
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveAurasDueToSpell(SPELL_FRENZY);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->LoadEquipment(1, true);

        if (Creature* zealot = instance->GetCreature(DATA_LORKHAN))
        {
            zealot->AI()->Reset();
        }

        if (Creature* zealot = instance->GetCreature(DATA_ZATH))
        {
            zealot->AI()->Reset();
        }

        // emote idle loop
        scheduler.Schedule(5s, 25s, [this](TaskContext context)
        {
            // pick a random emote from the list of available emotes
            me->HandleEmoteCommand(
                RAND(
                    EMOTE_ONESHOT_TALK,
                    EMOTE_ONESHOT_FLEX,
                    EMOTE_ONESHOT_POINT
                )
            );
            context.Repeat(5s, 25s);
        });

        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });
    }

    void JustDied(Unit* /*killer*/) override
    {
        _JustDied();
        Talk(SAY_DEATH);

        if (Creature* zealot = instance->GetCreature(DATA_LORKHAN))
        {
            zealot->Kill(zealot, zealot);
        }

        if (Creature* zealot = instance->GetCreature(DATA_ZATH))
        {
            zealot->Kill(zealot, zealot);
        }
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _JustEngagedWith();

        scheduler.CancelAll();
        scheduler.Schedule(4s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_MORTALCLEAVE);
            context.Repeat(15s, 20s);
        }).Schedule(9s, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SILENCE);
            context.Repeat(20s, 25s);
        }).Schedule(16s, [this](TaskContext context)
        {
            DoCastSelf(SPELL_BLOODLUST);
            context.Repeat(20s, 28s);
        });
    }

    void SetData(uint32 /*type*/, uint32 data) override
    {
        UpdateZealotStatus(data, true);
        CheckPhaseTransition();

        scheduler.Schedule(10s, [this, data](TaskContext /*context*/)
        {
            if (!_lorkhanDied || !_zathDied || !_wasDead)
            {
                ReviveZealot(data);
            }
        });
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageEffectType, SpellSchoolMask spellSchoolMask) override
    {
        if (!me->HasAura(SPELL_TIGER_FORM) && damage >= me->GetHealth())
        {
            damage = me->GetHealth() - 1;

            if (!_wasDead)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_PASSIVE);
                me->SetStandState(UNIT_STAND_STATE_DEAD);
                me->AttackStop();
                DoResetThreatList();
                _wasDead = true;
                CheckPhaseTransition();
                Talk(EMOTE_DIES);
            }
        }

        BossAI::DamageTaken(attacker, damage, damageEffectType, spellSchoolMask);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_RESSURRECT)
        {
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
            me->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
            me->RestoreFaction();
            me->SetReactState(REACT_AGGRESSIVE);
            me->SetFullHealth();
            _wasDead = false;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->IsInCombat() && !UpdateVictim())
        {
            return;
        }
        else if (me->IsInCombat())
        {
            scheduler.Update(diff,
            std::bind(&BossAI::DoMeleeAttackIfReady, this));
        }
        else
        {
            scheduler.Update(diff);
        }
    }

    void ReviveZealot(uint32 zealotData)
    {
        if (Creature* zealot = instance->GetCreature(zealotData))
        {
            zealot->Respawn(true);
            zealot->SetInCombatWithZone();
            UpdateZealotStatus(zealotData, false);
        }
    }

    void UpdateZealotStatus(uint32 data, bool dead)
    {
        if (data == DATA_LORKHAN)
        {
            _lorkhanDied = dead;
        }
        else if (data == DATA_ZATH)
        {
            _zathDied = dead;
        }
    }

    void CheckPhaseTransition()
    {
        if (_wasDead && _lorkhanDied && _zathDied)
        {
            scheduler.Schedule(3s, [this](TaskContext /*context*/)
            {
                me->SetStandState(UNIT_STAND_STATE_STAND);
                DoCastSelf(SPELL_RESURRECTION_IMPACT_VISUAL, true);

                scheduler.Schedule(50ms, [this](TaskContext /*context*/)
                {
                    Talk(SAY_AGGRO);
                });

                scheduler.Schedule(6s, [this](TaskContext /*context*/)
                {
                    DoCastSelf(SPELL_TIGER_FORM);
                    me->LoadEquipment(0, true);
                    me->SetFullHealth();
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);

                    scheduler.Schedule(30s, [this](TaskContext context)
                    {
                        DoCastSelf(SPELL_FRENZY);
                        context.Repeat();
                    }).Schedule(4s, [this](TaskContext context)
                    {
                        DoCastVictim(SPELL_FORCEPUNCH);
                        context.Repeat(16s, 21s);
                    }).Schedule(12s, [this](TaskContext context)
                    {
                        // charge a random target that is at least 8 yards away (min range of charge is 8 yards)
                        if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, -8.0f))
                        {
                            DoCast(target, SPELL_CHARGE);
                            DoResetThreatList();
                            AttackStart(target);
                        }
                        context.Repeat(15s, 22s);
                    }).Schedule(25s, 35s, [this](TaskContext context)
                    {
                        DoCastVictim(SPELL_SUMMONTIGERS, true);
                        sc::Talk(me, "Thekal summons tigers to his side!", CHAT_MSG_RAID_BOSS_EMOTE);
                        context.Repeat(25s, 35s);
                    });

                    // schedule Enrage at 20% health
                    ScheduleHealthCheckEvent(20, [this]
                    {
                        DoCastSelf(SPELL_ENRAGE);
                    });
                });
            });
        }
        else
        {
            scheduler.Schedule(10s, [this](TaskContext /*context*/)
            {
                if (!(_wasDead && _lorkhanDied && _zathDied))
                {
                    DoAction(ACTION_RESSURRECT);
                }
            });
        }
    }

    private:
        bool _lorkhanDied;
        bool _zathDied;
        bool _enraged;
        bool _wasDead;
};

class sc_boss_thekal_DatabaseScript : public DatabaseScript
{
public:
    sc_boss_thekal_DatabaseScript() : DatabaseScript("sc_boss_thekal_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        // Zealot Zath (11348) - Tiger Boss Add
        // make kite-able
        creatureTemplates[11348]->MechanicImmuneMask = 536936977; // can't be CC'd, but can be slowed/distracted/rooted/etc
    }
};

void load_sc_boss_thekal()
{
    LOG_DEBUG("module.Smallcraft", "SmallCraft: Vanilla/Zul'Gurub/High Priest Thekal is enabled.");

    // High Priest Thekal (14509) - Tiger Boss
    RegisterCreatureAI(boss_thekal);

    new sc_boss_thekal_DatabaseScript();
}