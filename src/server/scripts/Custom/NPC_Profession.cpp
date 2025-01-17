/*
	Profession trainer for Trinitycore
	Code: Unknown
	Updated: by Nekro
	July 2020
	Discord: https://discord.gg/Zgn2SvH
*/
#include "Language.h"
#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"
#include "SpellMgr.h"
#include "GossipDef.h"
#include <numeric>
#include "Player.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include <string>
#include "StringFormat.h"

#define DEFAULT_MESSAGE 907

class NPC_Profession : public CreatureScript
{
public:
    NPC_Profession() : CreatureScript("NPC_Profession") { }
	
	struct NPC_ProfessionAI : public ScriptedAI
    {
        NPC_ProfessionAI(Creature* creature) : ScriptedAI(creature) { }     
        uint32 maxskillcount = sConfigMgr->GetIntDefault("MaxPrimaryTradeSkill", 2);
        std::string maxamount = "Failure! You already have " + std::to_string(maxskillcount) + " professions.";
        const char * mit = maxamount.c_str();
		bool PlayerAlreadyHasMaxProfessions(Player* player)
		{
			uint32 skillCount = 0;
			if (player->HasSkill(SKILL_MINING))
				skillCount++;
			if (player->HasSkill(SKILL_SKINNING))
				skillCount++;
			if (player->HasSkill(SKILL_HERBALISM))
				skillCount++;
		
			if (skillCount >= maxskillcount)
				return true;
		
			for (uint32 i = 1; i < sSkillLineStore.GetNumRows(); ++i)
			{
				SkillLineEntry const *SkillInfo = sSkillLineStore.LookupEntry(i);
				if (!SkillInfo)
					continue;
		
				if (SkillInfo->CategoryID == SKILL_CATEGORY_SECONDARY)
					continue;
		
				if ((SkillInfo->CategoryID != SKILL_CATEGORY_PROFESSION) || !SkillInfo->CanLink)
					continue;
		
				uint32 skillID = SkillInfo->ID;
				if (player->HasSkill(skillID))
					skillCount++;
		
				if (skillCount >= maxskillcount)
					return true;
			}
		
			return false;
		}
		
		bool LearnAllRecipesInProfession(Player* player, SkillType skill)
		{
			char const* skill_name;
		
			SkillLineEntry const* SkillInfo = sSkillLineStore.LookupEntry(skill);
			//skill_name = SkillInfo->name[ChatHandler(player->GetSession()).GetSessionDbcLocale()];
			
			uint8 locale = 0;
            for (; locale < TOTAL_LOCALES; ++locale)
            {
                if (locale == ChatHandler(player->GetSession()).GetSessionDbcLocale())
                    continue;

                skill_name = SkillInfo->DisplayName[locale];
            }

			if (!SkillInfo)
				return false;
			
			if (sConfigMgr->GetBoolDefault("ProfessionNPC.Max", true))
			{
			
				player->SetSkill(SkillInfo->ID, player->GetSkillStep(SkillInfo->ID), 450, 450);
				LearnSkillRecipesHelper(player, SkillInfo->ID);
				ChatHandler(player->GetSession()).PSendSysMessage(LANG_COMMAND_LEARN_ALL_RECIPES, skill_name);
			}
			else
			{
				
				player->SetSkill(SkillInfo->ID, player->GetSkillStep(SkillInfo->ID), 1, 75);
				LearnApprentice(player, skill);
			}

		
			return true;
		}
		
		void LearnApprentice(Player* player, SkillType skill)
		{
			if (skill == SKILL_ALCHEMY)
			{
				player->LearnSpell(2259, false);
			}
			
			if (skill == SKILL_BLACKSMITHING)
			{
				player->LearnSpell(2018, false);
			}
			
			if (skill == SKILL_COOKING)
			{
				player->LearnSpell(2550, false);
			}
			
			if (skill == SKILL_ENCHANTING)
			{
				player->LearnSpell(7411, false);
			}
			
			if (skill == SKILL_ENGINEERING)
			{
				player->LearnSpell(4036, false);
			}
			
			if (skill == SKILL_FIRST_AID)
			{
				player->LearnSpell(3273, false);
			}
			
			if (skill == SKILL_FISHING)
			{
				player->LearnSpell(7620, false);
			}
			
			if (skill == SKILL_HERBALISM)
			{
				player->LearnSpell(2366, false);
			}
			
			if (skill == SKILL_INSCRIPTION)
			{
				player->LearnSpell(45357, false);
			}
			
			if (skill == SKILL_JEWELCRAFTING)
			{
				player->LearnSpell(25229, false);
			}
			
			if (skill == SKILL_LEATHERWORKING)
			{
				player->LearnSpell(2108, false);
			}
			
			if (skill == SKILL_MINING)
			{
				player->LearnSpell(2575, false);
			}
			
			if (skill == SKILL_SKINNING)
			{
				player->LearnSpell(8613, false);
			}
			
			if (skill == SKILL_TAILORING)
			{
				player->LearnSpell(3908, false);
			}
			

		}
		
		void LearnSkillRecipesHelper(Player* player, uint32 skillId)
		{
        uint32 classmask = player->GetClassMask();

        for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
        {
            SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
            if (!skillLine)
                continue;

            // wrong skill
            if (skillLine->SkillLine != skillId)
                continue;

            // not high rank
            if (skillLine->SupercededBySpell)
                continue;

            // skip racial skills
            if (skillLine->RaceMask != 0)
                continue;

            // skip wrong class skills
            if (skillLine->ClassMask && (skillLine->ClassMask & classmask) == 0)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(skillLine->Spell);
            if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
                continue;

			player->LearnSpell(skillLine->Spell, false);
			

        }
		
		
    }
	
		bool IsSecondarySkill(SkillType skill)
		{
			return skill == SKILL_COOKING || skill == SKILL_FIRST_AID || skill == SKILL_FISHING;
		}
		
		void CompleteLearnProfession(Player* player, /*Creature* creature,*/ SkillType skill)
		{
			if ((PlayerAlreadyHasMaxProfessions(player)) && (!IsSecondarySkill(skill)))
				ChatHandler(player->GetSession()).PSendSysMessage(mit);
			else
			{
				if (!LearnAllRecipesInProfession(player, skill))
					ChatHandler(player->GetSession()).PSendSysMessage("Failure! Internal.");
			}
		}
		
		bool OnGossipHello(Player* player)
		{
			if (!player || !me)
				return true;
			
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\trade_alchemy:30|t Alchemy", GOSSIP_SENDER_MAIN, 1);
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\INV_Ingot_05:30|t Blacksmithing", GOSSIP_SENDER_MAIN, 2);
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\INV_Misc_LeatherScrap_02:30|t Leatherworking", GOSSIP_SENDER_MAIN, 3);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\INV_Fabric_Felcloth_Ebon:30|t Tailoring.", GOSSIP_SENDER_MAIN, 4);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\inv_misc_wrench_01:30|t Engineering.", GOSSIP_SENDER_MAIN, 5);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\trade_engraving:30|t Enchanting.", GOSSIP_SENDER_MAIN, 6);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\inv_misc_gem_01:30|t Jewelcrafting.", GOSSIP_SENDER_MAIN, 7);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\INV_Scroll_08:30|t Inscription.", GOSSIP_SENDER_MAIN, 8);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\INV_Misc_Herb_07:30|t Herbalism.", GOSSIP_SENDER_MAIN, 9);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\inv_misc_pelt_wolf_01:30|t Skinning.", GOSSIP_SENDER_MAIN, 10);
            AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\trade_mining:30|t Mining.", GOSSIP_SENDER_MAIN, 11);
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\inv_misc_food_15:30|t Cooking", GOSSIP_SENDER_MAIN, 12);
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\Spell_Holy_SealOfSacrifice:30|t First Aid", GOSSIP_SENDER_MAIN, 13);
			AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "|TInterface\\icons\\Inv_fishingpole_02:30|t Fishing", GOSSIP_SENDER_MAIN, 14);
			AddGossipItemFor(player, GOSSIP_ICON_TALK, "|TInterface\\icons\\spell_chargenegative:30|t Nevermind!", GOSSIP_SENDER_MAIN, 15);
			SendGossipMenuFor(player, DEFAULT_MESSAGE, me->GetGUID());
			return true;
		}
		
		bool OnGossipSelect(Player* player, /*Creature* creature, */uint32  /*sender */, uint32 gossipListId)
		{
			uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
			if (!player || !me)
				return true;
		
			
			player->PlayerTalkClass->ClearMenus();
		
			switch (action)
			{
			case 1:
				if (player->HasSkill(SKILL_ALCHEMY))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_ALCHEMY);
				CloseGossipMenuFor(player);
				break;
		
			case 2:
				if (player->HasSkill(SKILL_BLACKSMITHING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_BLACKSMITHING);
				CloseGossipMenuFor(player);
				break;
		
			case 3:
				if (player->HasSkill(SKILL_LEATHERWORKING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_LEATHERWORKING);
				CloseGossipMenuFor(player);
				break;
		
			case 4:
				if (player->HasSkill(SKILL_TAILORING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_TAILORING);
				player->PlayerTalkClass->SendCloseGossip();
				break;
		
			case 5:
				if (player->HasSkill(SKILL_ENGINEERING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_ENGINEERING);
				CloseGossipMenuFor(player);
				break;
		
			case 6:
				if (player->HasSkill(SKILL_ENCHANTING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_ENCHANTING);
				CloseGossipMenuFor(player);
				break;
		
			case 7:
				if (player->HasSkill(SKILL_JEWELCRAFTING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_JEWELCRAFTING);
				CloseGossipMenuFor(player);
				break;
		
			case 8:
				if (player->HasSkill(SKILL_INSCRIPTION))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_INSCRIPTION);
				CloseGossipMenuFor(player);
				break;
		
			case 9:
				if (player->HasSkill(SKILL_HERBALISM))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_HERBALISM);
				CloseGossipMenuFor(player);
				break;
		
			case 10:
				if (player->HasSkill(SKILL_SKINNING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_SKINNING);
				CloseGossipMenuFor(player);
				break;
		
			case 11:
				if (player->HasSkill(SKILL_MINING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_MINING);
				CloseGossipMenuFor(player);
				break;
		
			case 12:
				if (player->HasSkill(SKILL_COOKING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_COOKING);
				CloseGossipMenuFor(player);
				break;
		
			case 13:
				if (player->HasSkill(SKILL_FIRST_AID))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_FIRST_AID);
				CloseGossipMenuFor(player);;
				break;
		
			case 14:
				if (player->HasSkill(SKILL_FISHING))
				{
					CloseGossipMenuFor(player);
					return false;
				}
		
				CompleteLearnProfession(player, SKILL_FISHING);
				CloseGossipMenuFor(player);;
				break;
		
			case 15:
				CloseGossipMenuFor(player);;
				break;
			
			}
		
			return true;
		}
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new NPC_ProfessionAI(creature);
    }
		
};

void AddSC_NPC_Profession()
{
    new NPC_Profession();
}
