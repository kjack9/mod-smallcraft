#include "Config.h"
#include "Log.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "CreatureGroups.h"
#include "InstanceScript.h"
#include "Player.h"
#include "TaskScheduler.h"

#include "../../src/server/scripts/Kalimdor/RuinsOfAhnQiraj/ruins_of_ahnqiraj.h"
// namespace ac
// {
//     #include "../../src/server/scripts/Kalimdor/RuinsOfAhnQiraj/instance_ruins_of_ahnqiraj.cpp"
// }

namespace sc
{
ObjectData const creatureData[] =
{
    { NPC_BURU,      DATA_BURU      },
    { NPC_KURINNAXX, DATA_KURINNAXX },
    { NPC_RAJAXX,    DATA_RAJAXX    },
    { NPC_AYAMISS,   DATA_AYAMISS   },
    { NPC_OSSIRIAN,  DATA_OSSIRIAN  },
    { NPC_QUUEZ,     DATA_QUUEZ     },
    { NPC_TUUBID,    DATA_TUUBID    },
    { NPC_DRENN,     DATA_DRENN     },
    { NPC_XURREM,    DATA_XURREM    },
    { NPC_YEGGETH,   DATA_YEGGETH   },
    { NPC_PAKKON,    DATA_PAKKON    },
    { NPC_ZERRAN,    DATA_ZERRAN    },
    { 0,             0              }
};

enum RajaxxWaveEvent
{
    SAY_WAVE3  = 0,
    SAY_WAVE4  = 1,
    SAY_WAVE5  = 2,
    SAY_WAVE6  = 3,
    SAY_WAVE7  = 4,
    SAY_ENGAGE = 5,

    DATA_RAJAXX_WAVE_ENGAGED = 1,
    GROUP_RAJAXX_WAVE_TIMER  = 1
};

std::array<uint32, 8> RajaxxWavesData[] =
{
    { DATA_QUUEZ,     0          },
    { DATA_TUUBID,    0          },
    { DATA_DRENN,     SAY_WAVE3  },
    { DATA_XURREM,    SAY_WAVE4  },
    { DATA_YEGGETH,   SAY_WAVE5  },
    { DATA_PAKKON,    SAY_WAVE6  },
    { DATA_ZERRAN,    SAY_WAVE7  },
    { DATA_RAJAXX,    SAY_ENGAGE }
};

class instance_ruins_of_ahnqiraj : public InstanceMapScript
{
public:
    instance_ruins_of_ahnqiraj() : InstanceMapScript("instance_ruins_of_ahnqiraj", 509) { }

    struct instance_ruins_of_ahnqiraj_InstanceMapScript : public InstanceScript
    {
        instance_ruins_of_ahnqiraj_InstanceMapScript(Map* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetBossNumber(NUM_ENCOUNTER);
            LoadObjectData(creatureData, nullptr);
            _rajaxWaveCounter = 0;
            _buruPhase = 1;
        }

        void OnPlayerEnter(Player* player) override
        {
            if (GetBossState(DATA_KURINNAXX) == DONE && GetBossState(DATA_RAJAXX) != DONE)
            {
                Map::PlayerList const &playerList = instance->GetPlayers();

                if (playerList.IsEmpty())
                    return;

                bool anyPlayerInCombat = false;
                for (Map::PlayerList::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
                {
                    Player* player = itr->GetSource();
                    if (player && player->IsInCombat())
                    {
                        anyPlayerInCombat = true;
                        break;
                    }
                }

                if (!anyPlayerInCombat && !_andorovGUID)
                {
                    instance->LoadGrid(-8538.17f, 1486.09f); // Andorov run path grid
                    Creature* creature = player->SummonCreature(NPC_ANDOROV, -8538.177f, 1486.0956f, 32.39054f, 3.7638654f, TEMPSUMMON_CORPSE_DESPAWN, 600000000);
                    if (creature)
                    {
                        creature->setActive(true);
                    }
                }
            }
        }

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            switch (creature->GetEntry())
            {
                case NPC_KURINNAXX:
                    _kurinnaxxGUID = creature->GetGUID();
                    break;
                case NPC_RAJAXX:
                    _rajaxxGUID = creature->GetGUID();
                    break;
                case NPC_MOAM:
                    _moamGUID = creature->GetGUID();
                    break;
                case NPC_BURU:
                    _buruGUID = creature->GetGUID();
                    break;
                case NPC_OSSIRIAN:
                    _ossirianGUID = creature->GetGUID();
                    break;
                case NPC_ANDOROV:
                    _andorovGUID = creature->GetGUID();
                    break;
            }
        }

        void OnCreatureEvade(Creature* creature) override
        {
            if (CreatureGroup* formation = creature->GetFormation())
            {
                if (Creature* leader = formation->GetLeader())
                {
                    switch (leader->GetEntry())
                    {
                        case NPC_QUUEZ:
                        case NPC_TUUBID:
                        case NPC_DRENN:
                        case NPC_XURREM:
                        case NPC_YEGGETH:
                        case NPC_PAKKON:
                        case NPC_ZERRAN:
                            if (!formation->IsFormationInCombat())
                            {
                                ResetRajaxxWaves();
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (creature->GetEntry() == NPC_RAJAXX)
            {
                ResetRajaxxWaves();
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_RAJAXX_WAVE_ENGAGED:
                    scheduler.CancelGroup(GROUP_RAJAXX_WAVE_TIMER);
                    scheduler.Schedule(2min, [this](TaskContext context)
                    {
                        CallNextRajaxxLeader();
                        context.SetGroup(GROUP_RAJAXX_WAVE_TIMER);
                        context.Repeat();
                    });
                    break;
                case DATA_BURU_PHASE:
                    _buruPhase = data;
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_BURU_PHASE)
                return _buruPhase;

            return 0;
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (Creature* creature = unit->ToCreature())
            {
                if (CreatureGroup* formation = creature->GetFormation())
                {
                    if (Creature* leader = formation->GetLeader())
                    {
                        switch (leader->GetEntry())
                        {
                            case NPC_QUUEZ:
                            case NPC_TUUBID:
                            case NPC_DRENN:
                            case NPC_XURREM:
                            case NPC_YEGGETH:
                            case NPC_PAKKON:
                            case NPC_ZERRAN:
                                scheduler.CancelAll();
                                scheduler.Schedule(5s, [this, formation](TaskContext /*context*/)
                                {
                                    if (!formation->IsAnyMemberAlive())
                                    {
                                        CallNextRajaxxLeader(true);
                                    }
                                });
                                break;
                            default:
                                break;
                        }
                    }
                }

                if (creature->GetEntry() == NPC_ANDOROV)
                {
                    _andorovGUID.Clear();
                }
            }
        }

        void SetGuidData(uint32 type, ObjectGuid data) override
        {
            if (type == DATA_PARALYZED)
                _paralyzedGUID = data;
        }

        ObjectGuid GetGuidData(uint32 type) const override
        {
            switch (type)
            {
                case DATA_KURINNAXX:
                    return _kurinnaxxGUID;
                case DATA_RAJAXX:
                    return _rajaxxGUID;
                case DATA_MOAM:
                    return _moamGUID;
                case DATA_BURU:
                    return _buruGUID;
                case DATA_OSSIRIAN:
                    return _ossirianGUID;
                case DATA_PARALYZED:
                    return _paralyzedGUID;
                case DATA_ANDOROV:
                    return _andorovGUID;
            }

            return ObjectGuid::Empty;
        }

        void CallNextRajaxxLeader(bool announce = false)
        {
            ++_rajaxWaveCounter;

            if (Creature* nextLeader = GetCreature(RajaxxWavesData[_rajaxWaveCounter].at(0)))
            {
                if (announce)
                {
                    if (_rajaxWaveCounter >= 2)
                    {
                        if (Creature* rajaxx = GetCreature(DATA_RAJAXX))
                        {
                            rajaxx->AI()->Talk(RajaxxWavesData[_rajaxWaveCounter].at(1));
                        }
                    }
                }

                if (nextLeader->IsAlive())
                {
                    Creature* generalAndorov = instance->GetCreature(_andorovGUID);
                    if (generalAndorov && generalAndorov->IsAlive() && generalAndorov->AI()->GetData(DATA_ANDOROV))
                    {
                        nextLeader->AI()->AttackStart(generalAndorov);
                    }
                    else
                    {
                        nextLeader->SetInCombatWithZone();

                        if (!nextLeader->SelectVictim())
                        {
                            LOG_DEBUG("module.SmallCraft.ai", "SmallCraft:instance_ruins_of_ahnqiraj:CallNextRajaxxLeader: No victim for {}, reset.", nextLeader->GetName());
                            ResetRajaxxWaves();
                        }
                    }
                }
                else
                {
                    CallNextRajaxxLeader();
                }
            }
        }

        void ResetRajaxxWaves()
        {
            _rajaxWaveCounter = 0;
            scheduler.CancelAll();
            for (auto const& data : RajaxxWavesData)
            {
                if (Creature* creature = GetCreature(data.at(0)))
                {
                    if (CreatureGroup* group = creature->GetFormation())
                    {
                        group->RespawnFormation(true);
                    }
                }
            }
        }

    private:
        ObjectGuid _kurinnaxxGUID;
        ObjectGuid _rajaxxGUID;
        ObjectGuid _moamGUID;
        ObjectGuid _buruGUID;
        ObjectGuid _ossirianGUID;
        ObjectGuid _paralyzedGUID;
        ObjectGuid _andorovGUID;
        uint32 _rajaxWaveCounter;
        uint8 _buruPhase;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_ruins_of_ahnqiraj_InstanceMapScript(map);
    }
};

// Load function declarations
void load_boss_kurinnaxx();

void load_instance_ahnqiraj()
{
    LOG_DEBUG("module.SmallCraft", "SmallCraft: Vanilla/AhnQiraj is enabled.");
    new instance_ruins_of_ahnqiraj();

    if (sConfigMgr->GetOption<bool>("Smallcraft.RaidChanges.Vanilla.AhnQiraj.Kurinnaxx", true))
        load_boss_kurinnaxx();
}
} // namespace sc