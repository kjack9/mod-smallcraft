#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "sc_boss_jeklik.cpp"

class sc_instance_zulgurub_GlobalScript : public GlobalScript
{
public:
    sc_instance_zulgurub_GlobalScript() : GlobalScript("sc_instance_zulgurub_GlobalScript") { }

    void OnLoadSpellCustomAttr(SpellInfo* spellInfo) override
    {
        switch (spellInfo->Id)
        {
            // Jeklik - Bat Aspect Boss
            // Blaze (23972)
            case (23972):
                LOG_DEBUG("module.Smallcraft", "sc::sc_instance_zulgurub_GlobalScript:: Correcting spell {} ({})...", spellInfo->Id, spellInfo->SpellName[0]);
                spellInfo->AttributesEx2 |= SPELL_ATTR2_CANT_CRIT;
                break;
            default:
                break;
        }
    }
};


void load_sc_instance_zulgurub()
{
    new sc_instance_zulgurub_GlobalScript();
    load_sc_boss_jeklik();
}