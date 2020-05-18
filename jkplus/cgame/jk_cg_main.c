/*
======================= Jedi Knight Plus Mod ========================
By Tr!Force. Work copyrighted (C) with holder attribution 2005 - 2020
=====================================================================
[Description]: Main client module, cvar definitions, etc...
=====================================================================
*/

#include "../../code/cgame/cg_local.h"	// Original header

/*
=====================================================================
Cvar table list
=====================================================================
*/

static qboolean jkmod_duel_warning = qfalse;

typedef struct { // Cvar table struct
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

vmCvar_t	jkcvar_cg_privateDuel;

vmCvar_t	jkcvar_cg_test1;
vmCvar_t	jkcvar_cg_test2;

static cvarTable_t	JKModCGCvarTable[] = {

	{ &jkcvar_cg_privateDuel,		"jk_cg_privateDuel",		"0",	CVAR_ARCHIVE },

	{ &jkcvar_cg_test1,				"jk_cg_test1",				"0",	CVAR_ARCHIVE },
	{ &jkcvar_cg_test2,				"jk_cg_test2",				"0",	CVAR_ARCHIVE },

};

static int JKModCGCvarTableSize = sizeof(JKModCGCvarTable) / sizeof(JKModCGCvarTable[0]);

/*
=====================================================================
Register / update cvars functions
=====================================================================
*/

void JKMod_CG_RegisterCvars(void)
{
	int			i;
	cvarTable_t	*cv;

	// Register all the cvars
	for (i = 0, cv = JKModCGCvarTable; i < JKModCGCvarTableSize; i++, cv++) {
		trap_Cvar_Register(cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags);
	}

	// Launch original register cvars function
	BaseJK2_CG_RegisterCvars();

	// Set the client plugin version
	trap_Cvar_Register(NULL, "jkmod_clientversion", "", CVAR_USERINFO | CVAR_ROM);
	trap_Cvar_Set("jkmod_clientversion", JK_VERSION);
}

void JKMod_CG_UpdateCvars(void)
{
	int			i;
	cvarTable_t	*cv;

	// Update all the cvars
	for (i = 0, cv = JKModCGCvarTable; i < JKModCGCvarTableSize; i++, cv++) {
		trap_Cvar_Update(cv->vmCvar);
	}

	// Warning private duel dimiension
	if (jkcvar_cg_privateDuel.integer)
	{
		if (!(cgs.jkmodCvar.altDimensions & (1 << DIMENSION_DUEL)))
		{
			if (!jkmod_duel_warning) Com_Printf("WARNING: Duel dimension is not available, you may collide with a non-dueling player during private duels\n");
			jkmod_duel_warning = qtrue;
		}
		else jkmod_duel_warning = qfalse;
	}
	else jkmod_duel_warning = qfalse;

	// Launch original update cvars function
	BaseJK2_CG_UpdateCvars();
}
