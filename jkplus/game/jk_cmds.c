/*
======================= Jedi Knight Plus Mod ========================
By Tr!Force. Work copyrighted (C) with holder attribution 2005 - 2020
=====================================================================
[Description]: Main commands functions
=====================================================================
*/

#include "../../code/game/g_local.h" // Original header

/*
=====================================================================
Drop flag function
=====================================================================
*/

static void JKPlus_dropFlag(gentity_t *ent, int clientNum)
{
	gitem_t		*item = ent->client->sess.sessionTeam == TEAM_RED ? BG_FindItem("team_CTF_blueflag") : BG_FindItem("team_CTF_redflag");
	vec3_t		angles, velocity, org, offset, mins, maxs;
	trace_t		tr;

	if (jkcvar_dropFlag.integer != 1)
	{
		trap_SendServerCommand(ent - g_entities, "print \"Drop flag is disabled by the server\n\"");
		return;
	}

	if (!(g_gametype.integer == GT_CTF || g_gametype.integer == GT_CTY))
	{
		trap_SendServerCommand(ent - g_entities, "cp \"You can't drop a flag in this game type\n\"");
		return;
	}

	if ((ent->client->sess.sessionTeam == TEAM_RED && ent->client->ps.powerups[PW_BLUEFLAG] == 0) || 
		(ent->client->sess.sessionTeam == TEAM_BLUE && ent->client->ps.powerups[PW_REDFLAG] == 0))
	{
		trap_SendServerCommand(ent - g_entities, "cp \"You doesn't have any flag to drop\n\"");
		return;
	}

	if (ent->client->JKPlusDropFlagTime > 0)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"You have to wait %d seconds to take the flag again\n\"", ent->client->JKPlusDropFlagTime));
		return;
	}
	
	ent->client->ps.powerups[PW_REDFLAG] = 0;
	ent->client->ps.powerups[PW_BLUEFLAG] = 0;
	ent->client->ps.persistant[PERS_SCORE] -= CTF_FLAG_BONUS;

	VectorCopy(ent->client->ps.viewangles, angles);

	angles[PITCH] = 0;

	AngleVectors(angles, velocity, NULL, NULL);
	VectorScale(velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight / 2.f;
	VectorScale(velocity, 75, velocity);
	velocity[2] += 50 + random() * 35;

	VectorAdd(ent->client->ps.origin, offset, org);

	VectorSet(mins, -ITEM_RADIUS, -ITEM_RADIUS, 0);
	VectorSet(maxs, ITEM_RADIUS, ITEM_RADIUS, 2 * ITEM_RADIUS);

	trap_Trace(&tr, ent->client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID);
	VectorCopy(tr.endpos, org);

	ent->client->JKPlusDropFlagTime = jkcvar_dropFlagTime.integer;
	LaunchItem(item, org, velocity);
}

/*
=====================================================================
Instructs all chat to be ignored by the given
=====================================================================
*/
void JKPlus_IgnoreClient(char *option, int ignorer, int ignored, qboolean ignore)
{
	if (ignorer == ignored)
	{
		return;
	}
	if (g_entities[ignored].client->pers.connected != CON_CONNECTED)
	{
		return;
	}
	if (ignore)
	{
		if (!Q_stricmp(option, "chat")) {
			g_entities[ignored].client->sess.JKPlusIgnoredChats[ignorer / 32] |= (1 << (ignorer % 32));
		}
		else if (!Q_stricmp(option, "duel")) {
			g_entities[ignored].client->sess.JKPlusIgnoredDuels[ignorer / 32] |= (1 << (ignorer % 32));
		}
	}
	else
	{
		if (!Q_stricmp(option, "chat")) {
			g_entities[ignored].client->sess.JKPlusIgnoredChats[ignorer / 32] &= ~(1 << (ignorer % 32));
		}
		else if (!Q_stricmp(option, "duel")) {
			g_entities[ignored].client->sess.JKPlusIgnoredDuels[ignorer / 32] &= ~(1 << (ignorer % 32));
		}
	}
}

/*
=====================================================================
Checks to see if the given client is being ignored by a specific client
=====================================================================
*/
qboolean JKPlus_IsClientIgnored(char *option, int ignorer, int ignored)
{
	if (!Q_stricmp(option, "chat")) {
		if (g_entities[ignored].client->sess.JKPlusIgnoredChats[ignorer / 32] & (1 << (ignorer % 32)))
		{
			return qtrue;
		}
	}
	else if (!Q_stricmp(option, "duel")) {
		if (g_entities[ignored].client->sess.JKPlusIgnoredDuels[ignorer / 32] & (1 << (ignorer % 32)))
		{
			return qtrue;
		}
	}
	return qfalse;
}

/*
=====================================================================
Clears any possible ignore flags that were set and not reset
=====================================================================
*/
void JKPlus_RemoveFromAllIgnoreLists(char *option, int ignorer)
{
	int i;

	for (i = 0; i < level.maxclients; i++) {
		if (!Q_stricmp(option, "chat")) {
			g_entities[i].client->sess.JKPlusIgnoredChats[ignorer / 32] &= ~(1 << (ignorer % 32));
		}
		else if (!Q_stricmp(option, "duel")) {
			g_entities[i].client->sess.JKPlusIgnoredDuels[ignorer / 32] &= ~(1 << (ignorer % 32));
		}
	}
}

/*
=====================================================================
Client command function
=====================================================================
*/
void JKPlus_ClientCommand(int clientNum)
{
	gentity_t *ent = &g_entities[clientNum];

	char cmd[MAX_TOKEN_CHARS];

	if (!ent->inuse || !ent->client)
	{
		return;
	}

	trap_Argv(0, cmd, sizeof(cmd));

	// Start commands
	if (JKPlus_emoteCheck(cmd, ent))
	{
		return;
	}
	else if (Q_stricmp(cmd, "help") == 0)
	{
		char    arg1[MAX_TOKEN_CHARS];

		trap_Argv(1, arg1, sizeof(arg1));

		if(trap_Argc() < 2)
		{
			trap_SendServerCommand(ent - g_entities, va("print \""
				"^5[^7 Help ^5]^7\n"
				"^7This server is running ^5%s %s.%s.%s\n"
				"^7You can read the desired help topic using following command: ^2/help <topic>\n"
				"^5----------\n"
				"^7Topic list:\n"
				"^3Admin          Accounts       Ranking\n"
				"^3Emotes         Commands       Build\n"
				"^3GamePlay       GameModes      CallVote\n"
				"^3About          Credits        \n"
				"^7\"", JKPLUS_LONGNAME, JKPLUS_MAJOR, JKPLUS_MINOR, JKPLUS_PATCH));
				return;
		}
		if(!Q_stricmp(arg1, "emotes"))
		{
			trap_SendServerCommand(ent - g_entities, va("print \""
				"^5[^7 Emotes ^5]^7\n"
				"^7Emotes are visual animations that allows you to sit down, greet someone, etc...\n"
				"^7There are over 50 emotes to choose from. (Some might be disabled by the server).\n"
				"^7You can assign an emote to a key using the following command: ^2/bind <key> <emote>\n"
				"^5----------\n"
				"^7Emote list:\n"
				"^3Bar            Beg            Buried        Cocky       ComeOn      ComTalk\n"
				"^3CrossArms      DontKillMe     DontKnow      DontKnow2   Explain     Explain2\n"
				"^3FakeDead       Flip           HandHips      Hi          Hug         Kiss\n"
				"^3Kneel          Laugh          Look          Look2       Nod         Point\n"
				"^3Point2         Punch          Sit           Sit2        Sit3        Shake\n"
				"^3Super          Super2         Surrender     Spin        Spin2       Spin3\n"
				"^3Spin4          Taunt2         Taunt3        Think       Threaten    ThumbsUp\n"
				"^3ThumbsDown     TossBack       TossOver      TossUp      Type        Type2\n"
				"^3Victory        Victory2       Waiting       WatchOut    Writing     Writing2\n"
				"^7\""));
				return;
		}
		else
		{
			trap_SendServerCommand(ent - g_entities, va("print \"The option ^3%s ^7is disabled at the moment\n\"", arg1));
			return;
		}
	}
	else if (Q_stricmp(cmd, "dropflag") == 0)
	{
		JKPlus_dropFlag(ent, clientNum);
	}
	else if (Q_stricmp(cmd, "showmotd") == 0)
	{
		if (*jkcvar_serverMotd.string && jkcvar_serverMotd.string[0] && !Q_stricmp(jkcvar_serverMotd.string, "0") == 0)
		{
			ent->client->JKPlusMotdTime = jkcvar_serverMotdTime.integer;
		}
	}
	else if (Q_stricmp(cmd, "ignore") == 0)
	{
		qboolean ignore;
		int		ignored = -1;
		char	*option;
		char	name[MAX_STRING_CHARS];
		char    arg1[MAX_TOKEN_CHARS];
		char    arg2[MAX_TOKEN_CHARS];

		trap_Argv(1, arg1, sizeof(arg1));
		trap_Argv(2, arg2, sizeof(arg2));

		Com_sprintf(arg1, sizeof(arg1), "%s", arg1);
		Com_sprintf(arg2, sizeof(arg2), "%s", arg2);

		if (!jkcvar_playerIgnore.integer) 
		{
			trap_SendServerCommand(ent - g_entities, va("print \"This command is ^1disabled^7 by the server\n\""));
			return;
		}

		if (trap_Argc() < 3)
		{
			trap_SendServerCommand(ent - g_entities, va("print \""
				"^5[^7 Ignore ^5]^7\n"
				"^7Ignore a player chat or duel challenge\n"
				"^7You can use this feature using the following command: ^2/ignore <option> <user>\n"
				"^5----------\n"
				"^7Options:\n"
				"^3chat\n"
				"^3duel\n"
				"^5----------\n"
				"^2Note 1: ^7No need to use full name or color name, you can use just a part of it\n"
				"^2Note 2: ^7Use this command again to undo the changes\n"
				"^7\""));
			return;
		}
		else 
		{
			if (!(!Q_stricmp(arg1, "chat") || !Q_stricmp(arg1, "duel")))
			{
				trap_SendServerCommand(ent - g_entities, va("print \"The option ^3%s ^7is not valid\n\"", arg1));
				return;
			}
			else 
			{
				ignored = JKPlus_ClientNumberFromArg(arg2);

				if (ignored == -1)
				{
					trap_SendServerCommand(ent - g_entities, va("print \"Can't find the name ^3%s\n\"", arg2));
					return;
				}
				else if (ignored == -2)
				{
					trap_SendServerCommand(ent - g_entities, va("print \"There are more names that contains ^3%s\n\"", arg2));
					return;
				}
				else if (ignored >= MAX_CLIENTS || ignored < 0)
				{
					trap_SendServerCommand(ent - g_entities, va("print \"Invalid name for ^3%s\n\"", arg2));
					return;
				}
				else if (ignored == ent->client->ps.clientNum)
				{
					trap_SendServerCommand(ent - g_entities, va("print \"You can't do it to yourself\n\"", arg2));
					return;
				}
				else if (!g_entities[ignored].inuse)
				{
					trap_SendServerCommand(ent - g_entities, va("print \"The user ^3%s ^7is not active\n\"", arg2));
					return;
				}
				else
				{
					ignore = JKPlus_IsClientIgnored(arg1, ent->client->ps.clientNum, ignored) ? qfalse : qtrue;

					JKPlus_IgnoreClient(arg1, ent->client->ps.clientNum, ignored, ignore);

					if (ignore)
					{
						trap_SendServerCommand(ent - g_entities, va("print \"You are ignoring ^3%s ^7%ss now\n\"", g_entities[ignored].client->pers.netname, arg1));
					}
					else
					{
						trap_SendServerCommand(ent - g_entities, va("print \"You are no longer ignoring %ss from ^3%s\n\"", arg1, g_entities[ignored].client->pers.netname));
					}
				}
			}
		}
	}
	else if (Q_stricmp(cmd, "testcmd") == 0)
	{
		char *message = jkcvar_test1.integer == 1 ? "Test 1 is enabled" : "Test 1 is disabled";
		
		trap_SendServerCommand(ent - g_entities, va("print \"%s and the value is %i. Test 2 is %i\n\"", message, jkcvar_test1.integer, jkcvar_test2.integer));
	}

	// Launch original client command function
	else
	{
		BaseJK2_ClientCommand(clientNum, cmd);
	}
}
