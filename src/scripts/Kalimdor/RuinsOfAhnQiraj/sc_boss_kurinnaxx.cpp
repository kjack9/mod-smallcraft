#include "GameObjectAI.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "Smallcraft.h"
#include "SmartAI.h"
#include "SmartScript.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "../../src/server/scripts/Kalimdor/RuinsOfAhnQiraj/ruins_of_ahnqiraj.h"

namespace ac
{
    #include "../../src/server/scripts/Kalimdor/RuinsOfAhnQiraj/boss_kurinnaxx.cpp"
}

namespace sc
{
enum Spells
{
    SPELL_MORTAL_WOUND              = 25646,
    SPELL_SAND_TRAP                 = 25648,
    SPELL_ENRAGE                    = 26527,
    SPELL_HARDENED                  = 62733,
    SPELL_WIDE_SLASH                = 25814,
    SPELL_THRASH                    = 3391,
    SPELL_VERTEX_COLOR_RED          = 69844,

    SPELL_PLAYER_HUSTLE             = 49857,
    SPELL_PLAYER_MARKED             = 34832
};

enum Texts
{
    SAY_CHASE_TARGET        = 1,
    SAY_KURINNAXX_DEATH     = 5 // Yell by 'Ossirian the Unscarred'
};

enum Phases
{
    PHASE_NORMAL,
    PHASE_CHASE
};

/**
 * @brief SmallCraft AI for the Kurinnaxx (15348) creature.
 */
struct boss_kurinnaxx : public BossAI
{
    boss_kurinnaxx(Creature* creature) : BossAI(creature, DATA_KURINNAXX) {}

    void InitializeAI() override
    {
        // increase aggro radius
        me->m_CombatDistance = 50.0f;

        // don't interrupt casting
        scheduler.SetValidator([this]
        {
            return !me->HasUnitState(UNIT_STATE_CASTING);
        });
    }

    void Reset() override
    {
        BossAI::Reset();

        me->SetReactState(REACT_DEFENSIVE);
        me->SetSpeed(MOVE_RUN, me->GetCreatureTemplate()->speed_run);
        me->SetControlled(false, UNIT_STATE_STUNNED);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_STUN, false);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_AURA_MOD_STUN, false);
        _immuneToStun = false;
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        scheduler.CancelAll();
        _phaseNormal();
    }

    void SpellHit(Unit* caster, SpellInfo const* spellInfo) override
    {
        // if the spell that hit Kurinnaxx is a stun
        if (sc::SpellHasAuraWithType(spellInfo, AuraType::SPELL_AURA_MOD_STUN))
        {
            LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx was hit by a stun spell! {} ({})",
                spellInfo->SpellName[0],
                spellInfo->Id
            );

            // instantly remove the original spell
            me->RemoveAurasDueToSpell(spellInfo->Id);

            // if the max spell duration is greater than 4s, transition to PHASE_CHASE
            if
            (
                spellInfo->GetMaxDuration() && spellInfo->GetMaxDuration() > 4000 &&    // only longer, intentional stuns/roots
                !spellInfo->IsPassive() &&                                              // prevents passive procs like Mage's freeze talent from triggering this
                !_immuneToStun                                                          // our own stun immunity tracking
            )
            {
                LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Transitioning to PHASE_CHASE.");

                // schedule events for PHASE_CHASE
                _phaseChase();
            }
        }

        BossAI::SpellHit(caster, spellInfo);
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->GetReactState() != REACT_PASSIVE)
        {
            BossAI::UpdateAI(diff);
        }
        else
        {
            scheduler.Update(diff);
        }
    }

    void JustDied(Unit* killer) override
    {
        // summon Andorov (15471) - dialog with him initiates Rajaxx encounter
        if (killer)
        {
            killer->GetMap()->LoadGrid(-9502.80f, 2042.65f); // Ossirian grid
            killer->GetMap()->LoadGrid(-8538.17f, 1486.09f); // Andorov run path grid

            if (Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself())
            {
                Creature* creature = player->SummonCreature(NPC_ANDOROV, -8538.177f, 1486.0956f, 32.39054f, 3.7638654f, TEMPSUMMON_CORPSE_DESPAWN, 600000000);
                if (creature)
                {
                    creature->setActive(true);
                }
            }
        }

        // Ossirian yell emote
        if (Creature* ossirian = instance->GetCreature(DATA_OSSIRIAN))
        {
            ossirian->setActive(true);
            if (ossirian->GetAI())
                ossirian->AI()->Talk(SAY_KURINNAXX_DEATH);
        }

        BossAI::JustDied(killer);
    }

private:
    void _phaseNormal()
    {
        LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Scheduling PHASE_NORMAL.");

        // schedule all abilities
        scheduler.Schedule(8s, 10s, PHASE_NORMAL, [this](TaskContext context)
        {
            DoCastVictim(SPELL_MORTAL_WOUND);
            context.Repeat(8s, 10s);
        }).Schedule(5s, 15s, [this](TaskContext context)
        {
            if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 100.f, true))
            {
                target->CastSpell(target, SPELL_SAND_TRAP, true, nullptr, nullptr, me->GetGUID());
            }
            context.Repeat(5s, 15s);
        }).Schedule(10s, 15s, PHASE_NORMAL, [this](TaskContext context)
        {
            DoCastSelf(SPELL_WIDE_SLASH);
            context.Repeat(12s, 15s);
        }).Schedule(16s, PHASE_NORMAL, [this](TaskContext context)
        {
            DoCastSelf(SPELL_THRASH);
            context.Repeat(16s);
        });

        // enrage at 30%
        ScheduleHealthCheckEvent(30, [&]
        {
            scheduler.Schedule(2s, [this](TaskContext /*context*/)
            {
                DoCastSelf(SPELL_ENRAGE);
            });
        });
    }

    void _phaseChase()
    {
        LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Scheduling PHASE_CHASE.");

        // pick a target who is not the current tank (unless only the tank is left)
        if (me->GetThreatMgr().GetThreatListSize() > 1)
        {
            _chaseVictim = SelectTarget(SelectTargetMethod::Random, 1, 100.0f, true, false);
        }
        else
        {
            _chaseVictim = SelectTarget(SelectTargetMethod::MaxThreat, 0, 100.0f, true, true);
        }

        if (!_chaseVictim)
        {
            return;
        }

        scheduler.CancelAll();

        LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx is getting ready to chase {}!", _chaseVictim->GetName());

        // apply new immunities
        LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx is immune to taunt and stun.");
        me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
        _immuneToStun = true;

        // apply one last stack of Mortal Wound to the current tank
        if (Unit* tank = me->GetVictim())
        {
            me->AddAura(SPELL_MORTAL_WOUND, tank);
        }

        // fake a 2s stun
        me->TextEmote("Kurinnaxx is stunned!", 0, false);
        me->SetControlled(true, UNIT_STATE_STUNNED);
        me->SetControlled(true, UNIT_STATE_ROOT);

        me->GetThreatMgr().ResetAllThreat();

        // be still
        me->SetReactState(REACT_PASSIVE);
        me->AttackStop();
        me->StopMoving();
        me->GetMotionMaster()->Clear();

        // remove the stun
        scheduler.Schedule(1950ms, [this](TaskContext /*context*/)
        {
            me->SetControlled(false, UNIT_STATE_STUNNED);
        });

        // prep for chase
        scheduler.Schedule(2s, [this](TaskContext /*context*/)
        {
            // stare down victim
            me->SetFacingToObject(_chaseVictim);
            sc::Talk(me, "Enraged, Kurinnaxx fixates on " + _chaseVictim->GetName() + "!", CHAT_MSG_RAID_BOSS_EMOTE);

            // reduce all damage taken
            Aura* auraHardened = me->AddAura(SPELL_HARDENED, me);
            auraHardened->SetMaxDuration(18000);
            auraHardened->SetDuration(18000);

            // enrage!
            // extends 10 seconds into the "cooldown" phase
            Aura* auraEnraged = me->AddAura(SPELL_ENRAGE, me);
            auraEnraged->SetMaxDuration(28000);
            auraEnraged->SetDuration(28000);

            // hunter's mark
            Aura* auraMark = me->AddAura(SPELL_PLAYER_MARKED, _chaseVictim);
            auraMark->SetMaxDuration(18000);
            auraMark->SetDuration(18000);

            // help the victim out with some extra speed
            Aura* auraHustle = me->AddAura(SPELL_PLAYER_HUSTLE, _chaseVictim);
            auraHustle->SetMaxDuration(19500);
            auraHustle->SetDuration(19500);

            // go get 'em
            scheduler.Schedule(3s, [this](TaskContext /*context*/)
            {
                LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx is chasing {} for 15 seconds!", _chaseVictim->GetName());
                me->GetThreatMgr().AddThreat(_chaseVictim, 10000000.0f);
                me->SetControlled(false, UNIT_STATE_ROOT);
                me->SetSpeed(MOVE_RUN, me->GetCreatureTemplate()->speed_run * 0.8f);
                me->SetReactState(REACT_AGGRESSIVE);

                DoMeleeAttackIfReady();

                // chase for 15 seconds, then start going back to normal
                scheduler.Schedule(15s, [this](TaskContext /*context*/)
                {
                    LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx returns to PHASE_NORMAL.");

                    // normal speed
                    me->SetSpeed(MOVE_RUN, me->GetCreatureTemplate()->speed_run);

                    // normal size (overrides Enrage's size increase)
                    me->SetObjectScale(me->GetCreatureTemplate()->scale);

                    // no longer immune to taunt
                    LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx is no longer immune to taunt.");
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);

                    sc::Talk(me, "Kurinnaxx's enrage starts to fade...", CHAT_MSG_RAID_BOSS_EMOTE);

                    // ensure that Kurinnaxx stays red until they are back to normal
                    Aura* redAura = me->AddAura(SPELL_VERTEX_COLOR_RED, me);
                    redAura->SetMaxDuration(20000);
                    redAura->SetDuration(20000);

                    // schedule the removal of stun immunity
                    scheduler.Schedule(20s, [this](TaskContext /*context*/)
                    {
                        sc::Talk(me, "Kurinnaxx's enrage fades.", CHAT_MSG_RAID_BOSS_EMOTE);
                        LOG_DEBUG("module.SmallCraft", "SmallCraft:boss_kurinnaxx: Kurinnaxx is no longer immune to stun.");
                        _immuneToStun = false;

                        // restore normal size
                        me->SetObjectScale(me->GetCreatureTemplate()->scale);
                    });

                    // resume attacking
                    DoZoneInCombat();
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->GetThreatMgr().ResetAllThreat();
                    if (me->GetThreatMgr().SelectVictim())
                    {
                        me->GetMotionMaster()->MoveChase(me->GetVictim());
                        me->Attack(me->GetVictim(), true);
                        me->GetThreatMgr().AddThreat(me->GetVictim(), 1000.0f);
                        DoZoneInCombat(me, 1000.0f);
                    }

                    _phaseNormal();
                });
            });
        });
    }

    Unit* _chaseVictim = nullptr;
    bool _immuneToStun = false;

};

// class sc_spell_boss_kurinaxx_hustle : public SpellScriptLoader
// {
// public:
//     sc_spell_boss_kurinaxx_hustle() : SpellScriptLoader("spell_boss_kurinaxx_hustle") { }

//     class sc_spell_boss_kurinaxx_hustle_AuraScript : public AuraScript
//     {
//         PrepareAuraScript(sc_spell_boss_kurinaxx_hustle_AuraScript);

//         void HandleEffectApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
//         {
//             SetMaxDuration(18000);
//             SetDuration(18000);
//         }

//         void Register() override
//         {
//             OnEffectApply += AuraEffectApplyFn(sc_spell_boss_kurinaxx_hustle_AuraScript::HandleEffectApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
//         }
//     };

//     AuraScript* GetAuraScript() const override
//     {
//         return new sc_spell_boss_kurinaxx_hustle_AuraScript();
//     }
// };

// struct go_sand_trap : public GameObjectAI
// {
//     go_sand_trap(GameObject* go) : GameObjectAI(go) { }

//     void Reset() override
//     {
//         _scheduler.Schedule(5s, [this](TaskContext /*context*/)
//         {
//             if (InstanceScript* instance = me->GetInstanceScript())
//                 if (Creature* kurinnaxx = instance->GetCreature(DATA_KURINNAXX))
//                     me->Use(kurinnaxx);
//         });
//     }

//     void UpdateAI(uint32 const diff) override
//     {
//         _scheduler.Update(diff);
//     }

// protected:
//     TaskScheduler _scheduler;
// };

class sc_boss_kurinnaxx_DatabaseScript : public DatabaseScript
{
public:
    sc_boss_kurinnaxx_DatabaseScript() : DatabaseScript("sc_boss_kurinnaxx_DatabaseScript") { }

    void OnAfterDatabaseLoadCreatureTemplates(std::vector<CreatureTemplate*> creatureTemplates) override
    {
        // Kurinnaxx (15348) - Boss
        creatureTemplates[NPC_KURINNAXX]->MechanicImmuneMask = 617297755; // allow stun
    }
};

void load_sc_boss_kurinnaxx()
{
    LOG_DEBUG("module.SmallCraft", "SmallCraft: Vanilla/AhnQiraj/Kurinnaxx is enabled.");

    // Kurinnaxx (15348) - Boss
    RegisterCreatureAI(boss_kurinnaxx);

    // // Spells
    // RegisterSmallcraftSpellScript(SPELL_PLAYER_HUSTLE, sc_spell_boss_kurinaxx_hustle);

    new sc_boss_kurinnaxx_DatabaseScript();
}
} // namespace sc