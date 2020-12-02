/*
===========================================================================

Return to Castle Wolfenstein multiplayer GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein multiplayer GPL Source Code (RTCW MP Source Code).  

RTCW MP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW MP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW MP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW MP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW MP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
 * name:		cg_consolecmds.c
 *
 * desc:		text commands typed in at the local console, or executed by a key binding
 *
*/


#include "cg_local.h"
#include "../ui/ui_shared.h"

static void CG_UpdateCameraInfo(void);
static void CG_UpdateCameraInfoExt(int startUpdatePoint);
static void CG_GotoViewPointMark_f(void);
static void CG_ChangeSelectedCameraPoints_f(void);

void CG_TargetCommand_f( void ) {
	int targetNum;
	char test[4];

	targetNum = CG_CrosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendClientCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}



/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i",(int)( cg_viewsize.integer + 10 ) ) );
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f( void ) {
	trap_Cvar_Set( "cg_viewsize", va( "%i",(int)( cg_viewsize.integer - 10 ) ) );
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f( void ) {
	CG_Printf( "(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
			   (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
			   (int)cg.refdefViewAngles[YAW] );
}


static void CG_ScoresDown_f( void ) {
	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}


extern menuDef_t *menuScoreboard;
void Menu_Reset( void );          // FIXME: add to right include file

static void CG_LoadHud_f( void ) {
	char buff[1024];
	const char *hudSet;
	memset( buff, 0, sizeof( buff ) );

	String_Init();
	Menu_Reset();

//	trap_Cvar_VariableStringBuffer( "cg_hudFiles", buff, sizeof( buff ) );
//	hudSet = buff;
//	if ( hudSet[0] == '\0' ) {
		hudSet = "ui_mp/hud.txt";
//	}

	CG_LoadMenus( hudSet );
	menuScoreboard = NULL;
}

// TTimo: defined but not used
/*
static void CG_scrollScoresDown_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
	}
}


static void CG_scrollScoresUp_f( void) {
	if (menuScoreboard && cg.scoreBoardShowing) {
		Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
		Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
	}
}


static void CG_spWin_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
//	CG_AddBufferedSound(cgs.media.winnerSound);
	//trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU WIN!", SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
	trap_Cvar_Set("cg_cameraOrbit", "2");
	trap_Cvar_Set("cg_cameraOrbitDelay", "35");
	trap_Cvar_Set("cg_thirdPerson", "1");
	trap_Cvar_Set("cg_thirdPersonAngle", "0");
	trap_Cvar_Set("cg_thirdPersonRange", "100");
//	CG_AddBufferedSound(cgs.media.loserSound);
	//trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
	CG_CenterPrint("YOU LOSE...", SCREEN_HEIGHT * .30, 0);
}
*/

//----(SA)	item (key/pickup) drawing
static void CG_InventoryDown_f( void ) {
	cg.showItems = qtrue;
}

static void CG_InventoryUp_f( void ) {
	cg.showItems = qfalse;
	cg.itemFadeTime = cg.time;
}

//----(SA)	end

static void CG_TellTarget_f( void ) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int clientNum;
	char command[128];
	char message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}


// TTimo: defined but not used
/*
static void CG_NextTeamMember_f( void ) {
  CG_SelectNextPlayer();
}

static void CG_PrevTeamMember_f( void ) {
  CG_SelectPrevPlayer();
}
*/

/////////// cameras

#define MAX_CAMERAS 64  // matches define in splines.cpp
qboolean cameraInuse[MAX_CAMERAS];

int CG_LoadCamera( const char *name ) {
	int i;
	for ( i = 1; i < MAX_CAMERAS; i++ ) {    // start at '1' since '0' is always taken by the cutscene camera
		if ( !cameraInuse[i] ) {
			if ( trap_loadCamera( i, name ) ) {
				cameraInuse[i] = qtrue;
				return i;
			}
		}
	}
	return -1;
}

void CG_FreeCamera( int camNum ) {
	cameraInuse[camNum] = qfalse;
}

/*
==============
CG_StartCamera
==============
*/
void CG_StartCamera( const char *name, qboolean startBlack ) {
	char lname[MAX_QPATH];

	if ( cgs.gametype != GT_SINGLE_PLAYER ) {
		return;
	}

	COM_StripExtension( name, lname, sizeof( lname ) );    //----(SA)	added
	Q_strcat( lname, sizeof( lname ), ".camera" );

	if ( trap_loadCamera( CAM_PRIMARY, va( "cameras/%s", lname ) ) ) {
		cg.cameraMode = qtrue;				// camera on in cgame
		if ( startBlack ) {
			CG_Fade( 0, 0, 0, 255, 0 );		// go black
		}
		trap_Cvar_Set( "cg_letterbox", "1" ); // go letterbox
		trap_SendClientCommand( "startCamera" );	// camera on in game
		trap_startCamera( CAM_PRIMARY, cg.time );	// camera on in client
	} else {
		//----(SA)	temp until radiant stores cameras in own directory
		//			check cameras dir then main dir
		if ( trap_loadCamera( CAM_PRIMARY, name ) ) {
			cg.cameraMode = qtrue;
			trap_SendClientCommand( "startCamera" );
			trap_startCamera( CAM_PRIMARY, cg.time );
			return;
		}
		//----(SA)	end (remove when radiant stores cameras...)
		cg.cameraMode = qfalse;
		trap_SendClientCommand( "stopCamera" );
		CG_Fade( 0, 0, 0, 0, 0 );             // ensure fadeup
		trap_Cvar_Set( "cg_letterbox", "0" );
		CG_Printf( "Unable to load camera %s\n",lname );
	}
}

// TTimo: defined but not used
/*
static void CG_Camera_f( void ) {
	char name[MAX_QPATH];

	if ( cgs.gametype != GT_SINGLE_PLAYER )
		return;

	trap_Argv( 1, name, sizeof(name));

	CG_StartCamera(name, qfalse );
}
*/

static void CG_Fade_f( void ) {
	int r, g, b, a;
	float duration;

	if ( trap_Argc() < 6 ) {
		return;
	}

	r = atof( CG_Argv( 1 ) );
	g = atof( CG_Argv( 2 ) );
	b = atof( CG_Argv( 3 ) );
	a = atof( CG_Argv( 4 ) );

	duration = atof( CG_Argv( 5 ) ) * 1000;

	CG_Fade( r, g, b, a, duration );
}

// NERVE - SMF
static void CG_QuickMessage_f( void ) {
	if ( cg_quickMessageAlt.integer ) {
		trap_UI_Popup( "UIMENU_WM_QUICKMESSAGEALT" );
	} else {
		trap_UI_Popup( "UIMENU_WM_QUICKMESSAGE" );
	}
}

static void CG_OpenLimbo_f( void ) {
	int currentTeam;
	char buf[32];

	// set correct team, also set current team to detect if its changed
	if ( cg.snap ) {
		currentTeam = cg.snap->ps.persistant[PERS_TEAM] - 1;
	} else {
		currentTeam = 0;
	}

	if ( currentTeam > 2 ) {
		currentTeam = 2;
	}

	// Arnout - don't set currentteam when following as it won't be the actual currentteam
	if ( currentTeam != mp_team.integer && cg.snap && !( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
		trap_Cvar_Set( "mp_team", va( "%d", currentTeam ) );
	}

	if ( currentTeam != mp_currentTeam.integer && cg.snap && !( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
		trap_Cvar_Set( "mp_currentTeam", va( "%d", currentTeam ) );
	}

	// set current player type
	if ( mp_currentPlayerType.integer != cg.snap->ps.stats[ STAT_PLAYER_CLASS ] ) {
		trap_Cvar_Set( "mp_currentPlayerType", va( "%i", cg.snap->ps.stats[ STAT_PLAYER_CLASS ] ) );
	}

	// set isSpectator
	trap_Cvar_VariableStringBuffer( "ui_isSpectator", buf, 32 );

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && cg.snap->ps.pm_type != PM_INTERMISSION ) {
		trap_SendConsoleCommand( "+scores\n" );           // NERVE - SMF - blah

		if ( !atoi( buf ) ) {
			trap_Cvar_Set( "ui_isSpectator", "1" );
		}
	} else {
		if ( atoi( buf ) ) {
			trap_Cvar_Set( "ui_isSpectator", "0" );
		}
	}

	trap_UI_Popup( "UIMENU_WM_LIMBO" );
}

static void CG_CloseLimbo_f( void ) {
	trap_UI_ClosePopup( "UIMENU_WM_LIMBO" );
}

static void CG_LimboMessage_f( void ) {
	char teamStr[80], classStr[80], weapStr[80];

	Q_strncpyz( teamStr, CG_TranslateString( CG_Argv( 1 ) ), 80 );
	Q_strncpyz( classStr, CG_TranslateString( CG_Argv( 2 ) ), 80 );
	Q_strncpyz( weapStr, CG_TranslateString( CG_Argv( 3 ) ), 80 );

	CG_PriorityCenterPrint( va( "%s %s %s %s %s.", CG_TranslateString( "You will spawn as an" ),
								teamStr, classStr, CG_TranslateString( "with a" ), weapStr ), SCREEN_HEIGHT - ( SCREEN_HEIGHT * 0.25 ), SMALLCHAR_WIDTH, -1 );
}

static void CG_VoiceChat_f( void ) {
	char chatCmd[64];

	if ( cgs.gametype < GT_WOLF || trap_Argc() != 2 ) {
		return;
	}

	// NERVE - SMF - don't let spectators voice chat
	// NOTE - This cg.snap will be the person you are following, but its just for intermission test
	if ( cg.snap && ( cg.snap->ps.pm_type != PM_INTERMISSION ) ) {
		if ( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR || cgs.clientinfo[cg.clientNum].team == TEAM_FREE ) {
			CG_Printf( "%s", CG_TranslateString( "Can't voice chat as a spectator.\n" ) );
			return;
		}
	}

	trap_Argv( 1, chatCmd, 64 );

	trap_SendConsoleCommand( va( "cmd vsay %s\n", chatCmd ) );
}

static void CG_TeamVoiceChat_f( void ) {
	char chatCmd[64];

	if ( cgs.gametype < GT_WOLF || trap_Argc() != 2 ) {
		return;
	}

	// NERVE - SMF - don't let spectators voice chat
	// NOTE - This cg.snap will be the person you are following, but its just for intermission test
	if ( cg.snap && ( cg.snap->ps.pm_type != PM_INTERMISSION ) ) {
		if ( cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR || cgs.clientinfo[cg.clientNum].team == TEAM_FREE ) {
			CG_Printf( "%s", CG_TranslateString( "Can't team voice chat as a spectator.\n" ) );
			return;
		}
	}

	trap_Argv( 1, chatCmd, 64 );

	trap_SendConsoleCommand( va( "cmd vsay_team %s\n", chatCmd ) );
}

static void CG_SetWeaponCrosshair_f( void ) {
	char crosshair[64];

	trap_Argv( 1, crosshair, 64 );
	cg.newCrosshairIndex = atoi( crosshair ) + 1;
}
// -NERVE - SMF

/*
===================
CG_DumpLocation_f

Dump a target_location definition to a file
===================
*/
static void CG_DumpLocation_f( void ) {
	char locfilename[MAX_QPATH];
	char locname[MAX_STRING_CHARS];
	char *extptr, *buffptr;
	fileHandle_t f;

	// Check for argument
	if ( trap_Argc() < 2 ) {
		CG_Printf( "Usage: dumploc <locationname>\n" );
		return;
	}
	trap_Args( locname, sizeof( locname ) );

	// Open locations file
	Q_strncpyz( locfilename, cgs.mapname, sizeof( locfilename ) );
	extptr = locfilename + strlen( locfilename ) - 4;
	if ( extptr < locfilename || Q_stricmp( extptr, ".bsp" ) ) {
		CG_Printf( "Unable to dump, unknown map name?\n" );
		return;
	}
	Q_strncpyz( extptr, ".loc", 5 );
	trap_FS_FOpenFile( locfilename, &f, FS_APPEND_SYNC );
	if ( !f ) {
		CG_Printf( "Failed to open '%s' for writing.\n", locfilename );
		return;
	}

	// Strip bad characters out
	for ( buffptr = locname; *buffptr; buffptr++ )
	{
		if ( *buffptr == '\n' ) {
			*buffptr = ' ';
		} else if ( *buffptr == '"' ) {
			*buffptr = '\'';
		}
	}
	// Kill any trailing space as well
	if ( *( buffptr - 1 ) == ' ' ) {
		*( buffptr - 1 ) = 0;
	}

	// Build the entity definition
	buffptr = va(   "{\n\"classname\" \"target_location\"\n\"origin\" \"%i %i %i\"\n\"message\" \"%s\"\n}\n\n",
					(int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2], locname );

	// And write out/acknowledge
	trap_FS_Write( buffptr, strlen( buffptr ), f );
	trap_FS_FCloseFile( f );
	CG_Printf( "Entity dumped to '%s' (%i %i %i).\n", locfilename,
			   (int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2] );
}

static cameraPoint_t* LastAddedCameraPointPtr = NULL;
static qboolean LastAddedCameraPointSet = qfalse;

//static void CG_Chase_f(void)
//{
//	int ent;
//	int argc;
//	qboolean haveRange;
//	float range;
//	float angle;
//
//	argc = CG_Argc();
//
//	if (argc < 2) {
//		Com_Printf("usage: chase <entity number> [x offset] [y offset] [z offset] [range] [angle]\n");
//		Com_Printf("    set entity number to -1 to disable chase mode\n");
//		Com_Printf("    range can be a number, 'here' (range from entity to current view origin), or 'herez' (same as 'here' but also bases z offset on current view origin)\n");
//
//		if (cg.chaseEnt == -1) {
//			Com_Printf("\ncurrent chase entity: none\n");
//		}
//		else {
//			Com_Printf("\ncurrent chase entity: %d  ^5%f %f %f\n", cg.chaseEnt, cg.chaseEntOffsetX, cg.chaseEntOffsetY, cg.chaseEntOffsetZ);
//		}
//
//		return;
//	}
//
//	ent = atoi(CG_Argv(1));
//
//	if (ent < 0 || ent >= MAX_GENTITIES) {
//		cg.chaseEnt = -1;
//		return;
//	}
//	cg.chaseEnt = ent;
//
//	// keep previous values for cg.chaseEntOffset[XYZ]
//
//	if (argc >= 3) {
//		cg.chaseEntOffsetX = atof(CG_Argv(2));
//	}
//	if (argc >= 4) {
//		cg.chaseEntOffsetY = atof(CG_Argv(3));
//	}
//	if (argc >= 5) {
//		cg.chaseEntOffsetZ = atof(CG_Argv(4));
//	}
//
//	haveRange = qfalse;
//	range = 0;
//	angle = 0;
//
//	if (argc >= 6) {
//		if (!Q_stricmp("here", CG_Argv(5)) || !Q_stricmp("herez", CG_Argv(5))) {
//			vec3_t end;
//
//			VectorCopy(cg.refdef.vieworg, end);
//			end[2] = cg_entities[cg.chaseEnt].lerpOrigin[2];
//			range = Distance(cg_entities[cg.chaseEnt].lerpOrigin, end);
//
//			if (!Q_stricmp("herez", CG_Argv(5))) {
//				cg.chaseEntOffsetZ = cg.refdef.vieworg[2] - cg_entities[cg.chaseEnt].lerpOrigin[2];
//			}
//		}
//		else {
//			range = atof(CG_Argv(5));
//		}
//		haveRange = qtrue;
//	}
//	if (argc >= 7) {
//		angle = atof(CG_Argv(6));
//	}
//
//	if (haveRange) {
//		vec3_t dir;
//		vec3_t rotatedDir;
//		vec3_t start;
//		vec3_t end;
//		vec3_t point;
//		vec3_t up = { 0, 0, 1 };
//
//		VectorCopy(cg_entities[cg.chaseEnt].lerpOrigin, start);
//		VectorCopy(cg.refdef.vieworg, end);
//
//		end[2] = start[2];
//
//		VectorSubtract(end, start, dir);
//		VectorNormalize(dir);
//		RotatePointAroundVector(rotatedDir, up, dir, angle);
//
//		VectorMA(start, range, rotatedDir, point);
//
//		cg.chaseEntOffsetX = point[0] - start[0];
//		cg.chaseEntOffsetY = point[1] - start[1];
//		// cg.chaseEntOffsetZ already set
//	}
//
//	Com_Printf("chase %d  %f %f %f\n", cg.chaseEnt, cg.chaseEntOffsetX, cg.chaseEntOffsetY, cg.chaseEntOffsetZ);
//}
//
//static void CG_AddCameraPoint_f(void)
//{
//	cameraPoint_t* cp;
//	int i;
//	int j;
//	qboolean incrementCameraPoints;
//	qboolean lastAdded;
//	int cameraPointNum;
//
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	lastAdded = qfalse;
//	incrementCameraPoints = qtrue;
//	cp = NULL;
//	if (cg.atCameraPoint) {
//		// we will be modifying the camera point, not adding a new one
//		incrementCameraPoints = qfalse;
//		cp = &cg.cameraPoints[cg.selectedCameraPointMin];
//		cameraPointNum = cg.selectedCameraPointMin;
//		//Com_Printf("at camera point %d\n", cameraPointNum);
//	}
//	else {
//		for (i = 0; i < cg.numCameraPoints; i++) {
//			//FIXME broken, kept for paused camera point add...
//			if ((double)cg.ftime == cg.cameraPoints[i].cgtime) {
//				// we will be modifying the camera point, not adding a new one
//				incrementCameraPoints = qfalse;
//				cp = &cg.cameraPoints[i];
//				cameraPointNum = i;
//				break;
//			}
//
//			if ((double)cg.ftime < cg.cameraPoints[i].cgtime) {
//				if (cg.numCameraPoints >= MAX_CAMERAPOINTS - 3) {  // three fake ones
//					Com_Printf("too many camera points\n");
//					return;
//				}
//				for (j = cg.numCameraPoints - 1; j >= i; j--) {
//					memcpy(&cg.cameraPoints[j + 1], &cg.cameraPoints[j], sizeof(cameraPoint_t));
//				}
//				cp = &cg.cameraPoints[i];
//				cameraPointNum = i;
//				break;
//			}
//		}
//	}
//
//	if (!cp) {
//		if (cg.numCameraPoints >= MAX_CAMERAPOINTS - 3) {  // three fake ones
//			Com_Printf("too many camera points\n");
//			return;
//		}
//		cp = &cg.cameraPoints[cg.numCameraPoints];
//		cameraPointNum = cg.numCameraPoints;
//		lastAdded = qtrue;
//	}
//
//	if (cg_cameraAddUsePreviousValues.integer && cg.numCameraPoints > 0 && LastAddedCameraPointSet) {
//		*cp = *LastAddedCameraPointPtr;
//	}
//	else {
//		memset(cp, 0, sizeof(*cp));
//	}
//
//	VectorCopy(cg.refdef.vieworg, cp->origin);
//	VectorCopy(cg.refdefViewAngles, cp->angles);
//	cp->cgtime = cg.ftime;
//
//	if (!cg_cameraAddUsePreviousValues.integer || cg.numCameraPoints == 0 || LastAddedCameraPointSet == qfalse) {
//		char* type;
//
//		type = cg_cameraDefaultOriginType.string;
//		if (!Q_stricmp(type, "spline")) {
//			cp->type = CAMERA_SPLINE;
//		}
//		else if (!Q_stricmp(type, "interp")) {
//			cp->type = CAMERA_INTERP;
//		}
//		else if (!Q_stricmp(type, "jump")) {
//			cp->type = CAMERA_JUMP;
//		}
//		else if (!Q_stricmp(type, "curve")) {
//			cp->type = CAMERA_CURVE;
//		}
//		else if (!Q_stricmp(type, "splinebezier")) {
//			cp->type = CAMERA_SPLINE_BEZIER;
//		}
//		else if (!Q_stricmp(type, "splinecatmullrom")) {
//			cp->type = CAMERA_SPLINE_CATMULLROM;
//		}
//		else {
//			cp->type = CAMERA_SPLINE_BEZIER;
//		}
//		cp->viewType = CAMERA_ANGLES_SPLINE;
//		cp->rollType = CAMERA_ROLL_AS_ANGLES;
//		cp->splineType = SPLINE_FIXED;
//		cp->numSplines = DEFAULT_NUM_SPLINES;
//		cp->viewEnt = -1;
//		cp->fov = -1;
//		cp->fovType = CAMERA_FOV_USE_CURRENT;
//
//		cp->flags = CAM_ORIGIN | CAM_ANGLES | CAM_FOV;
//	}
//
//	LastAddedCameraPointSet = qtrue;
//
//	cp->useOriginVelocity = qfalse;
//	cp->useAnglesVelocity = qfalse;
//	cp->useXoffsetVelocity = qfalse;
//	cp->useYoffsetVelocity = qfalse;
//	cp->useZoffsetVelocity = qfalse;
//	cp->useFovVelocity = qfalse;
//	cp->useRollVelocity = qfalse;
//
//	if (cg.viewEnt > -1) {
//		cp->viewType = CAMERA_ANGLES_ENT;
//		cp->viewEnt = cg.viewEnt;
//		cp->offsetType = CAMERA_OFFSET_INTERP;
//		cp->xoffset = cg.viewEntOffsetX;
//		cp->yoffset = cg.viewEntOffsetY;
//		cp->zoffset = cg.viewEntOffsetZ;
//		VectorCopy(cg_entities[cg.viewEnt].lerpOrigin, cp->viewEntStartingOrigin);
//		//VectorCopy(cg_entities[cg.viewEnt].currentState.pos.trBase, cp->viewEntStartingOrigin);
//		cp->viewEntStartingOriginSet = qtrue;
//	}
//
//	if (incrementCameraPoints) {
//		cg.numCameraPoints++;
//	}
//
//	LastAddedCameraPointPtr = cp;
//
//	if (lastAdded) {
//		CG_UpdateCameraInfoExt(cg.numCameraPoints - 1);
//	}
//	else {
//		CG_UpdateCameraInfo();
//	}
//
//	cg.selectedCameraPointMin = cameraPointNum;
//	cg.selectedCameraPointMax = cameraPointNum;
//
//	//Com_Printf("add camera point selected : %d\n", cg.selectedCameraPointMin);
//}
//
//static void CG_ClearCameraPoints_f(void)
//{
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	cg.numCameraPoints = 0;
//	cg.numSplinePoints = 0;
//	cg.cameraPointsPointer = NULL;
//	cg.selectedCameraPointMin = 0;
//	cg.selectedCameraPointMax = 0;
//}
//
//static void CG_PlayCamera_f(void)
//{
//	double extraTime;
//
//	if (cg.playPath) {
//		Com_Printf("can't play camera, path is playing\n");
//		return;
//	}
//
//	if (cg.numCameraPoints < 2) {
//		Com_Printf("can't play camera, need at least 2 camera points\n");
//		return;
//	}
//
//	//FIXME why this dependency ?
//	if (cg_cameraQue.integer) {
//		extraTime = 1000.0 * cg_cameraRewindTime.value;
//		if (extraTime < 0) {
//			extraTime = 0;
//		}
//		trap_SendConsoleCommand(va("seekservertime %f\n", cg.cameraPoints[0].cgtime - extraTime));
//	}
//
//	//cg.cameraQ3mmePlaying = qfalse;
//	cg.cameraPlaying = qtrue;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	cg.currentCameraPoint = 0;
//	//cg.cameraWaitToSync = qfalse;  //FIXME stupid
//	cg.playCameraCommandIssued = qtrue;
//}
//
//static void CG_StopCamera_f(void)
//{
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//}
//
//static void CG_WriteString(const char* s, qhandle_t file)
//{
//	trap_FS_Write(s, strlen(s), file);
//}
//
//static void CG_SaveCamera_f(void)
//{
//	fileHandle_t f;
//	const char* s;
//	int i;
//	const cameraPoint_t* cp;
//	int len;
//	const char* fname;
//	qboolean useDefaultFolder = qtrue;
//
//	if (CG_Argc() < 2) {
//		Com_Printf("usage: savecamera <filename>\n");
//		return;
//	}
//
//	if (cg.numCameraPoints < 2) {
//		Com_Printf("need at least 2 camera points\n");
//		return;
//	}
//
//	fname = CG_Argv(1);
//	for (i = 0; i < strlen(fname); i++) {
//		if (fname[i] == '/') {
//			useDefaultFolder = qfalse;
//			break;
//		}
//	}
//
//	if (useDefaultFolder) {
//		trap_FS_FOpenFile(va("cameras/%s.cam%d", CG_Argv(1), WOLFCAM_CAMERA_VERSION), &f, FS_WRITE);
//	}
//	else {
//		trap_FS_FOpenFile(va("%s.cam%d", fname, WOLFCAM_CAMERA_VERSION), &f, FS_WRITE);
//	}
//
//	if (!f) {
//		Com_Printf("^1couldn't create %s.cam%d\n", CG_Argv(1), WOLFCAM_CAMERA_VERSION);
//		return;
//	}
//	s = va("WolfcamCamera %d\n", WOLFCAM_CAMERA_VERSION);
//	trap_FS_Write(s, strlen(s), f);
//
//	for (i = 0; i < cg.numCameraPoints; i++) {
//		cp = &cg.cameraPoints[i];
//
//		CG_WriteString(va("%d  camera point number\n", i), f);
//		CG_WriteString(va("%f %f %f  origin\n", cp->origin[0], cp->origin[1], cp->origin[2]), f);
//		CG_WriteString(va("%f %f %f  angles\n", cp->angles[0], cp->angles[1], cp->angles[2]), f);
//		CG_WriteString(va("%d  type\n", cp->type), f);
//		CG_WriteString(va("%d  viewType\n", cp->viewType), f);
//		CG_WriteString(va("%d  rollType\n", cp->rollType), f);
//		CG_WriteString(va("%d  flags\n", cp->flags), f);
//		CG_WriteString(va("%f  cgtime\n", cp->cgtime), f);
//		CG_WriteString(va("%d  splineType\n", cp->splineType), f);
//		CG_WriteString(va("%d  numSplines\n", cp->numSplines), f);
//
//		CG_WriteString(va("%f %f %f  viewPointOrigin\n", cp->viewPointOrigin[0], cp->viewPointOrigin[1], cp->viewPointOrigin[2]), f);
//		CG_WriteString(va("%d  viewPointOriginSet\n", cp->viewPointOriginSet), f);
//		CG_WriteString(va("%d  viewEnt\n", cp->viewEnt), f);
//		CG_WriteString(va("%f %f %f  viewEntStartingOrigin\n", cp->viewEntStartingOrigin[0], cp->viewEntStartingOrigin[1], cp->viewEntStartingOrigin[2]), f);
//		CG_WriteString(va("%d  viewEntStartingOriginSet\n", cp->viewEntStartingOriginSet), f);
//		CG_WriteString(va("%d  offsetType\n", cp->offsetType), f);
//		CG_WriteString(va("%f  xoffset\n", cp->xoffset), f);
//		CG_WriteString(va("%f  yoffset\n", cp->yoffset), f);
//		CG_WriteString(va("%f  zoffset\n", cp->zoffset), f);
//
//		CG_WriteString(va("%f  fov\n", cp->fov), f);
//		CG_WriteString(va("%d  fovType\n", cp->fovType), f);
//
//		CG_WriteString(va("%d  useOriginVelocity\n", cp->useOriginVelocity), f);
//		CG_WriteString(va("%f  originInitialVelocity\n", cp->originInitialVelocity), f);
//		CG_WriteString(va("%f  originFinalVelocity\n", cp->originFinalVelocity), f);
//
//		CG_WriteString(va("%d  useAnglesVelocity\n", cp->useAnglesVelocity), f);
//		CG_WriteString(va("%f  anglesInitialVelocity\n", cp->anglesInitialVelocity), f);
//		CG_WriteString(va("%f  anglesFinalVelocity\n", cp->anglesFinalVelocity), f);
//
//		CG_WriteString(va("%d  useXoffsetVelocity\n", cp->useXoffsetVelocity), f);
//		CG_WriteString(va("%f  xoffsetInitialVelocity\n", cp->xoffsetInitialVelocity), f);
//		CG_WriteString(va("%f  xoffsetFinalVelocity\n", cp->xoffsetFinalVelocity), f);
//
//		CG_WriteString(va("%d  useYoffsetVelocity\n", cp->useYoffsetVelocity), f);
//		CG_WriteString(va("%f  yoffsetInitialVelocity\n", cp->yoffsetInitialVelocity), f);
//		CG_WriteString(va("%f  yoffsetFinalVelocity\n", cp->yoffsetFinalVelocity), f);
//
//		CG_WriteString(va("%d  useZoffsetVelocity\n", cp->useZoffsetVelocity), f);
//		CG_WriteString(va("%f  zoffsetInitialVelocity\n", cp->zoffsetInitialVelocity), f);
//		CG_WriteString(va("%f  zoffsetFinalVelocity\n", cp->zoffsetFinalVelocity), f);
//
//		CG_WriteString(va("%d  useFovVelocity\n", cp->useFovVelocity), f);
//		CG_WriteString(va("%f  fovInitialVelocity\n", cp->fovInitialVelocity), f);
//		CG_WriteString(va("%f  fovFinalVelocity\n", cp->fovFinalVelocity), f);
//
//		CG_WriteString(va("%d  useRollVelocity\n", cp->useRollVelocity), f);
//		CG_WriteString(va("%f  rollInitialVelocity\n", cp->rollInitialVelocity), f);
//		CG_WriteString(va("%f  rollFinalVelocity\n", cp->rollFinalVelocity), f);
//		len = strlen(cp->command);
//		CG_WriteString(va("%d  commandStrLen\n", len), f);
//		if (len) {
//			CG_WriteString(cp->command, f);
//		}
//
//		CG_WriteString("\n-------------------------------------\n", f);
//	}
//	trap_FS_FCloseFile(f);
//	//Com_Printf("camera saved\n");
//}
//
//static void CG_CamtraceSave_f(void)
//{
//	int i;
//	const cameraPoint_t* cp;
//	char buffer[MAX_STRING_CHARS];
//	qboolean useOldFormat;
//
//	if (cg.numCameraPoints < 2) {
//		Com_Printf("need at least 2 camera points\n");
//		return;
//	}
//
//	useOldFormat = qfalse;
//	if (CG_Argc() >= 2) {
//		if (!Q_stricmp("old", CG_Argv(1))) {
//			useOldFormat = qtrue;
//			//Com_Printf("old\n");
//		}
//	}
//
//
//	// etqw
//	// ]freecamgetpos
//	// Copying output to clipboard...
//	// (x y z) pitch yaw roll time
//	// (0 0 0) 0.00 0.00 0.00 137742
//	// TODO: Sys_SetClipboardData
//
//	// old format:
//	// "clear; viewpos; cg_fov; condump Cam\Pos\pos01.epcp; Echo Cam01;"
//	//
//	// (2838 7449 -27) : -83
//	// "cg_fov" is:"102" default:"90"
//	// Dumped console text to Cam\Pos\pos01.epcp.
//
//	for (i = 0; i < cg.numCameraPoints; i++) {
//		fileHandle_t f;
//		int fov;
//
//		if (cgs.realProtocol >= 91 && cg_useDemoFov.integer == 1) {
//			fov = cg.demoFov;
//		}
//		else {
//			fov = cg_fov.integer;
//		}
//
//		cp = &cg.cameraPoints[i];
//		//Com_Printf ("(%i %i w%i) : %i\n", (int)cp->origin[0], (int)cp->origin[1], (int)cp->origin[2], (int)cp->angles[YAW]);
//
//		if (useOldFormat) {
//			trap_FS_FOpenFile(va("cameras/ct3d/pos/pos%03d.epcp", i + 1), &f, FS_WRITE);
//		}
//		else {
//			trap_FS_FOpenFile(va("cameras/ct3d/pos/pos%03d.qwcp", i + 1), &f, FS_WRITE);
//		}
//		if (!f) {
//			Com_Printf("^1couldn't open file for camera position %d\n", i + 1);
//			break;
//		}
//		//Com_sprintf(buffer, sizeof(buffer), "(%i %i %i) : %i\n\"cg_fov\" is:\"%d\" default:\"90\"\nDumped console text to cameras/ct3d/pos/pos%d03d.epcp.\n\n", (int)cp->origin[0], (int)cp->origin[1], (int)cp->origin[2], (int)cp->angles[YAW], (int)cp->fov, i);
//		if (useOldFormat) {
//			Com_sprintf(buffer, sizeof(buffer), "(%i %i %i) : %i\n\"cg_fov\" is:\"%d\" default:\"90\"\nDumped console text to cameras/ct3d/pos/pos%03d.epcp.\n\n", (int)cp->origin[0], (int)cp->origin[1], (int)cp->origin[2], (int)cp->angles[YAW], cp->fov == -1 ? fov : (int)cp->fov, i);
//		}
//		else {
//			Com_sprintf(buffer, sizeof(buffer), "Copying output to clipboard...\n(x y z) pitch yaw roll time\n(%f %f %f) %f %f %f %i\n\"g_fov\" is:\"%d\" default:\"90\"\nDumped console text to cameras/ct3d/pos/pos%03d.qwcp.\n\n", cp->origin[0], cp->origin[1], cp->origin[2], cp->angles[PITCH], cp->angles[YAW], cp->angles[ROLL], cg.time, cp->fov == -1 ? fov : (int)cp->fov, i);
//		}
//		trap_FS_Write(buffer, strlen(buffer), f);
//		trap_FS_FCloseFile(f);
//	}
//}
//
//static void CG_LoadCamera_f(void)
//{
//	fileHandle_t f;
//	int len;
//	int version;
//	const char* s;
//	cameraPoint_t* cp;
//	int slen;
//	qboolean useDefaultFolder = qtrue;
//	const char* fname;
//	int i;
//
//	if (CG_Argc() < 2) {
//		Com_Printf("usage: loadcamera <camera name>\n");
//		return;
//	}
//
//	fname = CG_Argv(1);
//	for (i = 0; i < strlen(fname); i++) {
//		if (fname[i] == '/') {
//			useDefaultFolder = qfalse;
//			break;
//		}
//	}
//
//	if (useDefaultFolder) {
//		trap_FS_FOpenFile(va("cameras/%s.cam%d", CG_Argv(1), WOLFCAM_CAMERA_VERSION), &f, FS_READ);
//		if (!f) {
//			trap_FS_FOpenFile(va("cameras/%s.cam%d", CG_Argv(1), WOLFCAM_CAMERA_VERSION - 1), &f, FS_READ);
//		}
//	}
//	else {
//		trap_FS_FOpenFile(va("%s.cam%d", fname, WOLFCAM_CAMERA_VERSION), &f, FS_READ);
//		if (!f) {
//			trap_FS_FOpenFile(va("%s.cam%d", fname, WOLFCAM_CAMERA_VERSION - 1), &f, FS_READ);
//		}
//	}
//
//	if (!f) {
//		Com_Printf("^1couldn't open %s\n", CG_Argv(1));
//		return;
//	}
//
//	cg.numCameraPoints = 0;
//	cg.numSplinePoints = 0;
//
//	s = CG_FS_ReadLine(f, &len);
//	//FIXME check name
//	sscanf(s + strlen("WolfcamCamera "), "%d", &version);
//
//	while (len) {
//		cp = &cg.cameraPoints[cg.numCameraPoints];
//		memset(cp, 0, sizeof(*cp));
//
//		s = CG_FS_ReadLine(f, &len);  // camera point number
//		if (len) {
//			//Com_Printf(va("%s", s));
//		}
//		else {
//			//  all done
//			break;
//		}
//
//		cp->version = version;
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%f %f %f", &cp->origin[0], &cp->origin[1], &cp->origin[2]);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%f %f %f", &cp->angles[0], &cp->angles[1], &cp->angles[2]);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->type);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->viewType);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->rollType);
//
//		if (version > 8) {
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", &cp->flags);
//		}
//		else {
//			cp->flags = CAM_ORIGIN | CAM_ANGLES | CAM_FOV;
//		}
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%lf", &cp->cgtime);
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->splineType);
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->numSplines);
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%f %f %f", &cp->viewPointOrigin[0], &cp->viewPointOrigin[1], &cp->viewPointOrigin[2]);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", (int*)&cp->viewPointOriginSet);
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->viewEnt);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%f %f %f", &cp->viewEntStartingOrigin[0], &cp->viewEntStartingOrigin[1], &cp->viewEntStartingOrigin[2]);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", (int*)&cp->viewEntStartingOriginSet);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->offsetType);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%lf", &cp->xoffset);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%lf", &cp->yoffset);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%lf", &cp->zoffset);
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%lf", &cp->fov);
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &cp->fovType);
//
//		if (version < 10) {
//			// never used
//			s = CG_FS_ReadLine(f, &len);  // cp->timescale
//			s = CG_FS_ReadLine(f, &len);  // cp->timescaleInterp
//		}
//
//		if (version > 7) {
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useOriginVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->originInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->originFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useAnglesVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->anglesInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->anglesFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useXoffsetVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->xoffsetInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->xoffsetFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useYoffsetVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->yoffsetInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->yoffsetFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useZoffsetVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->zoffsetInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->zoffsetFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useFovVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->fovInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->fovFinalVelocity);
//
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%d", (int*)&cp->useRollVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->rollInitialVelocity);
//			s = CG_FS_ReadLine(f, &len);
//			sscanf(s, "%lf", &cp->rollFinalVelocity);
//		}
//
//		s = CG_FS_ReadLine(f, &len);
//		sscanf(s, "%d", &slen);
//		if (slen) {
//			trap_FS_Read(cp->command, slen, f);
//		}
//
//		if (!len) {
//			Com_Printf("^1ERROR  corrupt camera file\n");
//			cg.numCameraPoints = 0;
//			break;
//		}
//
//		s = CG_FS_ReadLine(f, &len);  // \n
//		s = CG_FS_ReadLine(f, &len);  // -------------------------\n
//
//		cg.numCameraPoints++;
//	}
//	trap_FS_FCloseFile(f);
//
//	CG_UpdateCameraInfo();
//	cg.selectedCameraPointMin = 0;
//	cg.selectedCameraPointMax = 0;
//	Com_Printf("camera loaded (version %d)\n", version);
//}
//
//static void CG_SelectCameraPoint_f(void)
//{
//	int n;
//	const char* s;
//
//	if (CG_Argc() < 2) {
//		Com_Printf("usage: selectcamerapoint <point 1> [point 2]\n");
//		Com_Printf("       point 1  is a number or it can be [all, first, last, inner]\n");
//		return;
//	}
//
//	s = CG_Argv(1);
//	if (!Q_stricmp(s, "all")) {
//		cg.selectedCameraPointMin = 0;
//		cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//		return;
//	}
//	else if (!Q_stricmp(s, "first")) {
//		if (CG_Argc() >= 3) {
//			cg.selectedCameraPointMin = 0;
//		}
//		else {
//			cg.selectedCameraPointMin = 0;
//			cg.selectedCameraPointMax = 0;
//			return;
//		}
//	}
//	else if (!Q_stricmp(s, "last")) {
//		cg.selectedCameraPointMin = cg.numCameraPoints - 1;
//		cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//		return;
//	}
//	else if (!Q_stricmp(s, "inner")) {
//		if (cg.numCameraPoints < 3) {
//			return;
//		}
//		cg.selectedCameraPointMin = 1;
//		cg.selectedCameraPointMax = cg.numCameraPoints - 2;
//	}
//	else {
//		n = atoi(CG_Argv(1));
//		if (n >= cg.numCameraPoints || n < 0) {
//			Com_Printf("invalid camera point\n");
//			cg.selectedCameraPointMin = cg.numCameraPoints - 1;
//			cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//			return;
//		}
//		cg.selectedCameraPointMin = n;
//		cg.selectedCameraPointMax = n;
//	}
//
//	if (CG_Argc() >= 3) {
//		if (!Q_stricmp(CG_Argv(2), "last")) {
//			cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//		}
//		else {
//			n = atoi(CG_Argv(2));
//			if (n >= cg.numCameraPoints || n < 0) {
//				Com_Printf("invalid camera point\n");
//				cg.selectedCameraPointMin = cg.numCameraPoints - 1;
//				cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//				return;
//			}
//			cg.selectedCameraPointMax = n;
//		}
//	}
//
//	if (cg.selectedCameraPointMin > cg.selectedCameraPointMax) {
//		Com_Printf("invalid camera points\n");
//		cg.selectedCameraPointMin = cg.numCameraPoints - 1;
//		cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//		return;
//	}
//}
//
//static void CG_EditCameraPoint_f(void)
//{
//	qboolean gotoRealPoint = qtrue;
//	int cameraPoint = cg.numCameraPoints - 1;
//	const char* s;
//
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	if (CG_Argc() < 2) {
//		cameraPoint = cg.selectedCameraPointMin;
//	}
//	else {
//		s = CG_Argv(1);
//		if (!Q_stricmp(s, "next")) {
//			cameraPoint = cg.selectedCameraPointMin + 1;
//			if (cameraPoint >= cg.numCameraPoints) {
//				cameraPoint = 0;
//			}
//		}
//		else if (!Q_stricmp(s, "previous")) {
//			cameraPoint = cg.selectedCameraPointMin - 1;
//			if (cameraPoint < 0) {
//				cameraPoint = cg.numCameraPoints - 1;
//			}
//		}
//		else {
//			cameraPoint = atoi(CG_Argv(1));
//		}
//	}
//
//	if (CG_Argc() >= 3) {
//		if (!Q_stricmp(CG_Argv(2), "real")) {
//			gotoRealPoint = qfalse;
//		}
//	}
//
//	if (cameraPoint < 0 || cameraPoint >= cg.numCameraPoints) {
//		Com_Printf("invalid camera point\n");
//		return;
//	}
//
//	if (cg.numCameraPoints < 2 || (cg.cameraPoints[cameraPoint].type != CAMERA_SPLINE || cg.cameraPoints[cameraPoint].type != CAMERA_SPLINE_BEZIER || cg.cameraPoints[cameraPoint].type != CAMERA_SPLINE_CATMULLROM)) {
//		gotoRealPoint = qfalse;
//	}
//
//	//FIXME
//	//cg.editingCameraPoint = qtrue;
//	cg.selectedCameraPointMin = cameraPoint;
//	cg.selectedCameraPointMax = cameraPoint;
//
//	if (gotoRealPoint) {
//		if (cg.cameraPoints[cameraPoint].type == CAMERA_SPLINE) {
//			VectorCopy(cg.splinePoints[cg.cameraPoints[cameraPoint].splineStart], cg.freecamPlayerState.origin);
//		}
//		else if (cg.cameraPoints[cameraPoint].type == CAMERA_SPLINE_BEZIER) {
//			CG_CameraSplineOriginAt(cg.cameraPoints[cameraPoint].cgtime, posBezier, cg.freecamPlayerState.origin);
//		}
//		else if (cg.cameraPoints[cameraPoint].type == CAMERA_SPLINE_CATMULLROM) {
//			CG_CameraSplineOriginAt(cg.cameraPoints[cameraPoint].cgtime, posCatmullRom, cg.freecamPlayerState.origin);
//		}
//		else {
//			VectorCopy(cg.cameraPoints[cameraPoint].origin, cg.freecamPlayerState.origin);
//		}
//		VectorCopy(cg.freecamPlayerState.origin, cg.fpos);
//
//		if (cg.cameraPoints[cameraPoint].viewType == CAMERA_ANGLES_SPLINE) {
//			CG_CameraSplineAnglesAt(cg.cameraPoints[cameraPoint].cgtime, cg.freecamPlayerState.viewangles);
//		}
//		else {
//			VectorCopy(cg.cameraPoints[cameraPoint].angles, cg.freecamPlayerState.viewangles);
//		}
//		VectorCopy(cg.freecamPlayerState.viewangles, cg.fang);
//	}
//	else {  // goto set values
//		VectorCopy(cg.cameraPoints[cameraPoint].origin, cg.freecamPlayerState.origin);
//		VectorCopy(cg.freecamPlayerState.origin, cg.fpos);
//		VectorCopy(cg.cameraPoints[cameraPoint].angles, cg.freecamPlayerState.viewangles);
//		VectorCopy(cg.freecamPlayerState.viewangles, cg.fang);
//	}
//	cg.freecamPlayerState.origin[2] -= DEFAULT_VIEWHEIGHT;
//	cg.fpos[2] -= DEFAULT_VIEWHEIGHT;
//	VectorSet(cg.freecamPlayerState.velocity, 0, 0, 0);
//	//Com_Printf("^3camera time: %f\n", cg.cameraPoints[cameraPoint].cgtime);
//	trap_Cvar_Set("cl_freezeDemo", "1");
//	//trap_SendConsoleCommand(va("seekservertime %f\n", cg.cameraPoints[cameraPoint].cgtime));
//	trap_SendConsoleCommandNow(va("seekservertime %f\n", cg.cameraPoints[cameraPoint].cgtime));
//	//FIXME bad hack
//	cg.atCameraPoint = qtrue;
//}
//
//static void CG_DeleteCameraPoint_f(void)
//{
//	//int n;
//	int i;
//
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	if (!cg.numCameraPoints) {
//		return;
//	}
//
//	for (i = 0; i < (cg.numCameraPoints - 1) - cg.selectedCameraPointMax; i++) {
//		Com_Printf("%d -> %d\n", cg.selectedCameraPointMax + 1 + i, cg.selectedCameraPointMin + i);
//		memcpy(&cg.cameraPoints[cg.selectedCameraPointMin + i], &cg.cameraPoints[cg.selectedCameraPointMax + 1 + i], sizeof(cameraPoint_t));
//	}
//
//	cg.numCameraPoints = cg.selectedCameraPointMin + (cg.numCameraPoints - 1) - cg.selectedCameraPointMax;
//
//	cg.selectedCameraPointMin = cg.numCameraPoints - 1;
//	cg.selectedCameraPointMax = cg.numCameraPoints - 1;
//
//	CG_UpdateCameraInfo();
//
//	return;
//}
//
//#if 0
////static void FindQuadratic (double x0, double y0, double x1, double y1, double x2, double y2, double *a, double *b, double *c)
//static void FindQuadratic(long double x0, long double y0, long double x1, long double y1, long double x2, long double y2, long double* a, long double* b, long double* c)
//{
//	//double e, f;
//	long double e, f;
//
//	Com_Printf("x0 %LF y0 %LF x1 %LF y1 %LF x2 %LF y2 %LF\n", x0, y0, x1, y1, x2, y2);
//
//	e = x0 - x1;
//	f = x0 - x2;
//
//	*a = (f * (y0 - y1) - e * (y0 - y2)) / (f * (x0 * x0 - x1 * x1) - e * (x0 * x0 - x2 * x2));
//
//	*b = ((y0 - y1) - (long double)*a * (x0 * x0 - x1 * x1)) / (x0 - x1);
//
//	*c = y0 - (long double)*a * x0 * x0 - (long double)*b * x0;
//
//	Com_Printf("^3quadratic: %LFx**2  +  %LFx  +  %LF\n", *a, *b, *c);
//	//Com_Printf("^3quadratic: %LFx**2  +  %LFx  +  %\n", *a, *b, *c);
//}
//#endif
//
//static void FindQuadratic(long double x0, long double y0, long double x1, long double y1, long double x2, long double y2, long double* a, long double* b, long double* c)
//{
//	long double e, f;
//
//	//Com_Printf("x0 %LF y0 %LF x1 %LF y1 %LF x2 %LF y2 %LF\n", x0, y0, x1, y1, x2, y2);
//
//	e = x0 - x1;
//	f = x0 - x2;
//
//	*a = (f * (y0 - y1) - e * (y0 - y2)) / (f * (x0 * x0 - x1 * x1) - e * (x0 * x0 - x2 * x2));
//
//	*b = ((y0 - y1) - (long double)(*a) * (x0 * x0 - x1 * x1)) / (x0 - x1);
//
//	*c = y0 - (long double)(*a) * x0 * x0 - (long double)(*b) * x0;
//
//	//Com_Printf("^3quadratic: %LFx**2  +  %LFx  +  %LF\n", *a, *b, *c);
//	//Com_Printf("^3quadratic: %LFx**2  +  %LFx  +  %\n", *a, *b, *c);
//}
//
//static double CameraCurveDistance(const cameraPoint_t* cp, const cameraPoint_t* cpnext)
//{
//	long double dist;
//	int i;
//	vec3_t origin;
//	vec3_t newOrigin;
//	int samples;
//	long double t;
//	int j;
//
//	VectorSet(origin, 0, 0, 0);  // silence gcc warning
//	dist = 0;
//	samples = 500;
//
//	for (i = 0; i < samples; i++) {
//		t = ((cpnext->cgtime - cp->quadraticStartTime) - (cp->cgtime - cp->quadraticStartTime)) / (long double)samples * (long double)i;
//
//		//t += cp->cgtime;
//		t /= 1000.0;
//
//		//Com_Printf("t %LF   start %f  %f\n", t, cp->quadraticStartTime, cp->cgtime);
//
//		for (j = 0; j < 3; j++) {
//			newOrigin[j] = cp->a[j] * t * t + cp->b[j] * t + cp->c[j];
//		}
//
//		if (i > 0) {
//			dist += Distance(newOrigin, origin);
//		}
//
//		VectorCopy(newOrigin, origin);
//	}
//
//	return dist;
//}
//
//static void CG_UpdateCameraInfoExt(int startUpdatePoint)
//{
//	int i;
//	float tension;
//	float granularity;
//	float x, y, z;
//	int j;
//	vec3_t bpoint0, bpoint1, bpoint2;
//	vec3_t point0, point1, point2;
//	int k;
//	int start;
//	qboolean midPointHit;
//	vec3_t dir;
//	double dist;
//	cameraPoint_t* cp;
//	cameraPoint_t* cpReal;
//	cameraPoint_t* cpnext;
//	const cameraPoint_t* cptmp;
//	const cameraPoint_t* cpprev;
//	const cameraPoint_t* first;
//	const cameraPoint_t* second;
//	const cameraPoint_t* last;
//	int count;
//	int passStart;
//	int passEnd;
//	//int realStart;
//	int curveCount;
//	vec3_t a0, a1;
//	double avg;
//	int ourUpdateStartPoint;
//
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	if (cg.numCameraPoints < 2) {
//		// update pointers for q3mme camera functions
//		if (cg.numCameraPoints == 0) {
//			cg.cameraPointsPointer = NULL;
//		}
//		else {  // 1
//			cg.cameraPointsPointer = &cg.cameraPoints[0];
//			cg.cameraPoints[0].prev = NULL;
//			cg.cameraPoints[0].next = NULL;
//			cg.cameraPoints[0].len = -1;
//		}
//
//		return;
//	}
//
//	// there's at least 2 camera points now
//	ourUpdateStartPoint = startUpdatePoint - 3;
//	if (ourUpdateStartPoint < 0) {
//		ourUpdateStartPoint = 0;
//	}
//	else if (ourUpdateStartPoint >= cg.numCameraPoints) {
//		Com_Printf("^2ERROR: invalid start camera point give for update camera info (%d >= %d)\n", ourUpdateStartPoint, cg.numCameraPoints);
//		return;
//	}
//
//	// update camera pointers for q3mme camera functions, needs to be done
//	// early since this functions calls q3mme cam functions
//	for (i = ourUpdateStartPoint; i < cg.numCameraPoints; i++) {
//		if (i == 0) {
//			cg.cameraPoints[i].prev = NULL;
//			cg.cameraPoints[i].next = &cg.cameraPoints[i + 1];
//		}
//		else if (i == cg.numCameraPoints - 1) {
//			cg.cameraPoints[i].prev = &cg.cameraPoints[i - 1];
//			cg.cameraPoints[i].next = NULL;
//		}
//		else {
//			cg.cameraPoints[i].prev = &cg.cameraPoints[i - 1];
//			cg.cameraPoints[i].next = &cg.cameraPoints[i + 1];
//		}
//
//		cg.cameraPoints[i].len = -1;
//	}
//	cg.cameraPointsPointer = &cg.cameraPoints[0];
//
//	// calculate spline points
//
//	cg.numSplinePoints = 0;
//	for (i = 0; i < ourUpdateStartPoint; i++) {
//		cameraPoint_t* cp;
//
//		cp = &cg.cameraPoints[i];
//		if (!(cp->flags & CAM_ORIGIN)) {
//			continue;
//		}
//		cg.numSplinePoints += cp->numSplines;
//	}
//
//	granularity = 0.025;  //FIXME cvar
//
//	first = cg.cameraPointsPointer;
//	//first = &cg.cameraPoints[ourUpdateStartPoint];
//
//	while (first && !(first->flags & CAM_ORIGIN)) {
//		first = first->next;
//	}
//
//	second = NULL;
//	if (first) {
//		second = first->next;
//		while (second && !(second->flags & CAM_ORIGIN)) {
//			second = second->next;
//		}
//	}
//
//	last = &cg.cameraPoints[cg.numCameraPoints - 1];
//	while (last && !(last->flags & CAM_ORIGIN)) {
//		last = last->prev;
//	}
//
//	if (!first || !second || !last) {
//		// nothing to calculate
//		goto splinesDone;
//	}
//
//	VectorCopy(first->origin, bpoint0);
//	VectorCopy(first->origin, bpoint1);
//	VectorCopy(first->origin, bpoint2);
//
//	VectorSubtract(first->origin, second->origin, dir);
//	dist = Distance(second->origin, first->origin);
//	//Com_Printf("beg dist %f\n", dist);
//	VectorNormalize(dir);
//
//	// hack to keep spline point 0 with camera point 0
//	VectorMA(bpoint0, (float)dist * 1.0 * 3.0, dir, bpoint0);
//	VectorMA(bpoint1, (float)dist * 0.66 * 3.0, dir, bpoint1);
//	VectorMA(bpoint2, (float)dist * 0.33 * 3.0, dir, bpoint2);
//
//	VectorCopy(last->origin, point0);
//	VectorCopy(last->origin, point1);
//	VectorCopy(last->origin, point2);
//
//	VectorCopy(point0, cg.cameraPoints[cg.numCameraPoints + 0].origin);
//	VectorCopy(point1, cg.cameraPoints[cg.numCameraPoints + 1].origin);
//	VectorCopy(point2, cg.cameraPoints[cg.numCameraPoints + 2].origin);
//
//	for (i = 0; i < 3; i++) {
//		cg.cameraPoints[cg.numCameraPoints + i].numSplines = last->numSplines;
//		cg.cameraPoints[cg.numCameraPoints + i].flags = CAM_ORIGIN;
//
//		//FIXME bad hack
//		if (i == 0) {
//			cg.cameraPoints[cg.numCameraPoints + i].prev = &cg.cameraPoints[cg.numCameraPoints - 1];
//		}
//		else {
//			cg.cameraPoints[cg.numCameraPoints + i].prev = &cg.cameraPoints[cg.numCameraPoints + i - 1];
//		}
//	}
//
//	//for (i = 0;  i < cg.numCameraPoints + 3;  i++) {
//	for (i = ourUpdateStartPoint; i < cg.numCameraPoints + 3; i++) {
//		cp = &cg.cameraPoints[i];
//
//		// some clean up
//		cp->viewPointPassStart = -1;
//		cp->viewPointPassEnd = -1;
//		cp->rollPassStart = -1;
//		cp->rollPassEnd = -1;
//		cp->fovPassStart = -1;
//		cp->fovPassEnd = -1;
//		cp->offsetPassStart = -1;
//		cp->offsetPassEnd = -1;
//
//		if (!(cp->flags & CAM_ORIGIN)) {
//			continue;
//		}
//
//		//Com_Printf("camera point (%d):  %f %f %f\n", i, cg.cameraPoints[i].origin[0], cg.cameraPoints[i].origin[1], cg.cameraPoints[i].origin[2]);
//
//		// get valid origin cam point i - 2
//
//		cpReal = cp->prev;
//		while (cpReal && !(cpReal->flags & CAM_ORIGIN)) {
//			cpReal = cpReal->prev;
//		}
//		if (cpReal) {
//			cpReal = cpReal->prev;
//			while (cpReal && !(cpReal->flags & CAM_ORIGIN)) {
//				cpReal = cpReal->prev;
//			}
//		}
//
//		if (cpReal) {
//			cpReal->splineStart = cg.numSplinePoints;
//			//Com_Printf("new  %p\n", cpReal);
//		}
//
//		start = cg.numSplinePoints;
//
//		granularity = 1.0 / (float)cg.cameraPoints[i].numSplines;
//
//		midPointHit = qfalse;
//
//		for (tension = 0.0; tension < 0.999 /*1.001*/; tension += granularity) {
//			x = y = z = 0;
//			for (j = 0; j < 4; j++) {
//				vec3_t origin;
//
//				k = i - (3 - j);
//
//				if (k == -3) {
//					VectorCopy(bpoint0, origin);
//				}
//				else if (k == -2) {
//					VectorCopy(bpoint1, origin);
//				}
//				else if (k == -1) {
//					VectorCopy(bpoint2, origin);
//				}
//				else {
//					int n;
//
//					cpReal = cp;
//					while (cpReal && !(cpReal->flags & CAM_ORIGIN)) {
//						Com_Printf("^1tension spline shouldn't happen...\n");
//						cpReal = cpReal->prev;
//					}
//					n = 3 - j;
//					while (n > 0) {
//						if (cpReal) {
//							cpReal = cpReal->prev;
//							while (cpReal && !(cpReal->flags & CAM_ORIGIN)) {
//								cpReal = cpReal->prev;
//							}
//						}
//						n--;
//					}
//
//					// no, use flags/mask
//					//VectorCopy(cg.cameraPoints[k].origin, origin);
//
//					if (!cpReal) {
//						//Com_Printf("^1cpReal not found j == %d, i == %d\n", j, i);
//						VectorCopy(bpoint0, origin);
//					}
//					else {
//						VectorCopy(cpReal->origin, origin);
//					}
//				}
//
//				x += origin[0] * CG_CalcSpline(j, tension);
//				y += origin[1] * CG_CalcSpline(j, tension);
//				z += origin[2] * CG_CalcSpline(j, tension);
//			}
//			VectorSet(cg.splinePoints[cg.numSplinePoints], x, y, z);
//			cg.splinePointsCameraPoints[cg.numSplinePoints] = i;
//			//cg.splinePointsCameraPoints[cg.numSplinePoints] = i - 2;
//			cg.numSplinePoints++;
//			if (tension > 0.49) {
//				if (!midPointHit) {
//					//cg.cameraPoints[i - 2].splineStart = cg.numSplinePoints;
//				}
//				midPointHit = qtrue;
//			}
//			//Com_Printf("%d  (%d) %f %f %f\n", i, cg.numSplinePoints - 1, x, y, z);
//			//Com_Printf("    pt %f %f %f\n", x, y, z);
//			if (cg.numSplinePoints >= MAX_SPLINEPOINTS) {
//				Com_Printf("^1ERROR cg.numSplinePoints >= MAX_SPLINEPOINTS\n");
//				//return;
//				goto splinesDone;
//			}
//		}
//		if (SC_Cvar_Get_Int("debug_splines")) {
//			if (i < cg.numCameraPoints) {
//				Com_Printf("cam point %d  %d splines  granularity %f\n", i, cg.numSplinePoints - start, granularity);
//			}
//		}
//	}
//
//	// other spline types
//	//FIXME camera curve done after everything else
//
//	//cp = cg.cameraPointsPointer;
//
//	//for (cp = cg.cameraPointsPointer;  cp != NULL;  cp = cp->next) {
//	for (cp = &cg.cameraPoints[ourUpdateStartPoint]; cp != NULL; cp = cp->next) {
//		const cameraPoint_t* next;
//		int numSplinePoints;
//		double totalTime;
//		double timeSlice;
//		posInterpolate_t posType;
//
//		if (!(cp->flags & CAM_ORIGIN)) {
//			continue;
//		}
//		if (cp->type != CAMERA_SPLINE_BEZIER && cp->type != CAMERA_SPLINE_CATMULLROM) {
//			continue;
//		}
//		if (cp->type == CAMERA_SPLINE_BEZIER) {
//			posType = posBezier;
//		}
//		else if (cp->type == CAMERA_SPLINE_CATMULLROM) {
//			posType = posCatmullRom;
//		}
//		else {
//			posType = posBezier;
//			Com_Printf("^1alt spline invalid type %d\n", cp->type);
//		}
//
//		next = cp->next;
//		while (next && !(next->flags & CAM_ORIGIN)) {
//			next = next->next;
//		}
//		if (!next) {
//			CG_CameraSplineOriginAt(cp->cgtime, posType, cg.splinePoints[cp->splineStart]);
//			break;
//		}
//
//
//		numSplinePoints = next->splineStart - cp->splineStart;
//		totalTime = next->cgtime - cp->cgtime;
//		timeSlice = totalTime / (double)numSplinePoints;
//
//		for (i = 0; i < numSplinePoints; i++) {
//			CG_CameraSplineOriginAt(cp->cgtime + (timeSlice * i), posType, cg.splinePoints[cp->splineStart + i]);
//			//Com_Printf("new spline %p  %d  (%f %f %f)\n", cp, i, cg.splinePoints[cp->splineStart + i][0], cg.splinePoints[cp->splineStart + i][1], cg.splinePoints[cp->splineStart + i][2]);
//		}
//	}
//
//splinesDone:
//	// ugh.. this was already set
//	//cg.cameraPoints[cg.numCameraPoints - 1].splineStart = cg.numSplinePoints - 1;
//
//	if (SC_Cvar_Get_Int("debug_splines")) {
//		Com_Printf("UpdateCameraInfo(): CreateSplines  %d spline points\n", cg.numSplinePoints);
//	}
//
//	// origin initial velocities and quadratic calculation
//
//	curveCount = 0;
//
//	for (i = 0; i < cg.numCameraPoints - 1; i++) {
//		const cameraPoint_t* cpnextnext, * cpprevprev;  // for curve type
//
//		cp = &cg.cameraPoints[i];
//		if (!(cp->flags & CAM_ORIGIN)) {
//			continue;
//		}
//		cpnext = cp->next;
//		//Com_Printf("next:  %p\n", cpnext);
//		while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//			cpnext = cpnext->next;
//		}
//		if (!cpnext) {
//			//Com_Printf("^1%d / %d  next null\n", i, cg.numCameraPoints);
//			break;
//		}
//
//		cpnextnext = NULL;
//		if (cpnext->next) {
//			cpnextnext = cpnext->next;
//			while (cpnextnext && !(cpnextnext->flags & CAM_ORIGIN)) {
//				cpnextnext = cpnextnext->next;
//			}
//		}
//		if (cpnextnext && !(cpnextnext->flags & CAM_ORIGIN)) {
//			cpnextnext = NULL;
//		}
//
//		cpprev = NULL;
//		if (cp->prev) {
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//				cpprev = cpprev->prev;
//			}
//		}
//		if (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//			cpprev = NULL;
//		}
//
//		cpprevprev = NULL;
//		if (cpprev && cpprev->prev) {
//			cpprevprev = cpprev->prev;
//			while (cpprevprev && !(cpprevprev->flags & CAM_ORIGIN)) {
//				cpprevprev = cpprevprev->prev;
//			}
//		}
//		if (cpprevprev && !(cpprevprev->flags & CAM_ORIGIN)) {
//			cpprevprev = NULL;
//		}
//
//		if (cp->type == CAMERA_CURVE) {
//			const cameraPoint_t* p1, * p2, * p3;
//
//			cp->curveCount = curveCount;
//			curveCount++;
//
//			// we do this check here as well because we will need 'curveCount'
//			// later on
//			if (i < ourUpdateStartPoint) {
//				continue;
//			}
//
//			if (!cpnextnext) {
//				if (cpnext->type == CAMERA_CURVE) {
//					cpnext->curveCount = curveCount;
//				}
//			}
//
//			p1 = p2 = p3 = NULL;
//
//			if (cp->curveCount % 2 == 0) {
//				p1 = cp;
//				p2 = cpnext;
//				p3 = cpnextnext;
//
//				if (cpprev && cpprev->type == CAMERA_SPLINE) {
//					VectorCopy(cg.splinePoints[cp->splineStart], point0);
//				}
//				else if (cpprev && cpprev->type == CAMERA_SPLINE_BEZIER) {
//					CG_CameraSplineOriginAt(cp->cgtime, posBezier, point0);
//				}
//				else if (cpprev && cpprev->type == CAMERA_SPLINE_CATMULLROM) {
//					CG_CameraSplineOriginAt(cp->cgtime, posCatmullRom, point0);
//				}
//				else {
//					VectorCopy(cp->origin, point0);
//				}
//
//				if (p2) {
//					VectorCopy(p2->origin, point1);
//				}
//				if (p3) {
//					if (p3->type == CAMERA_SPLINE) {
//						VectorCopy(cg.splinePoints[p3->splineStart], point2);
//					}
//					else if (p3->type == CAMERA_SPLINE_BEZIER) {
//						CG_CameraSplineOriginAt(p3->cgtime, posBezier, point2);
//					}
//					else if (p3->type == CAMERA_SPLINE_CATMULLROM) {
//						CG_CameraSplineOriginAt(p3->cgtime, posCatmullRom, point2);
//					}
//					else {
//						VectorCopy(p3->origin, point2);
//					}
//				}
//			}
//			else {
//				// middle point
//				p1 = cpprev;
//				p2 = cp;
//				p3 = cpnext;
//
//				if (p1 && cpprevprev && cpprevprev->type == CAMERA_SPLINE) {
//					VectorCopy(cg.splinePoints[p1->splineStart], point0);
//				}
//				else if (p1 && cpprevprev && cpprevprev->type == CAMERA_SPLINE_BEZIER) {
//					CG_CameraSplineOriginAt(p1->cgtime, posBezier, point0);
//				}
//				else if (p1 && cpprevprev && cpprevprev->type == CAMERA_SPLINE_CATMULLROM) {
//					CG_CameraSplineOriginAt(p1->cgtime, posCatmullRom, point0);
//				}
//				else {
//					VectorCopy(p1->origin, point0);
//				}
//
//				if (p2) {
//					VectorCopy(p2->origin, point1);
//				}
//				if (p3) {
//					if (p3->type == CAMERA_SPLINE) {
//						VectorCopy(cg.splinePoints[p3->splineStart], point2);
//					}
//					else if (p3->type == CAMERA_SPLINE_BEZIER) {
//						CG_CameraSplineOriginAt(p3->cgtime, posBezier, point2);
//					}
//					else if (p3->type == CAMERA_SPLINE_CATMULLROM) {
//						CG_CameraSplineOriginAt(p3->cgtime, posCatmullRom, point2);
//					}
//					else {
//						VectorCopy(p3->origin, point2);
//					}
//				}
//			}
//
//			if (p1 && p2 && p3) {
//				for (j = 0; j < 3; j++) {
//					//FindQuadratic(p1->cgtime / 1000.0, (double)p1->origin[j], p2->cgtime / 1000.0, (double)p2->origin[j], p3->cgtime / 1000.0, (double)p3->origin[j], &cp->a[j], &cp->b[j], &cp->c[j]);
//					//FindQuadratic(p1->cgtime / 1000.0, (double)point0[j], p2->cgtime / 1000.0, (double)point1[j], p3->cgtime / 1000.0, (double)point2[j], &cp->a[j], &cp->b[j], &cp->c[j]);
//
//					// base it on p1->cgtime / 1000.0  being zero to increase
//					// precision
//					FindQuadratic(0.0, (double)point0[j], (p2->cgtime / 1000.0) - (p1->cgtime / 1000.0), (double)point1[j], (p3->cgtime / 1000.0) - (p1->cgtime / 1000.0), (double)point2[j], &cp->a[j], &cp->b[j], &cp->c[j]);
//					cp->hasQuadratic = qtrue;
//				}
//				cp->quadraticStartTime = p1->cgtime;
//				//Com_Printf("quad time %f\n", cp->quadraticStartTime);
//			}
//			else {
//				// hack for distance, just make it linear
//				Com_Printf("cam %d no quadratic %p %p %p\n", i, p1, p2, p3);
//				cp->hasQuadratic = qfalse;
//			}
//		}  // cp->type == CAMERA_CURVE
//
//		if (i < ourUpdateStartPoint) {
//			continue;
//		}
//
//		cp->originDistance = 0;
//
//		if (cp->type == CAMERA_SPLINE) {
//			for (j = cp->splineStart; j < cpnext->splineStart; j++) {
//				cp->originDistance += Distance(cg.splinePoints[j], cg.splinePoints[j + 1]);
//			}
//			cp->originAvgVelocity = cp->originDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//			cp->originImmediateInitialVelocity = Distance(cg.splinePoints[cp->splineStart], cg.splinePoints[cp->splineStart + 1]) / (((cpnext->cgtime - cp->cgtime) / 1000.0) / (cpnext->splineStart - cp->splineStart));
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					if (cp->originInitialVelocity > 2.0 * cp->originAvgVelocity) {
//						cp->originInitialVelocity = 2.0 * cp->originAvgVelocity;
//					}
//					cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//					cp->originImmediateInitialVelocity *= (cp->originInitialVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateInitialVelocity = 0;
//			}
//			cp->originImmediateFinalVelocity = Distance(cg.splinePoints[cpnext->splineStart - 1], cg.splinePoints[cpnext->splineStart - 2]) / (((cpnext->cgtime - cp->cgtime) / 1000.0) / (cpnext->splineStart - cp->splineStart));
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					cp->originImmediateFinalVelocity *= (cp->originFinalVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateFinalVelocity = 0;
//			}
//		}
//		else if (cp->type == CAMERA_SPLINE_BEZIER || cp->type == CAMERA_SPLINE_CATMULLROM) {
//			posInterpolate_t posType = posBezier;
//			double cameraTime;
//			double startTime, endTime;
//			vec3_t start, end;
//			double timeSlice;
//			int numSplines;
//
//			if (cp->type == CAMERA_SPLINE_CATMULLROM) {
//				posType = posCatmullRom;
//			}
//
//			cameraTime = cpnext->cgtime - cp->cgtime;
//			if (cameraTime <= 0.0) {
//				Com_Printf("^1invalid camera times found during spline calculation:  %f  -> %f\n", cp->cgtime, cpnext->cgtime);
//				return;
//			}
//
//			//FIXME DEFAULT_NUM_SPLINES
//			numSplines = DEFAULT_NUM_SPLINES;
//			if (numSplines <= 1) {
//				Com_Printf("^1invalid number of splines %d\n", numSplines);
//			}
//
//			timeSlice = cameraTime / numSplines;
//
//			for (j = 0; j < numSplines - 1; j++) {
//				startTime = ((double)(j + 0) * timeSlice) + cp->cgtime;
//				endTime = ((double)(j + 1) * timeSlice) + cp->cgtime;
//
//				CG_CameraSplineOriginAt(startTime, posType, start);
//				CG_CameraSplineOriginAt(endTime, posType, end);
//
//				cp->originDistance += Distance(start, end);
//			}
//
//			cp->originAvgVelocity = cp->originDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//			startTime = cp->cgtime;
//			endTime = ((double)(1) * timeSlice) + cp->cgtime;
//			CG_CameraSplineOriginAt(startTime, posType, start);
//			CG_CameraSplineOriginAt(endTime, posType, end);
//
//			cp->originImmediateInitialVelocity = Distance(start, end) / (endTime - startTime) * 1000.0;
//
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					if (cp->originInitialVelocity > 2.0 * cp->originAvgVelocity) {
//						cp->originInitialVelocity = 2.0 * cp->originAvgVelocity;
//					}
//					cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//					cp->originImmediateInitialVelocity *= (cp->originInitialVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateInitialVelocity = 0;
//			}
//
//			startTime = ((double)(numSplines - 1) * timeSlice) + cp->cgtime;
//			endTime = ((double)(numSplines - 0) * timeSlice) + cp->cgtime;
//			CG_CameraSplineOriginAt(startTime, posType, start);
//			CG_CameraSplineOriginAt(endTime, posType, end);
//
//			cp->originImmediateFinalVelocity = Distance(start, end) / (endTime - startTime) * 1000.0;
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					cp->originImmediateFinalVelocity *= (cp->originFinalVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateFinalVelocity = 0;
//			}
//		}
//		else if (cp->type == CAMERA_CURVE) {
//			//double dd;
//
//			if (cp->hasQuadratic) {
//				vec3_t vel;
//				long double tm;
//
//				cp->originDistance = CameraCurveDistance(cp, cpnext);
//				cp->originAvgVelocity = cp->originDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//				tm = ((long double)cp->cgtime - (long double)cp->quadraticStartTime) / (long double)1000.0;
//				for (j = 0; j < 3; j++) {
//					//vel[0] = 2.0 * cp->a[0] * tm + cp->b[0];
//					vel[j] = (long double)2.0 * cp->a[j] * tm + cp->b[j];
//					//Com_Printf("%d:  %f  (2 * %LFx + %LF)  tm: %LF\n", j, vel[j], cp->a[j], cp->b[j], tm);
//				}
//				cp->originImmediateInitialVelocity = VectorLength(vel);
//				if (cp->originAvgVelocity > 0.001) {
//					if (cp->useOriginVelocity) {
//						if (cp->originInitialVelocity > 2.0 * cp->originAvgVelocity) {
//							cp->originInitialVelocity = 2.0 * cp->originAvgVelocity;
//						}
//						cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//						cp->originImmediateInitialVelocity *= (cp->originInitialVelocity / cp->originAvgVelocity);
//					}
//					else {
//						//cp->originImmediateInitialVelocity = cp->originAvgVelocity;
//					}
//				}
//				else {
//					cp->originImmediateInitialVelocity = 0;
//				}
//				//Com_Printf("vel %f\n", VectorLength(vel));
//
//				//tm += ((long double)cpnext->cgtime - (long double)cp->cgtime) / (long double)1000.0;
//				tm = ((long double)cpnext->cgtime - (long double)cp->quadraticStartTime) / (long double)1000.0;
//				for (j = 0; j < 3; j++) {
//
//					vel[j] = (long double)2.0 * cp->a[j] * tm + cp->b[j];
//					//Com_Printf("%d:  %f  (2 * %LFx + %LF)  tm: %LF\n", j, vel[j], cp->a[j], cp->b[j], tm);
//				}
//				cp->originImmediateFinalVelocity = VectorLength(vel);
//				if (cp->originAvgVelocity > 0.001) {
//					if (cp->useOriginVelocity) {
//						cp->originImmediateFinalVelocity *= (cp->originFinalVelocity / cp->originAvgVelocity);
//					}
//				}
//				else {
//					cp->originImmediateFinalVelocity = 0;
//				}
//			}
//			else {
//				cp->originDistance = Distance(cp->origin, cpnext->origin);
//				cp->originAvgVelocity = cp->originDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//				cp->originImmediateInitialVelocity = cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0);
//				if (cp->originAvgVelocity > 0.001) {
//					if (cp->useOriginVelocity) {
//						if (cp->originInitialVelocity > 2.0 * cp->originAvgVelocity) {
//							cp->originInitialVelocity = 2.0 * cp->originAvgVelocity;
//						}
//						cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//						cp->originImmediateInitialVelocity *= (cp->originInitialVelocity / cp->originAvgVelocity);
//					}
//				}
//				else {
//					cp->originImmediateInitialVelocity = 0;
//				}
//				cp->originImmediateFinalVelocity = cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0);
//				if (cp->originAvgVelocity > 0.001) {
//					if (cp->useOriginVelocity) {
//						cp->originImmediateFinalVelocity *= (cp->originFinalVelocity / cp->originAvgVelocity);
//					}
//					else {
//						//cp->originImmediateFinalVelocity = cp->originAvgVelocity;
//					}
//				}
//				else {
//					cp->originImmediateFinalVelocity = 0;
//				}
//			}
//
//#if 0
//			dd = 0;
//			for (j = cp->splineStart; j < cpnext->splineStart; j++) {
//				dd += Distance(cg.splinePoints[j], cg.splinePoints[j + 1]);
//			}
//			Com_Printf("%f  approx  %f  %f\n", cp->originDistance, Distance(cp->origin, cpnext->origin), dd);
//#endif
//		}
//		else {  // not camera curve, or spline type
//			cp->originDistance = Distance(cp->origin, cpnext->origin);
//			cp->originAvgVelocity = cp->originDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//			cp->originImmediateInitialVelocity = cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0);
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					if (cp->originInitialVelocity > 2.0 * cp->originAvgVelocity) {
//						cp->originInitialVelocity = 2.0 * cp->originAvgVelocity;
//					}
//					cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//					cp->originImmediateInitialVelocity *= (cp->originInitialVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateInitialVelocity = 0;
//			}
//			cp->originImmediateFinalVelocity = cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0);
//			if (cp->originAvgVelocity > 0.001) {
//				if (cp->useOriginVelocity) {
//					cp->originImmediateFinalVelocity *= (cp->originFinalVelocity / cp->originAvgVelocity);
//				}
//			}
//			else {
//				cp->originImmediateFinalVelocity = 0;
//			}
//		}
//
//		//Com_Printf("cam %d  vel %f i %f  -> f %f\n", i, cp->originAvgVelocity, cp->originImmediateInitialVelocity, cp->originImmediateFinalVelocity);
//
//	}
//
//	// now camera angle velocities
//	for (i = ourUpdateStartPoint; i < cg.numCameraPoints - 1; i++) {
//		cp = &cg.cameraPoints[i];
//
//		if (!(cp->flags & CAM_ANGLES)) {
//			continue;
//		}
//		cpnext = cp->next;
//		//Com_Printf("next:  %p\n", cpnext);
//		while (cpnext && !(cpnext->flags & CAM_ANGLES)) {
//			cpnext = cpnext->next;
//		}
//		if (!cpnext) {
//			//Com_Printf("^1%d / %d angles next null\n", i, cg.numCameraPoints);
//			break;
//		}
//
//		cpprev = NULL;
//		if (cp->prev) {
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ANGLES)) {
//				cpprev = cpprev->prev;
//			}
//		}
//		if (cpprev && !(cpprev->flags & CAM_ANGLES)) {
//			cpprev = NULL;
//		}
//
//		// prelim values, pass is handled below
//		if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//			vec3_t atmp;
//			double timeSlice;
//
//			//FIXME initial velocity
//			CG_CameraSplineAnglesAt(cp->cgtime, a0);
//			CG_CameraSplineAnglesAt(cpnext->cgtime, a1);
//			//FIXME (maybe) -- not really true, but it doesn't matter that
//			// much since the value is only used as a base for setting avg
//			// initial and final velocities
//			if (cp->rollType == CAMERA_ROLL_AS_ANGLES) {
//				cp->anglesDistance = CG_CameraAngleLongestDistanceWithRoll(a1, a0);
//			}
//			else {
//				cp->anglesDistance = CG_CameraAngleLongestDistanceNoRoll(a1, a0);
//			}
//			cp->anglesAvgVelocity = cp->anglesDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//			//FIXME 40 fixed?
//			timeSlice = (cpnext->cgtime - cp->cgtime) / 40.0;
//			CG_CameraSplineAnglesAt(cp->cgtime + timeSlice, atmp);
//			if (cp->rollType == CAMERA_ROLL_AS_ANGLES) {
//				cp->anglesImmediateInitialVelocity = CG_CameraAngleLongestDistanceWithRoll(a0, atmp) / timeSlice * 1000.0;
//			}
//			else {
//				cp->anglesImmediateInitialVelocity = CG_CameraAngleLongestDistanceNoRoll(a0, atmp) / timeSlice * 1000.0;
//			}
//			CG_CameraSplineAnglesAt(cpnext->cgtime - timeSlice, atmp);
//			if (cp->rollType == CAMERA_ROLL_AS_ANGLES) {
//				cp->anglesImmediateFinalVelocity = CG_CameraAngleLongestDistanceWithRoll(a1, atmp) / timeSlice * 1000.0;
//			}
//			else {
//				cp->anglesImmediateFinalVelocity = CG_CameraAngleLongestDistanceNoRoll(a1, atmp) / timeSlice * 1000.0;
//			}
//		}
//		else {
//			VectorCopy(cp->angles, a0);
//			VectorCopy(cpnext->angles, a1);
//			if (cp->rollType != CAMERA_ROLL_AS_ANGLES) {
//				a0[ROLL] = 0;
//				a1[ROLL] = 0;
//			}
//			//AnglesSubtract(a1, a0, cp->anglesDistance);
//			if (cp->rollType != CAMERA_ROLL_AS_ANGLES) {
//				cp->anglesDistance = CG_CameraAngleLongestDistanceNoRoll(a1, a0);
//			}
//			else {
//				cp->anglesDistance = CG_CameraAngleLongestDistanceWithRoll(a1, a0);
//			}
//			cp->anglesAvgVelocity = cp->anglesDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//			cp->anglesImmediateInitialVelocity = cp->anglesAvgVelocity;
//			cp->anglesImmediateFinalVelocity = cp->anglesAvgVelocity;
//		}
//
//
//		//Com_Printf("%d angles distance %f (%f)\n", i, cp->anglesDistance, cp->anglesAvgVelocity);
//
//		if (cp->useAnglesVelocity) {
//			if (cp->anglesInitialVelocity > 2.0 * cp->anglesAvgVelocity) {
//				cp->anglesInitialVelocity = 2.0 * cp->anglesAvgVelocity;
//			}
//			// fucking hell... different pass types:  see /ecam
//			cp->anglesFinalVelocity = 2.0 * cp->anglesDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesInitialVelocity;
//
//			cp->anglesImmediateInitialVelocity *= (cp->anglesInitialVelocity / cp->anglesAvgVelocity);
//			cp->anglesImmediateFinalVelocity *= (cp->anglesFinalVelocity / cp->anglesAvgVelocity);
//		}
//
//		cp->xoffsetDistance = fabs(cpnext->xoffset - cp->xoffset);
//		cp->xoffsetAvgVelocity = cp->xoffsetDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		cp->yoffsetDistance = fabs(cpnext->yoffset - cp->yoffset);
//		cp->yoffsetAvgVelocity = cp->yoffsetDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		cp->zoffsetDistance = fabs(cpnext->zoffset - cp->zoffset);
//		cp->zoffsetAvgVelocity = cp->zoffsetDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		//cp->fovDistance = fabs(cpnext->fov - cp->fov);
//		//cp->fovAvgVelocity = cp->fovDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		cp->rollDistance = fabs(cpnext->angles[ROLL] - cp->angles[ROLL]);
//		while (cp->rollDistance >= 180.0) {
//			cp->rollDistance -= 180.0;
//		}
//		cp->rollAvgVelocity = cp->rollDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		// pass options
//		if (cp->viewType == CAMERA_ANGLES_VIEWPOINT_INTERP) {
//			count = 0;
//			cptmp = NULL;
//			for (j = i + 1; j < cg.numCameraPoints; j++) {
//				cptmp = &cg.cameraPoints[j];
//				if (!(cptmp->flags & CAM_ANGLES)) {
//					continue;
//				}
//				if (cptmp->viewType != CAMERA_ANGLES_VIEWPOINT_PASS) {
//					break;
//				}
//				count++;
//			}
//			if (cptmp && (cptmp->viewType == CAMERA_ANGLES_VIEWPOINT_INTERP || cptmp->viewType == CAMERA_ANGLES_VIEWPOINT_FIXED) && count > 0) {
//				Com_Printf("viewpoint match for %d -> %d\n", i, j);
//				passStart = i;
//				passEnd = j;
//
//#if 0
//				VectorCopy(cg.cameraPoints[passStart].angles, a0);
//				VectorCopy(cg.cameraPoints[passEnd].angles, a1);
//				a0[ROLL] = 0;
//				a1[ROLL] = 0;
//#endif
//				VectorCopy(cg.cameraPoints[passStart].viewPointOrigin, a0);
//				VectorCopy(cg.cameraPoints[passEnd].viewPointOrigin, a1);
//
//				dist = CG_CameraAngleLongestDistanceNoRoll(a0, a1);
//				avg = dist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//
//				for (j = i; j < passEnd; j++) {
//					cg.cameraPoints[j].viewPointPassStart = passStart;
//					cg.cameraPoints[j].viewPointPassEnd = passEnd;
//
//					cg.cameraPoints[j].anglesDistance = dist;
//					cg.cameraPoints[j].anglesAvgVelocity = avg;
//
//					Com_Printf("viewpoint pass info set for %d\n", j);
//				}
//			}
//			else if (cpnext->viewType == CAMERA_ANGLES_VIEWPOINT_INTERP || cpnext->viewType == CAMERA_ANGLES_VIEWPOINT_FIXED) {
//				cp->anglesDistance = CG_CameraAngleLongestDistanceNoRoll(cp->viewPointOrigin, cpnext->viewPointOrigin);
//				cp->anglesAvgVelocity = cp->anglesDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//				//Com_Printf("yes\n");
//			}
//		}
//
//		if (cp->offsetType == CAMERA_OFFSET_INTERP) {
//			count = 0;
//			cptmp = NULL;
//			for (j = i + 1; j < cg.numCameraPoints; j++) {
//				cptmp = &cg.cameraPoints[j];
//				if (!(cptmp->flags & CAM_ANGLES)) {
//					continue;
//				}
//				if (cptmp->offsetType != CAMERA_OFFSET_PASS) {
//					break;
//				}
//				count++;
//			}
//			if (cptmp && (cptmp->offsetType == CAMERA_OFFSET_INTERP || cptmp->offsetType == CAMERA_OFFSET_FIXED) && count > 0) {
//				double xdist, ydist, zdist;
//				double xavg, yavg, zavg;
//
//				Com_Printf("offset match for %d -> %d\n", i, j);
//				passStart = i;
//				passEnd = j;
//
//				xdist = fabs(cg.cameraPoints[passEnd].xoffset - cg.cameraPoints[passStart].xoffset);
//				ydist = fabs(cg.cameraPoints[passEnd].yoffset - cg.cameraPoints[passStart].yoffset);
//				zdist = fabs(cg.cameraPoints[passEnd].zoffset - cg.cameraPoints[passStart].zoffset);
//
//				xavg = xdist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//				yavg = ydist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//				zavg = zdist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//
//				for (j = i; j < passEnd; j++) {
//					cg.cameraPoints[j].offsetPassStart = passStart;
//					cg.cameraPoints[j].offsetPassEnd = passEnd;
//
//					cg.cameraPoints[j].xoffsetDistance = xdist;
//					cg.cameraPoints[j].yoffsetDistance = ydist;
//					cg.cameraPoints[j].zoffsetDistance = zdist;
//
//					cg.cameraPoints[j].xoffsetAvgVelocity = xavg;
//					cg.cameraPoints[j].yoffsetAvgVelocity = yavg;
//					cg.cameraPoints[j].zoffsetAvgVelocity = zavg;
//
//					Com_Printf("offset pass info set for %d\n", j);
//				}
//			}
//		}
//
//		if (cp->rollType == CAMERA_ROLL_INTERP) {
//			count = 0;
//			cptmp = NULL;
//			for (j = i + 1; j < cg.numCameraPoints; j++) {
//				cptmp = &cg.cameraPoints[j];
//				if (!(cptmp->flags & CAM_ANGLES)) {
//					continue;
//				}
//				if (cptmp->rollType != CAMERA_ROLL_PASS) {
//					break;
//				}
//				count++;
//			}
//			if (cptmp && (cptmp->rollType == CAMERA_ROLL_INTERP || cptmp->rollType == CAMERA_ROLL_FIXED || cptmp->rollType == CAMERA_ROLL_AS_ANGLES) && count > 0) {
//				Com_Printf("roll match for %d -> %d\n", i, j);
//				passStart = i;
//				passEnd = j;
//
//				dist = fabs(cg.cameraPoints[passEnd].angles[ROLL] - cg.cameraPoints[passStart].angles[ROLL]);
//				while (dist >= 180.0) {
//					dist -= 180.0;
//				}
//				avg = dist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//
//				for (j = i; j < passEnd; j++) {
//					cg.cameraPoints[j].rollPassStart = passStart;
//					cg.cameraPoints[j].rollPassEnd = passEnd;
//
//					cg.cameraPoints[j].rollDistance = dist;
//					cg.cameraPoints[j].rollAvgVelocity = avg;
//
//					Com_Printf("roll pass info set for %d\n", j);
//				}
//			}
//		}
//
//	}
//
//	// now camera fov velocities
//	for (i = ourUpdateStartPoint; i < cg.numCameraPoints - 1; i++) {
//		cp = &cg.cameraPoints[i];
//
//		if (!(cp->flags & CAM_FOV)) {
//			continue;
//		}
//		cpnext = cp->next;
//		//Com_Printf("next:  %p\n", cpnext);
//		while (cpnext && !(cpnext->flags & CAM_FOV)) {
//			cpnext = cpnext->next;
//		}
//		if (!cpnext) {
//			//Com_Printf("^1%d / %d fov next null\n", i, cg.numCameraPoints);
//			break;
//		}
//
//		cpprev = NULL;
//		if (cp->prev) {
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_FOV)) {
//				cpprev = cpprev->prev;
//			}
//		}
//		if (cpprev && !(cpprev->flags & CAM_FOV)) {
//			cpprev = NULL;
//		}
//
//		cp->fovDistance = fabs(cpnext->fov - cp->fov);
//		cp->fovAvgVelocity = cp->fovDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		if (cp->fovType == CAMERA_FOV_INTERP) {
//			count = 0;
//			cptmp = NULL;
//			for (j = i + 1; j < cg.numCameraPoints; j++) {
//				cptmp = &cg.cameraPoints[j];
//				if (!(cptmp->flags & CAM_FOV)) {
//					continue;
//				}
//				if (cptmp->fovType != CAMERA_FOV_PASS) {
//					break;
//				}
//				count++;
//			}
//			if (cptmp && (cptmp->fovType == CAMERA_FOV_INTERP || cptmp->fovType == CAMERA_FOV_FIXED || cptmp->fovType == CAMERA_FOV_USE_CURRENT) && count > 0) {
//				Com_Printf("fov match for %d -> %d\n", i, j);
//				passStart = i;
//				passEnd = j;
//
//				dist = fabs(cg.cameraPoints[passEnd].fov - cg.cameraPoints[passStart].fov);
//				avg = dist / (cg.cameraPoints[passEnd].cgtime - cg.cameraPoints[passStart].cgtime) * 1000.0;
//
//				for (j = i; j < passEnd; j++) {
//					cg.cameraPoints[j].fovPassStart = passStart;
//					cg.cameraPoints[j].fovPassEnd = passEnd;
//
//					cg.cameraPoints[j].fovDistance = dist;
//					cg.cameraPoints[j].fovAvgVelocity = avg;
//
//					Com_Printf("fov pass info set for %d\n", j);
//				}
//			}
//		}
//		else if (cp->fovType == CAMERA_FOV_SPLINE) {
//			float startFov, endFov;
//
//			if (!CG_CameraSplineFovAt(cp->cgtime, &startFov)) {
//				if (cgs.realProtocol >= 91 && cg_useDemoFov.integer == 1) {
//					startFov = cg.demoFov;
//				}
//				else {
//					startFov = cg_fov.value;
//				}
//			}
//			if (!CG_CameraSplineFovAt(cpnext->cgtime, &endFov)) {
//				if (cgs.realProtocol >= 91 && cg_useDemoFov.integer == 1) {
//					endFov = cg.demoFov;
//				}
//				else {
//					endFov = cg_fov.value;
//				}
//			}
//			cp->fovDistance = fabs(startFov - endFov);
//			cp->fovAvgVelocity = cp->fovDistance / (cpnext->cgtime - cp->cgtime) * 1000.0;
//
//		}  // cp->fovType
//	}
//
//
//	// debugging
//#if 0
//	for (i = 0; i < cg.numCameraPoints; i++) {
//		Com_Printf("W %d:  %f %f %f\n", i, cg.cameraPoints[i].origin[0], cg.cameraPoints[i].origin[1], cg.cameraPoints[i].origin[2]);
//	}
//
//	{
//		int count = 0;
//		demoCameraPoint_t* p = demo.camera.points;
//		while (p) {
//			Com_Printf("Q %d:  %f %f %f\n", count, p->origin[0], p->origin[1], p->origin[2]);
//			count++;
//			p = p->next;
//		}
//	}
//#endif
//
//	//FIXME maybe not here
//	// curve type spline points for drawpath
//	if (cg.numSplinePoints < MAX_SPLINEPOINTS) {
//		const cameraPoint_t* cp;
//
//		//for (cp = cg.cameraPointsPointer;  cp != NULL;  cp = cp->next) {
//		for (cp = &cg.cameraPoints[ourUpdateStartPoint]; cp != NULL; cp = cp->next) {
//			const cameraPoint_t* cpnext;
//			int numSplinePoints;
//			long double t;
//			long double timeSlice;
//			int i;
//
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//			if (cp->type != CAMERA_CURVE) {
//				continue;
//			}
//			if (!cp->hasQuadratic) {  // already handled above
//				VectorCopy(cp->origin, cg.splinePoints[cp->splineStart]);
//				continue;
//			}
//
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//				cpnext = cpnext->next;
//			}
//
//			if (!cpnext) {
//				Com_Printf("^3curve spline calc !cpnext\n");
//				break;
//			}
//
//			numSplinePoints = cpnext->splineStart - cp->splineStart;
//			timeSlice = (cpnext->cgtime - cp->cgtime) / (long double)numSplinePoints;
//			for (i = 0; i < numSplinePoints; i++) {
//				int j;
//
//				t = (cp->cgtime - cp->quadraticStartTime) + (timeSlice * i);
//				// in seconds
//				t /= 1000.0;
//
//				for (j = 0; j < 3; j++) {
//					cg.splinePoints[cp->splineStart + i][j] = cp->a[j] * t * t + cp->b[j] * t + cp->c[j];
//				}
//			}
//		}
//	}  // else, spline creation was aborted above
//
//	//FIXME not here
//	trap_SendConsoleCommand("savecamera wolfcam-autosave\n");
//}
//
//static void CG_UpdateCameraInfo(void)
//{
//	CG_UpdateCameraInfoExt(0);
//}
//
//static void CG_ResetCameraVelocities(int cameraNum)
//{
//	cameraPoint_t* cp;
//
//	if (cameraNum < 0 || cameraNum >= cg.numCameraPoints) {
//		return;
//	}
//
//	cp = &cg.cameraPoints[cameraNum];
//
//	cp->useOriginVelocity = qfalse;
//	//cp->originInitialVelocity = cp->originAvgVelocity;
//	//cp->originFinalVelocity = cp->originAvgVelocity;
//
//	cp->useAnglesVelocity = qfalse;
//	cp->anglesInitialVelocity = cp->anglesAvgVelocity;
//	cp->anglesFinalVelocity = cp->anglesAvgVelocity;
//
//	cp->useXoffsetVelocity = qfalse;
//	cp->useYoffsetVelocity = qfalse;
//	cp->useZoffsetVelocity = qfalse;
//
//	cp->useFovVelocity = qfalse;
//	cp->useRollVelocity = qfalse;
//}
//
//
//static vec3_t Old[MAX_CAMERAPOINTS];
//
//static const char* EcamHelpDoc = "\n"
//"Edit all currently selected camera points\n"
//"<...> are required\n"
//"[...] are optional\n"
//"\n"
//"/ecam type <spline, interp, jump, curve, splinebezier, splinecatmullrom>\n"
//"/ecam fov <current, interp, fixed, pass, spline> [fov value]\n"
//"/ecam command <command to be executed when cam point is hit>\n"
//"/ecam numsplines <number of spline points to use for this key point (default is 40)>\n"  //FIXME default 40 value
//"/ecam angles <interp, spline, interpuseprevious, fixed, fixeduseprevious, viewpointinterp, viewpointfixed, viewpointpass, ent>\n"
//"   the 'ent' option has additional parameter for the entity\n"
//"   /ecam angles ent [entity number]\n"
//"/ecam offset <interp, fixed, pass> [x offset] [y offset] [z offset]\n"
//"/ecam roll <interp, fixed, pass, angles> [roll value]\n"
//"/ecam flags [origin | angles | fov | time]\n"
//"/ecam initialVelocity <origin, angles, xoffset, yoffset, zoffset, fov, roll> <value, or 'reset' to reset to default fixed velocity>\n"
//"/ecam finalVelocity <origin, angles, xoffset, yoffset, zoffset, fov, roll> <value, or 'reset' to reset to default fixed velocity>\n"
//"/ecam rebase [origin | angles | dir | dirna | time | timen <server time>] ...\n"
//"   edit camera times to start now or at the time given time, use current\n"
//"   angles, origin, or direction as the new starting values\n"
//"   note:  dirna updates the camera direction without altering camera angles\n"
//"/ecam shifttime <milliseconds>\n"
//"/ecam smooth velocity\n"
//"   change camera times to have the final immediate velocity of a camera point\n"
//"   match the initial immediate velocity of the next camera point\n"
//"/ecam smooth avgvelocity\n"
//"   change camera times to have all points match the total average velocity\n"
//"   run command multiple times for better precision\n"
//"/ecam smooth origin\n"
//"   match, if possible, the final immediate velocity of a camera point to the\n"
//"   immediate initial velocity of the next camera point to have smooth origin\n"
//"   transitions  (done by setting the appropriate overall final and initial\n"
//"   velocities)\n"
//"/ecam smooth originf\n"
//"   aggresive origin smoothing which will change origins and camera times in\n"
//"   order to match velocities\n"
//"/ecam smooth anglesf\n"
//"   aggresive angles smoothing which will change angles (but not times) in order\n"
//"   to match angle velocities\n"
//"/ecam smooth time\n"
//"   change camera times so that the points have the same average velocity\n"
//"/ecam scale <speed up/down scale value>\n"
//"   speed up or down the selected camera points by adjusting camera time (2.0: twice as fast, 0.5: half speed)\n"
//"\n";
//
//static void CG_ChangeSelectedCameraPoints_f(void)
//{
//	cameraPoint_t* cp;
//	cameraPoint_t* cpprev;
//	cameraPoint_t* cpnext;
//	int i;
//	int j;
//	const char* s;
//	vec3_t angs0;
//	vec3_t angs1;
//	cameraPoint_t* cstart;
//	const cameraPoint_t* cend;
//	int n;
//
//	cg.cameraPlaying = qfalse;
//	cg.cameraPlayedLastFrame = qfalse;
//
//	if (CG_Argc() < 2 || !Q_stricmp(CG_Argv(1), "help")) {
//		Com_Printf("%s\n", EcamHelpDoc);
//		return;
//	}
//
//	if (!Q_stricmp(CG_Argv(1), "reset")) {
//		for (i = 0; i < cg.numCameraPoints; i++) {
//			CG_ResetCameraVelocities(i);
//		}
//
//		CG_UpdateCameraInfo();
//		return;
//	}
//
//	if (!Q_stricmp(CG_Argv(1), "rebase")) {
//		if (CG_Argc() < 3) {
//			Com_Printf("usage: ecam rebase [origin | angles | time | timen <server time>] ...\n");
//			return;
//		}
//
//		if (cg.numCameraPoints < 1) {
//			Com_Printf("no camera points\n");
//			return;
//		}
//
//		n = 2;
//		while (*CG_Argv(n)) {
//			if (!Q_stricmp(CG_Argv(n), "origin")) {
//				vec3_t offset;
//				vec3_t newOrigin;
//
//				VectorSubtract(cg.cameraPoints[0].origin, cg.refdef.vieworg, offset);
//
//				for (i = 0; i < cg.numCameraPoints; i++) {
//					VectorSubtract(cg.cameraPoints[i].origin, offset, newOrigin);
//					VectorCopy(newOrigin, cg.cameraPoints[i].origin);
//				}
//
//				CG_CameraResetInternalLengths();
//				CG_UpdateCameraInfo();
//			}
//
//			if (!Q_stricmp(CG_Argv(n), "angles")) {
//				vec3_t offset;
//				vec3_t newAngles;
//
//				AnglesSubtract(cg.cameraPoints[0].angles, cg.refdefViewAngles, offset);
//				for (i = 0; i < cg.numCameraPoints; i++) {
//					AnglesSubtract(cg.cameraPoints[i].angles, offset, newAngles);
//					VectorCopy(newAngles, cg.cameraPoints[i].angles);
//				}
//
//				CG_UpdateCameraInfo();
//			}
//
//
//			if (!Q_stricmp(CG_Argv(n), "dir") || !Q_stricmp(CG_Argv(n), "dirna")) {
//				vec3_t origForward, origRight, origUp;
//				vec3_t newForward, newRight, newUp;
//				vec3_t df, dr, du;
//				qboolean noAngles;
//				vec3_t p;
//				vec3_t origOrigin, newOrigin;
//				float scale;
//				vec3_t angles;
//				vec3_t dir;
//				vec3_t tmpAngles;
//				vec3_t fUp;
//
//				if (cg.numCameraPoints < 2) {
//					return;
//				}
//
//				if (!Q_stricmp(CG_Argv(n), "dirna")) {
//					noAngles = qtrue;
//				}
//				else {
//					noAngles = qfalse;
//				}
//
//				AngleVectors(cg.refdefViewAngles, newForward, newRight, newUp);
//				VectorNormalize(newForward);
//				VectorNormalize(newRight);
//				VectorNormalize(newUp);
//
//				VectorSubtract(cg.cameraPoints[1].origin, cg.cameraPoints[0].origin, origForward);
//				VectorNormalize(origForward);
//				vectoangles(origForward, angles);
//				AngleVectors(angles, origForward, origRight, origUp);
//				//AngleVectors(cg.cameraPoints[0].angles, NULL, NULL, aUp);
//				//AngleVectors(cg.cameraPoints[0].angles, origForward, origRight, origUp);
//
//				VectorCopy(cg.cameraPoints[0].origin, origOrigin);
//				VectorCopy(cg.refdef.vieworg, cg.cameraPoints[0].origin);
//				VectorCopy(cg.refdef.vieworg, newOrigin);
//				VectorCopy(origOrigin, Old[0]);
//
//				for (i = 1; i < cg.numCameraPoints; i++) {
//					cp = &cg.cameraPoints[i];
//
//					VectorCopy(cp->origin, Old[i]);
//
//					VectorClear(p);
//					ProjectPointOntoVector(cp->origin, origOrigin, origForward, p);
//					VectorSubtract(p, origOrigin, df);
//					ProjectPointOntoVector(cp->origin, origOrigin, origRight, p);
//					VectorSubtract(p, origOrigin, dr);
//					ProjectPointOntoVector(cp->origin, origOrigin, origUp, p);
//					VectorSubtract(p, origOrigin, du);
//
//					VectorClear(cp->origin);
//					scale = VectorGetScale(df, origForward);
//					VectorMA(newOrigin, scale, newForward, cp->origin);
//					scale = VectorGetScale(dr, origRight);
//					VectorMA(cp->origin, scale, newRight, cp->origin);
//					scale = VectorGetScale(du, origUp);
//					VectorMA(cp->origin, scale, newUp, cp->origin);
//
//				}
//
//				for (i = 0; i < cg.numCameraPoints; i++) {
//					if (!noAngles) {
//						float roll;
//						vec3_t forward, anglePoint;
//						vec3_t newAnglePoint;
//						vec3_t realUp;
//						vec3_t finalUp;
//						float f, g;
//
//						cp = &cg.cameraPoints[i];
//
//						roll = cp->angles[ROLL];
//						VectorCopy(cp->angles, tmpAngles);
//						tmpAngles[ROLL] = 0;
//						AngleVectors(tmpAngles, forward, NULL, realUp);
//						VectorNormalize(forward);
//						VectorMA(Old[i], 10, forward, anglePoint);
//						VectorSubtract(anglePoint, Old[i], dir);
//						vectoangles(dir, tmpAngles);
//						AngleVectors(tmpAngles, NULL, NULL, fUp);
//						f = AngleBetweenVectors(fUp, realUp);
//						Com_Printf("%d:  %f   (%f %f)  (%f %f)\n", i, f, realUp[0], realUp[1], fUp[0], fUp[1]);
//
//						VectorClear(p);
//						ProjectPointOntoVector(anglePoint, origOrigin, origForward, p);
//						VectorSubtract(p, origOrigin, df);
//						ProjectPointOntoVector(anglePoint, origOrigin, origRight, p);
//						VectorSubtract(p, origOrigin, dr);
//						ProjectPointOntoVector(anglePoint, origOrigin, origUp, p);
//						VectorSubtract(p, origOrigin, du);
//
//						VectorClear(newAnglePoint);
//						scale = VectorGetScale(df, origForward);
//						VectorMA(newOrigin, scale, newForward, newAnglePoint);
//						scale = VectorGetScale(dr, origRight);
//						VectorMA(newAnglePoint, scale, newRight, newAnglePoint);
//						scale = VectorGetScale(du, origUp);
//						VectorMA(newAnglePoint, scale, newUp, newAnglePoint);
//
//						VectorSubtract(newAnglePoint, cp->origin, dir);
//						vectoangles(dir, cp->angles);
//						cp->angles[YAW] = AngleNormalize180(cp->angles[YAW]);
//						cp->angles[PITCH] = AngleNormalize180(cp->angles[PITCH]);
//						AngleVectors(cp->angles, NULL, NULL, finalUp);
//
//						//f = AngleBetweenVectors(realUp, origUp);
//						g = AngleBetweenVectors(finalUp, newUp);
//
//						Com_Printf("%d:  %f  %f\n", i, f, g);
//						cp->angles[ROLL] = roll;  //90;  //roll;
//
//					}
//				}
//
//				CG_CameraResetInternalLengths();
//				CG_UpdateCameraInfo();
//			}
//
//			if (!Q_stricmp(CG_Argv(n), "time")) {
//				double startTimeOrig;
//				double ftime;
//
//				startTimeOrig = cg.cameraPoints[0].cgtime;
//				ftime = cg.ftime;
//
//				for (i = 0; i < cg.numCameraPoints; i++) {
//					cp = &cg.cameraPoints[i];
//					cp->cgtime = (cp->cgtime - startTimeOrig) + ftime;
//				}
//
//				CG_UpdateCameraInfo();
//			}
//
//			if (!Q_stricmp(CG_Argv(n), "timen")) {
//				double startTimeOrig;
//				double ftime;
//
//				startTimeOrig = cg.cameraPoints[0].cgtime;
//
//				ftime = atof(CG_Argv(n + 1));
//				n++;
//
//				for (i = 0; i < cg.numCameraPoints; i++) {
//					cp = &cg.cameraPoints[i];
//					cp->cgtime = (cp->cgtime - startTimeOrig) + ftime;
//				}
//
//				CG_UpdateCameraInfo();
//			}
//
//			// Time shifting is done below since it works with selected camera
//			// points.  'rebase' is for all points.
//			n++;
//		}
//
//		return;
//	}  // end '/ecam rebase ...'
//
//	if (cg.selectedCameraPointMin < 0 || cg.selectedCameraPointMax >= cg.numCameraPoints) {
//		Com_Printf("invalid min selected point\n");
//		return;
//	}
//	if (cg.selectedCameraPointMax < 0 || cg.selectedCameraPointMax >= cg.numCameraPoints) {
//		Com_Printf("invalid max selected point\n");
//		return;
//	}
//
//#if 0
//	if (!Q_stricmp(CG_Argv(1), "flip")) {
//		for (i = cg.selectedCameraPointMin; i <= cg.selectedCameraPointMax; i++) {
//			cp = &cg.cameraPoints[i];
//			Com_Printf("%d\n", i);
//			cp->angles[PITCH] = AngleNormalize180(cp->angles[PITCH] + 180);
//			cp->angles[YAW] = AngleNormalize180(cp->angles[YAW] + 180);
//		}
//		CG_UpdateCameraInfo();
//		return;
//	}
//#endif
//
//	if (!Q_stricmp(CG_Argv(1), "shifttime")) {
//		double shift;
//
//		if (CG_Argc() < 3) {
//			Com_Printf("usage: ecam shifttime <milliseconds>\n");
//			return;
//		}
//
//		shift = atof(CG_Argv(2));
//
//		for (i = cg.selectedCameraPointMin; i < cg.selectedCameraPointMax; i++) {
//			cg.cameraPoints[i].cgtime += shift;
//		}
//		CG_UpdateCameraInfo();
//		return;
//	}
//
//	if (!Q_stricmp(CG_Argv(1), "scale")) {
//		double s;
//
//		if (CG_Argc() < 3) {
//			Com_Printf("usage: ecam scale <speed up/down scale value>\n");
//			return;
//		}
//
//		s = atof(CG_Argv(2));
//
//		if (s <= 0.0) {
//			Com_Printf("invalid scale value\n");
//			Com_Printf("usage: ecam scale <speed up/down scale value>\n");
//			return;
//		}
//
//		s = 1.0 / s;
//
//		for (i = cg.selectedCameraPointMin; i < cg.selectedCameraPointMax; i++) {
//			double origTimeLength;
//			double newTimeLength;
//			double diff;
//			double origNextTime;
//
//			cp = &cg.cameraPoints[i];
//			cpnext = &cg.cameraPoints[i + 1];
//
//			origNextTime = cpnext->cgtime;
//			origTimeLength = cpnext->cgtime - cp->cgtime;
//			newTimeLength = origTimeLength * s;
//
//			cpnext->cgtime = cp->cgtime + newTimeLength;
//			diff = origNextTime - cpnext->cgtime;
//
//			for (j = i + 2; j <= cg.selectedCameraPointMax; j++) {
//				cg.cameraPoints[j].cgtime -= diff;
//			}
//		}
//		CG_UpdateCameraInfo();
//		return;
//	}  // end '/ecam scale ...'
//
//	// smoothing
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "time")) {
//		int start, end;
//		float totalLength;
//		double totalTime;
//		double avgVelocity;
//
//		start = cg.selectedCameraPointMin;
//		end = cg.selectedCameraPointMax;
//
//		if (end > cg.numCameraPoints - 1) {
//			end = cg.numCameraPoints - 1;
//		}
//
//		// get total length
//		totalLength = 0;
//		for (i = start; i <= end; i++) {
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//			totalLength += cp->originDistance;
//
//		}
//
//		if (totalLength <= 0) {
//			Com_Printf("^3camera total length is 0, can't smoothen\n");
//			return;
//		}
//		totalTime = cg.cameraPoints[end].cgtime - cg.cameraPoints[start].cgtime;
//		avgVelocity = totalLength / totalTime;
//
//		for (i = start; i <= end; i++) {
//			double origNextTime;
//			float scale;
//			cameraPoint_t* cpTmp;
//			double newTime;
//			double oldTime;
//			double diff;
//
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//				cpnext = cpnext->next;
//			}
//
//			if (!cpnext) {
//				break;
//			}
//
//			origNextTime = cpnext->cgtime;
//			oldTime = cpnext->cgtime - cp->cgtime;
//
//			newTime = cp->originDistance / avgVelocity;
//			scale = newTime / oldTime;
//
//			cpnext->cgtime = cp->cgtime + newTime;
//
//			// scale the points in between the two CAM_ORIGIN points
//			cpTmp = cp;
//			while (cpTmp->next != cpnext) {
//				double t;
//
//				t = cpTmp->cgtime - cp->cgtime;
//				t *= scale;
//				cpTmp->cgtime = cp->cgtime + t;
//
//				cpTmp = cpTmp->next;
//			}
//
//			// add the new diff to cpnext and beyond
//			diff = cpnext->cgtime - origNextTime;
//			cpTmp = cpnext->next;
//			while (cpTmp) {
//				cpTmp->cgtime += diff;
//
//				cpTmp = cpTmp->next;
//			}
//		}
//
//		CG_UpdateCameraInfo();
//		return;
//	}  // end '/ecam smooth time'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "origin")) {
//		int start, end;
//
//		start = cg.selectedCameraPointMin;
//		end = cg.selectedCameraPointMax;
//
//		if (start < 1) {
//			start = 1;
//		}
//		if (end > cg.numCameraPoints - 1) {
//			end = cg.numCameraPoints - 1;
//		}
//
//		for (i = start; i <= end; i++) {
//			long double scale;
//
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//				cpprev = cpprev->prev;
//			}
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//				cpnext = cpnext->next;
//			}
//
//			//cp->useOriginVelocity = qtrue;
//
//			if (!cpprev) {
//				Com_Printf("skipping %d, no previous camera point\n", i);
//				continue;
//			}
//			if (!cpnext) {
//				Com_Printf("skipping %d, no following camera point\n", i);
//				continue;
//			}
//
//			if (cpprev->originImmediateFinalVelocity < 0.001) {  //FIXME
//				Com_Printf("^3skipping %d, prev camera point final immediate velocity close to zero\n", i);
//				continue;
//			}
//
//			// want
//			// cp->originImmediateInitialVelocity == cpprev->originImmediateFinalVelocity;
//			scale = cpprev->originImmediateFinalVelocity / cp->originImmediateInitialVelocity;
//			if (cp->useOriginVelocity) {
//				cp->originInitialVelocity *= scale;
//			}
//			else {
//				cp->useOriginVelocity = qtrue;
//				cp->originInitialVelocity = cp->originAvgVelocity * scale;
//			}
//			Com_Printf("%d  scale %f (%f / %f)  vel %f\n", i, (double)scale, (double)cpprev->originImmediateFinalVelocity, (double)cp->originImmediateInitialVelocity, (double)cp->originAvgVelocity);
//			//cp->originInitialVelocity = cp->originAvgVelocity * scale;
//
//			if (cp->originInitialVelocity < 0.0) {
//				Com_Printf("^3velocity is less than zero: couldn't set correct initial velocity (%f)\n", (double)cp->originInitialVelocity);
//				cp->originInitialVelocity = 0;
//			}
//
//			if (cp->originInitialVelocity > cp->originAvgVelocity * cg_cameraSmoothFactor.value) {
//				Com_Printf("^3velocity is greater than %f * avgVelocity:  couldn't set correct initial velocity (%f)\n", cg_cameraSmoothFactor.value, (double)cp->originInitialVelocity);
//				cp->originInitialVelocity = cp->originAvgVelocity * cg_cameraSmoothFactor.value;
//			}
//
//			cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//			CG_UpdateCameraInfo();
//		}
//		//CG_UpdateCameraInfo();
//		return;
//	}  // end '/ecam smooth origin'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "originf")) {
//		int start, end;
//
//		start = cg.selectedCameraPointMin;
//		end = cg.selectedCameraPointMax;
//
//		//FIXME why?  because prev is changed?
//		if (start < 1) {
//			start = 1;
//		}
//		if (end > cg.numCameraPoints - 1) {
//			//FIXME why -2 and not -1 like '/ecam smooth origin'  ?
//			end = cg.numCameraPoints - 2;
//		}
//
//		for (i = start; i <= end; i++) {
//			vec3_t newOrigin;
//			vec3_t dir;
//			int runs;
//
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//				cpprev = cpprev->prev;
//			}
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//				cpnext = cpnext->next;
//			}
//
//			//cp->useOriginVelocity = qtrue;
//
//			if (!cpprev) {
//				Com_Printf("skipping %d, no previous camera point\n", i);
//				continue;
//			}
//			if (!cpnext) {
//				Com_Printf("skipping %d, no following camera point\n", i);
//				continue;
//			}
//
//			if (cpprev->originImmediateFinalVelocity < 0.001) {  //FIXME
//				Com_Printf("^3skipping %d  prev camera point final immediate velocity close to zero\n", i);
//				continue;
//			}
//
//			for (runs = 0; runs < 30; runs++) {
//				if ((cp->originImmediateInitialVelocity * cg_cameraSmoothFactor.value) <= cpprev->originImmediateFinalVelocity) {
//					long double diff;
//					long double origCpTime;
//					float length;
//					cameraPoint_t* cptmp;
//
//					cpprev->useOriginVelocity = qfalse;
//					cp->useOriginVelocity = qfalse;
//					cpnext->useOriginVelocity = qfalse;
//
//					//cp->originInitialVelocity = cp->originAvgVelocity * 2.0;
//					VectorSubtract(cpprev->origin, cp->origin, dir);
//					length = VectorNormalize(dir);
//					VectorMA(cp->origin, length * 0.1, dir, newOrigin);
//
//					Com_Printf("%d origin:  (%f %f %f) to (%f %f %f)  -->  (%f %f %f)\n", i, cp->origin[0], cp->origin[1], cp->origin[2], newOrigin[0], newOrigin[1], newOrigin[2], cpprev->origin[0], cpprev->origin[1], cpprev->origin[2]);
//					Com_Printf("  velocity: %f(imm) * %f(smooth)  <  %f(prev imm final)\n", cp->originImmediateInitialVelocity, cg_cameraSmoothFactor.value, cpprev->originImmediateFinalVelocity);
//
//					VectorCopy(newOrigin, cp->origin);
//					//VectorMA(cpprev->origin, -(cpprev->originDistance * 0.1), dir, cpprev->origin);
//
//					//diff = cp->cgtime - cpprev->cgtime;
//					diff = cpnext->cgtime - cp->cgtime;
//					origCpTime = cp->cgtime;
//
//					cpnext->cgtime -= (diff * 0.1);
//
//					//diff = cp->cgtime - cpprev->cgtime;
//					cp->cgtime += (diff * 0.1);
//
//					cptmp = cp->next;
//					while (cptmp && !(cptmp->flags & CAM_ORIGIN)) {
//						long double origFrac;
//
//						origFrac = (cptmp->cgtime - origCpTime) / diff;
//						cptmp->cgtime = origFrac * (cpnext->cgtime - cp->cgtime) + cp->cgtime;
//					}
//
//
//#if 0
//					//FIXME 2015-09-04  why move this one?
//
//					diff = cp->cgtime - cpprev->cgtime;
//					//FIXME what if cpprev->cgtime now less than cpprev->prev->cgtime ?
//					cpprev->cgtime -= (diff * 0.1);
//#endif
//
//					CG_UpdateCameraInfo();
//				}
//				else {
//					break;
//				}
//			}
//			//cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//			//CG_UpdateCameraInfo();
//		}
//
//		CG_CameraResetInternalLengths();
//		CG_UpdateCameraInfo();
//		return;
//	}  // end '/ecam smooth originf'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "angles")) {
//		int start, end;
//
//		start = cg.selectedCameraPointMin;
//		end = cg.selectedCameraPointMax;
//
//		if (start < 1) {
//			start = 1;
//		}
//		if (end > cg.numCameraPoints - 1) {
//			end = cg.numCameraPoints - 1;
//		}
//
//		for (i = start; i <= end; i++) {
//			long double scale;
//			long double final, initial;
//
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ANGLES)) {
//				continue;
//			}
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ANGLES)) {
//				cpprev = cpprev->prev;
//			}
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ANGLES)) {
//				cpnext = cpnext->next;
//			}
//
//			if (cpprev == NULL || cpnext == NULL) {
//				continue;
//			}
//
//			if (cpprev->useAnglesVelocity) {
//				final = cpprev->anglesFinalVelocity;
//			}
//			else {
//				final = cpprev->anglesAvgVelocity;
//			}
//
//			if (cp->useAnglesVelocity) {
//				initial = cp->anglesInitialVelocity;
//			}
//			else {
//				initial = cp->anglesAvgVelocity;
//			}
//
//			if (initial <= 0.001) {
//				initial = 0.01;
//				//initial = 1;
//			}
//
//			scale = final / initial;
//
//			if (cp->useAnglesVelocity) {
//				cp->anglesInitialVelocity *= scale;
//			}
//			else {
//				cp->useAnglesVelocity = qtrue;
//				cp->anglesInitialVelocity = cp->anglesAvgVelocity * scale;
//			}
//			Com_Printf("%d  scale %f (%f / %f)  vel %f\n", i, (double)scale, (double)cpprev->anglesFinalVelocity, (double)cp->anglesInitialVelocity, (double)cp->anglesAvgVelocity);
//
//			if (cp->anglesInitialVelocity < 0.0) {
//				//if (cp->anglesInitialVelocity < cp->anglesAvgVelocity / 2.0) {
//				Com_Printf("^3angle velocity is less than zero: couldn't set correct initial velocity (%f)\n", (double)cp->anglesInitialVelocity);
//				cp->anglesInitialVelocity = 0;
//				//cp->anglesInitialVelocity = cp->anglesAvgVelocity / 2.0;
//			}
//
//			if (cp->anglesInitialVelocity > cp->anglesAvgVelocity * cg_cameraSmoothFactor.value) {
//				Com_Printf("^3angle velocity is greater than %f * avgVelocity:  couldn't set correct initial velocity (%f)\n", cg_cameraSmoothFactor.value, (double)cp->anglesInitialVelocity);
//				cp->anglesInitialVelocity = cp->anglesAvgVelocity * cg_cameraSmoothFactor.value;
//			}
//
//			cp->anglesFinalVelocity = 2.0 * cp->anglesDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesInitialVelocity;
//			CG_UpdateCameraInfo();
//		}
//
//		return;
//	}  // end '/ecam smooth angles'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "anglesf")) {
//		int start, end;
//		int totalRuns;
//
//		start = cg.selectedCameraPointMin;
//		end = cg.selectedCameraPointMax;
//
//		if (start < 1) {
//			start = 1;
//		}
//		if (end > cg.numCameraPoints - 1) {
//			end = cg.numCameraPoints - 1;
//		}
//
//		for (totalRuns = 0; totalRuns < 10; totalRuns++) {
//			int numScaled;
//
//			numScaled = 0;
//			for (i = start; i <= end; i++) {
//				vec3_t newAngles;
//				int runs;
//				long double scale;
//
//				cp = &cg.cameraPoints[i];
//				if (!(cp->flags & CAM_ANGLES)) {
//					continue;
//				}
//				cpprev = cp->prev;
//				while (cpprev && !(cpprev->flags & CAM_ANGLES)) {
//					cpprev = cpprev->prev;
//				}
//				cpnext = cp->next;
//				while (cpnext && !(cpnext->flags & CAM_ANGLES)) {
//					cpnext = cpnext->next;
//				}
//				if (cpprev == NULL || cpnext == NULL) {
//					continue;
//				}
//
//				if (cpprev->anglesAvgVelocity <= 0.0) {
//					//Com_Printf("%d skipping\n", i);
//					continue;
//				}
//
//				scale = cg_cameraSmoothFactor.value;
//				for (runs = 0; runs < 10; runs++) {
//					if (cpprev->anglesAvgVelocity > scale * cp->anglesAvgVelocity) {
//						//CG_ResetCameraVelocities(i);
//
//						newAngles[YAW] = LerpAngle(cp->angles[YAW], cpprev->angles[YAW], 0.1);
//						newAngles[PITCH] = LerpAngle(cp->angles[PITCH], cpprev->angles[PITCH], 0.1);
//						if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//							newAngles[ROLL] = LerpAngle(cp->angles[ROLL], cpprev->angles[ROLL], 0.1);
//						}
//						else {
//							// silence gcc warning
//							newAngles[ROLL] = 0;
//						}
//
//						if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//							Com_Printf("%d:  (%f %f %f) -> (%f %f %f) vel: %f -> %f\n", i, newAngles[YAW], newAngles[PITCH], newAngles[ROLL], cpprev->angles[YAW], cpprev->angles[PITCH], cpprev->angles[ROLL], cp->anglesAvgVelocity, cpprev->anglesAvgVelocity);
//						}
//						else {
//							Com_Printf("%d:  (%f %f) -> (%f %f) vel: %f -> %f\n", i, newAngles[YAW], newAngles[PITCH], cpprev->angles[YAW], cpprev->angles[PITCH], cp->anglesAvgVelocity, cpprev->anglesAvgVelocity);
//						}
//
//						cp->angles[PITCH] = newAngles[PITCH];
//						cp->angles[YAW] = newAngles[YAW];
//						if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//							cp->angles[ROLL] = newAngles[ROLL];
//						}
//						cp->useAnglesVelocity = qfalse;
//
//						newAngles[YAW] = LerpAngle(cpprev->angles[YAW], cp->angles[YAW], 0.1);
//						newAngles[PITCH] = LerpAngle(cpprev->angles[PITCH], cp->angles[PITCH], 0.1);
//						if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//							newAngles[ROLL] = LerpAngle(cpprev->angles[ROLL], cp->angles[ROLL], 0.1);
//						}
//						cpprev->angles[YAW] = newAngles[YAW];
//						cpprev->angles[PITCH] = newAngles[PITCH];
//						if (cp->viewType == CAMERA_ANGLES_SPLINE) {
//							cpprev->angles[ROLL] = newAngles[ROLL];
//						}
//						cpprev->useAnglesVelocity = qfalse;
//
//						CG_UpdateCameraInfo();
//						numScaled++;
//					}
//					else {
//						break;
//					}
//				}
//			}
//
//			if (numScaled == 0) {
//				break;
//			}
//		}
//
//		CG_UpdateCameraInfo();
//		return;
//	}  // end '/ecam smooth anglesf'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "avgvelocity")) {
//		double distance = 0;
//		double avgSpeed;
//		int runs;
//		int pointMin, pointMax;
//
//		pointMin = cg.selectedCameraPointMin;
//		cp = &cg.cameraPoints[pointMin];
//		while (cp && !(cp->flags & CAM_ORIGIN)) {
//			cp = cp->next;
//			pointMin++;
//		}
//		if (cp == NULL) {
//			Com_Printf("invalid starting point\n");
//			return;
//		}
//		pointMax = cg.selectedCameraPointMax;
//		cp = &cg.cameraPoints[pointMax];
//		while (cp && !(cp->flags & CAM_ORIGIN)) {
//			cp = cp->prev;
//			pointMax--;
//		}
//		if (cp == NULL) {
//			Com_Printf("invalid ending point\n");
//			return;
//		}
//		if (pointMin >= pointMax) {
//			Com_Printf("invalid camera point range\n");
//			return;
//		}
//
//		for (runs = 0; runs < 30; runs++) {
//
//			distance = 0;
//
//			for (i = pointMin; i <= pointMax; i++) {
//				cp = &cg.cameraPoints[i];
//				if (!(cp->flags & CAM_ORIGIN)) {
//					continue;
//				}
//				distance += cp->originDistance;
//			}
//			if (distance <= 0.0) {
//				return;
//			}
//
//			avgSpeed = distance / ((cg.cameraPoints[pointMax].cgtime - cg.cameraPoints[pointMin].cgtime) / 1000.0);
//			if (avgSpeed <= 0.0) {
//				return;
//			}
//
//			for (i = pointMin; i < pointMax; i++) {
//				double newTime;
//
//				cp = &cg.cameraPoints[i];
//				if (!(cp->flags & CAM_ORIGIN)) {
//					continue;
//				}
//				cpnext = cp->next;
//				while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//					cpnext = cpnext->next;
//				}
//				if (cpnext == NULL) {
//					break;
//				}
//
//				newTime = (cp->originDistance + avgSpeed * (cp->cgtime / 1000.0)) / avgSpeed;
//				cpnext->cgtime = newTime * 1000.0;
//			}
//
//			CG_UpdateCameraInfo();
//		}
//
//		for (i = pointMin; i < pointMax; i++) {
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//			Com_Printf("cam %d  vel %f  %f -> %f\n", i, (double)cp->originAvgVelocity, (double)cp->originInitialVelocity, (double)cp->originFinalVelocity);
//		}
//
//		return;
//	}  // end '/ecam smooth avgvelocity'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth") && !Q_stricmp(CG_Argv(2), "velocity")) {
//		long double ctime;
//		long double scale;
//		long double diff;
//		double oldVelocity;
//		int start;
//		int end;
//
//		start = cg.selectedCameraPointMin;
//		if (start < 1) {
//			start = 1;
//		}
//
//		end = cg.selectedCameraPointMax;
//		if (end > cg.numCameraPoints - 2) {  // need cpnext
//			end = cg.numCameraPoints - 2;
//		}
//
//		for (i = start; i <= end; i++) {
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//				cpprev = cpprev->prev;
//			}
//			if (cpprev == NULL) {
//				break;
//			}
//
//			cpnext = cp->next;
//			while (cpnext && !(cpnext->flags & CAM_ORIGIN)) {
//				cpnext = cpnext->next;
//			}
//			if (cpnext == NULL) {
//				break;
//			}
//
//			oldVelocity = cp->originImmediateInitialVelocity;
//
//			ctime = cpnext->cgtime - cp->cgtime;
//			if (cpprev->originImmediateFinalVelocity > 0.0) {
//				scale = cp->originImmediateInitialVelocity / cpprev->originImmediateFinalVelocity;
//			}
//			else {
//				scale = 0;
//			}
//
//			if (scale > 1.0) {
//				diff = (ctime * scale) - ctime;
//			}
//			else if (scale < 1.0 && scale > 0.001) {
//				//diff = ctime - (ctime * (1.0 / scale));
//				diff = -(1.0 - scale) * ctime;
//			}
//			else {
//				diff = 0;
//				continue;
//			}
//
//			for (j = i + 1; j < cg.numCameraPoints - 1; j++) {
//				cg.cameraPoints[j].cgtime += diff;
//			}
//			CG_UpdateCameraInfo();
//			Com_Printf("^3%d  scale %f   %f -> %f\n", i, (double)scale, oldVelocity, cp->originImmediateInitialVelocity);
//		}
//
//		CG_UpdateCameraInfo();
//
//		for (i = 1; i < cg.numCameraPoints - 1; i++) {
//			cp = &cg.cameraPoints[i];
//			if (!(cp->flags & CAM_ORIGIN)) {
//				continue;
//			}
//			cpprev = cp->prev;
//			while (cpprev && !(cpprev->flags & CAM_ORIGIN)) {
//				cpprev = cpprev->prev;
//			}
//			if (cpprev == NULL) {
//				break;
//			}
//
//			Com_Printf("%d  ours %f  prev %f\n", i, (double)cp->originImmediateInitialVelocity, (double)cpprev->originImmediateFinalVelocity);
//		}
//
//		return;
//	}  // end '/ecam smooth velocity'
//
//	if (!Q_stricmp(CG_Argv(1), "smooth")) {
//		Com_Printf("usage: ecam smooth [origin | originf | angles | anglesf | avgvelocity | velocity | time]\n");
//		return;
//	}
//
//	// set camera point values
//
//	for (i = cg.selectedCameraPointMin; i <= cg.selectedCameraPointMax; i++) {
//		cp = &cg.cameraPoints[i];
//
//		if (i > 0) {
//			cpprev = &cg.cameraPoints[i - 1];
//		}
//		else {
//			cpprev = NULL;
//		}
//
//		if (i < (cg.numCameraPoints - 1)) {
//			cpnext = &cg.cameraPoints[i + 1];
//		}
//		else {
//			cpnext = NULL;
//		}
//
//		j = 1;
//		s = CG_Argv(j);
//
//		if (!Q_stricmp(s, "type")) {
//			s = CG_Argv(j + 1);
//			if (!Q_stricmp(s, "spline")) {
//				cp->type = CAMERA_SPLINE;
//			}
//			else if (!Q_stricmp(s, "interp")) {
//				cp->type = CAMERA_INTERP;
//			}
//			else if (!Q_stricmp(s, "jump")) {
//				cp->type = CAMERA_JUMP;
//			}
//			else if (!Q_stricmp(s, "curve")) {
//				cp->type = CAMERA_CURVE;
//			}
//			else if (!Q_stricmp(s, "splinebezier")) {
//				cp->type = CAMERA_SPLINE_BEZIER;
//			}
//			else if (!Q_stricmp(s, "splinecatmullrom")) {
//				cp->type = CAMERA_SPLINE_CATMULLROM;
//			}
//			else {
//				Com_Printf("unknown camera type\n");
//				Com_Printf("valid types:  spline interp jump curve splinebezier splinecatmullrom\n");
//				return;
//			}
//		}
//		else if (!Q_stricmp(s, "fov")) {
//			s = CG_Argv(j + 1);
//			if (!Q_stricmp(s, "current")) {
//				cp->fovType = CAMERA_FOV_USE_CURRENT;
//			}
//			else if (!Q_stricmp(s, "interp")) {
//				cp->fovType = CAMERA_FOV_INTERP;
//			}
//			else if (!Q_stricmp(s, "fixed")) {
//				cp->fovType = CAMERA_FOV_FIXED;
//			}
//			else if (!Q_stricmp(s, "pass")) {
//				cp->fovType = CAMERA_FOV_PASS;
//			}
//			else if (!Q_stricmp(s, "spline")) {
//				cp->fovType = CAMERA_FOV_SPLINE;
//			}
//			else {
//				Com_Printf("unknown fov type\n");
//				Com_Printf("valid types:  current interp fixed pass spline\n");
//				Com_Printf("current fov: %f\n", cp->fov);
//				Com_Printf("usage: ecam fov <fov type> [fov value]\n");
//				return;
//			}
//			if (!*CG_Argv(j + 2)) {
//				continue;
//			}
//			cp->fov = atof(CG_Argv(j + 2));
//			if (cp->fov <= 0.0) {
//				cp->fov = -1;
//			}
//		}
//		else if (!Q_stricmp(s, "command")) {
//			cp->command[0] = '\0';
//			j++;
//			while (CG_Argv(j)[0]) {
//				Q_strcat(cp->command, sizeof(cp->command), va("%s ", CG_Argv(j)));
//				j++;
//			}
//		}
//		else if (!Q_stricmp(s, "numsplines")) {
//			s = CG_Argv(j + 1);
//			if (!(s && *s)) {
//				//cp->numSplines = 40;  //FIXME define
//				Com_Printf("current numsplines: %d\n", cp->numSplines);
//				Com_Printf("usage: ecam numsplines <value>\n");
//				return;
//			}
//			else {
//				int ns;
//
//				ns = atoi(s);
//				if (ns < 2) {
//					Com_Printf("invalid number of splines, can't be less than 2\n");
//					return;
//				}
//				else {
//					cp->numSplines = ns;
//				}
//			}
//		}
//		else if (!Q_stricmp(s, "angles")) {
//			s = CG_Argv(j + 1);
//			if (!Q_stricmp(s, "viewpointinterp")) {
//				if (!cg.viewPointMarkSet) {
//					Com_Printf("viewpointmark isn't set\n");
//					return;
//				}
//				//FIXME if multiple cam points selected transition them
//				VectorCopy(cg.viewPointMarkOrigin, cp->viewPointOrigin);
//				cp->viewType = CAMERA_ANGLES_VIEWPOINT_INTERP;
//			}
//			else if (!Q_stricmp(s, "viewpointfixed")) {
//				if (!cg.viewPointMarkSet) {
//					Com_Printf("viewpointmark isn't set\n");
//					return;
//				}
//				//FIXME if multiple cam points selected transition them
//				VectorCopy(cg.viewPointMarkOrigin, cp->viewPointOrigin);
//				cp->viewType = CAMERA_ANGLES_VIEWPOINT_FIXED;
//			}
//			else if (!Q_stricmp(s, "viewpointpass")) {
//				cp->viewType = CAMERA_ANGLES_VIEWPOINT_PASS;
//			}
//			else if (!Q_stricmp(s, "ent")) {
//				cp->viewType = CAMERA_ANGLES_ENT;
//				s = CG_Argv(j + 2);
//				if (*s) {
//					cp->viewEnt = atoi(s);
//				}
//				else {
//					Com_Printf("value for ent not specified\n");
//					Com_Printf("current value: %d\n", cp->viewEnt);
//					return;
//				}
//			}
//			else if (!Q_stricmp(s, "interp")) {
//				cp->viewType = CAMERA_ANGLES_INTERP;
//			}
//			else if (!Q_stricmp(s, "spline")) {
//				cp->viewType = CAMERA_ANGLES_SPLINE;
//			}
//			else if (!Q_stricmp(s, "interpuseprevious")) {
//				cp->viewType = CAMERA_ANGLES_INTERP_USE_PREVIOUS;
//			}
//			else if (!Q_stricmp(s, "fixed")) {
//				cp->viewType = CAMERA_ANGLES_FIXED;
//			}
//			else if (!Q_stricmp(s, "fixeduseprevious")) {
//				cp->viewType = CAMERA_ANGLES_FIXED_USE_PREVIOUS;
//			}
//			else {
//				Com_Printf("unknown setting for angles '%s'\n", s);
//				Com_Printf("valid types:  interp spline fixed fixeduseprevious interpuseprevious ent viewpointpass viewpointfixed viewpointinterp\n");
//				return;
//			}
//		}
//		else if (!Q_stricmp(s, "roll")) {
//			j++;
//			s = CG_Argv(j);
//			if (!Q_stricmp(s, "interp")) {
//				cp->rollType = CAMERA_ROLL_INTERP;
//			}
//			else if (!Q_stricmp(s, "fixed")) {
//				cp->rollType = CAMERA_ROLL_FIXED;
//			}
//			else if (!Q_stricmp(s, "pass")) {
//				cp->rollType = CAMERA_ROLL_PASS;
//			}
//			else if (!Q_stricmp(s, "angles")) {
//				cp->rollType = CAMERA_ROLL_AS_ANGLES;
//			}
//			else {
//				Com_Printf("unknown setting for roll '%s'\n", s);
//				Com_Printf("valid values:  interp fixed pass angles\n");
//				Com_Printf("current roll value: %f\n", cp->angles[ROLL]);
//				Com_Printf("usage: ecam roll <roll type> [roll value]\n");
//				return;
//			}
//			j++;
//			s = CG_Argv(j);
//			if (*s) {
//				cp->angles[ROLL] = atof(s);
//			}
//		}
//		else if (!Q_stricmp(s, "flags")) {
//			int k;
//
//			for (k = 2; k < CG_Argc(); k++) {
//				if (!Q_stricmp(CG_Argv(k), "origin")) {
//					cp->flags ^= CAM_ORIGIN;
//				}
//				else if (!Q_stricmp(CG_Argv(k), "angles")) {
//					cp->flags ^= CAM_ANGLES;
//				}
//				else if (!Q_stricmp(CG_Argv(k), "fov")) {
//					cp->flags ^= CAM_FOV;
//				}
//				else if (!Q_stricmp(CG_Argv(k), "time")) {
//					cp->flags ^= CAM_TIME;
//				}
//				else {
//					Com_Printf("unknown flag type: '%s'\n", CG_Argv(k));
//				}
//			}
//
//			// print flag values
//			Com_Printf("[%d] flags: ", i);
//			if (cp->flags & CAM_ORIGIN) {
//				Com_Printf("origin ");
//			}
//			if (cp->flags & CAM_ANGLES) {
//				Com_Printf("angles ");
//			}
//			if (cp->flags & CAM_FOV) {
//				Com_Printf("fov ");
//			}
//			if (cp->flags & CAM_TIME) {
//				Com_Printf("time ");
//			}
//			Com_Printf("\n");
//		}
//		else if (!Q_stricmp(s, "offset")) {
//			j++;
//			s = CG_Argv(j);
//			if (!Q_stricmp(s, "interp")) {
//				cp->offsetType = CAMERA_OFFSET_INTERP;
//			}
//			else if (!Q_stricmp(s, "fixed")) {
//				cp->offsetType = CAMERA_OFFSET_FIXED;
//			}
//			else if (!Q_stricmp(s, "pass")) {
//				cp->offsetType = CAMERA_OFFSET_PASS;
//			}
//			else {
//				Com_Printf("uknown offset type '%s'\n", s);
//				Com_Printf("valid values:  interp fixed pass\n");
//				Com_Printf("current values: x: %f  y: %f  z: %f\n", cp->xoffset, cp->yoffset, cp->zoffset);
//				Com_Printf("usage: ecam offset <offset type> [xoffset] [yoffset] [zoffset]\n");
//				return;
//			}
//			j++;
//			s = CG_Argv(j);
//			if (*CG_Argv(j)) {
//				cp->xoffset = atof(CG_Argv(j));
//				if (*CG_Argv(j + 1)) {
//					cp->yoffset = atof(CG_Argv(j + 1));
//					if (*CG_Argv(j + 2)) {
//						cp->zoffset = atof(CG_Argv(j + 2));
//					}
//				}
//			}
//		}
//		else if (!Q_stricmp(s, "initialVelocity")) {
//			j++;
//			s = CG_Argv(j);
//			if (!Q_stricmp(s, "origin")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (!Q_stricmp(s, "reset")) {
//						cp->useOriginVelocity = qfalse;
//						continue;
//					}
//					else {
//						cp->useOriginVelocity = qtrue;
//					}
//					cp->originInitialVelocity = atof(s);
//					if (cp->originInitialVelocity <= 0.0) {
//						cp->originInitialVelocity = 0;
//					}
//
//					if (cp->originInitialVelocity > cp->originAvgVelocity * 2.0) {
//						cp->originInitialVelocity = cp->originAvgVelocity * 2.0;
//					}
//
//					cp->originFinalVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originInitialVelocity;
//					//Com_Printf("%f %f\n", cp->originInitialVelocity, cp->originFinalVelocity);
//				}
//				else {  // not given
//					Com_Printf("[%d] origin initial velocity value:  %f\n", i, cp->originInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "angles")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->viewPointPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->viewPointPassStart];
//						cend = &cg.cameraPoints[cp->viewPointPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useAnglesVelocity = qfalse;
//						if (cstart) {
//							cstart->useAnglesVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useAnglesVelocity = qtrue;
//					}
//					cp->anglesInitialVelocity = atof(s);
//					if (cp->anglesInitialVelocity <= 0.0) {
//						cp->anglesInitialVelocity = 0;
//					}
//					if (cp->viewType != CAMERA_ANGLES_INTERP_USE_PREVIOUS) {
//						if (cp->anglesInitialVelocity > cp->anglesAvgVelocity * 2.0) {
//							cp->anglesInitialVelocity = cp->anglesAvgVelocity * 2.0;
//						}
//					}
//					if (cstart) {
//						VectorCopy(cstart->viewPointOrigin, angs0);
//						VectorCopy(cg.cameraPoints[cp->viewPointPassEnd].viewPointOrigin, angs1);
//						cp->anglesFinalVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(angs0, angs1) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->anglesInitialVelocity;
//					}
//					else if (cp->viewType == CAMERA_ANGLES_VIEWPOINT_INTERP) {
//						cp->anglesFinalVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(cp->viewPointOrigin, cpnext->viewPointOrigin) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesInitialVelocity;
//					}
//					else {
//						VectorCopy(cp->angles, angs0);
//						VectorCopy(cpnext->angles, angs1);
//						angs0[ROLL] = 0;
//						angs1[ROLL] = 0;
//
//						cp->anglesFinalVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(angs0, angs1) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesInitialVelocity;
//					}
//
//					if (cstart) {
//						cstart->useAnglesVelocity = qtrue;
//						cstart->anglesInitialVelocity = cp->anglesInitialVelocity;
//						cstart->anglesFinalVelocity = cp->anglesFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] angles initial velocity value:  %f\n", i, cp->anglesInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "xoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useXoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useXoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useXoffsetVelocity = qtrue;
//					}
//					cp->xoffsetInitialVelocity = atof(s);
//					if (cp->xoffsetInitialVelocity <= 0.0) {
//						cp->xoffsetInitialVelocity = 0;
//					}
//					if (cp->xoffsetInitialVelocity > cp->xoffsetAvgVelocity * 2.0) {
//						cp->xoffsetInitialVelocity = cp->xoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->xoffsetFinalVelocity = 2.0 * fabs(cend->xoffset - cstart->xoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->xoffsetInitialVelocity;
//					}
//					else {
//						cp->xoffsetFinalVelocity = 2.0 * fabs(cpnext->xoffset - cp->xoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->xoffsetInitialVelocity;
//					}
//					if (cstart) {
//						cstart->useXoffsetVelocity = qtrue;
//						cstart->xoffsetInitialVelocity = cp->xoffsetInitialVelocity;
//						cstart->xoffsetFinalVelocity = cp->xoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] xoffset initial velocity value:  %f\n", i, cp->xoffsetInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "yoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useYoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useYoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useYoffsetVelocity = qtrue;
//					}
//					cp->yoffsetInitialVelocity = atof(s);
//					if (cp->yoffsetInitialVelocity <= 0.0) {
//						cp->yoffsetInitialVelocity = 0;
//					}
//					if (cp->yoffsetInitialVelocity > cp->yoffsetAvgVelocity * 2.0) {
//						cp->yoffsetInitialVelocity = cp->yoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->yoffsetFinalVelocity = 2.0 * fabs(cend->yoffset - cstart->yoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->yoffsetInitialVelocity;
//					}
//					else {
//						cp->yoffsetFinalVelocity = 2.0 * fabs(cpnext->yoffset - cp->yoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->yoffsetInitialVelocity;
//					}
//					if (cstart) {
//						cstart->useYoffsetVelocity = qtrue;
//						cstart->yoffsetInitialVelocity = cp->yoffsetInitialVelocity;
//						cstart->yoffsetFinalVelocity = cp->yoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] yoffset initial velocity value:  %f\n", i, cp->yoffsetInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "zoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useZoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useZoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useZoffsetVelocity = qtrue;
//					}
//					cp->zoffsetInitialVelocity = atof(s);
//					if (cp->zoffsetInitialVelocity <= 0.0) {
//						cp->zoffsetInitialVelocity = 0;
//					}
//					if (cp->zoffsetInitialVelocity > cp->zoffsetAvgVelocity * 2.0) {
//						cp->zoffsetInitialVelocity = cp->zoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->zoffsetFinalVelocity = 2.0 * fabs(cend->zoffset - cstart->zoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->zoffsetInitialVelocity;
//					}
//					else {
//						cp->zoffsetFinalVelocity = 2.0 * fabs(cpnext->zoffset - cp->zoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->zoffsetInitialVelocity;
//					}
//					if (cstart) {
//						cstart->useZoffsetVelocity = qtrue;
//						cstart->zoffsetInitialVelocity = cp->zoffsetInitialVelocity;
//						cstart->zoffsetFinalVelocity = cp->zoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] zoffset initial velocity value:  %f\n", i, cp->zoffsetInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "fov")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->fovPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->fovPassStart];
//						cend = &cg.cameraPoints[cp->fovPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useFovVelocity = qfalse;
//						if (cstart) {
//							cstart->useFovVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useFovVelocity = qtrue;
//					}
//					cp->fovInitialVelocity = atof(s);
//					if (cp->fovInitialVelocity <= 0.0) {
//						cp->fovInitialVelocity = 0;
//					}
//					if (cp->fovInitialVelocity > cp->fovAvgVelocity * 2.0) {
//						cp->fovInitialVelocity = cp->fovAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->fovFinalVelocity = 2.0 * fabs(cend->fov - cstart->fov) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->fovInitialVelocity;
//					}
//					else {
//						cp->fovFinalVelocity = 2.0 * fabs(cpnext->fov - cp->fov) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->fovInitialVelocity;
//					}
//					if (cstart) {
//						cstart->useFovVelocity = qtrue;
//						cstart->fovInitialVelocity = cp->fovInitialVelocity;
//						cstart->fovFinalVelocity = cp->fovFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] fov initial velocity value:  %f\n", i, cp->fovInitialVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "roll")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->rollPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->rollPassStart];
//						cend = &cg.cameraPoints[cp->rollPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useRollVelocity = qfalse;
//						if (cstart) {
//							cstart->useRollVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useRollVelocity = qtrue;
//					}
//					cp->rollInitialVelocity = atof(s);
//					if (cp->rollInitialVelocity <= 0.0) {
//						cp->rollInitialVelocity = 0;
//					}
//					if (cp->rollInitialVelocity > cp->rollAvgVelocity * 2.0) {
//						cp->rollInitialVelocity = cp->rollAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->rollFinalVelocity = 2.0 * fabs(AngleSubtract(cend->angles[ROLL], cstart->angles[ROLL])) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->rollInitialVelocity;
//					}
//					else {
//						cp->rollFinalVelocity = 2.0 * fabs(AngleSubtract(cpnext->angles[ROLL], cp->angles[ROLL])) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->rollInitialVelocity;
//					}
//					if (cstart) {
//						cstart->useRollVelocity = qtrue;
//						cstart->rollInitialVelocity = cp->rollInitialVelocity;
//						cstart->rollFinalVelocity = cp->rollFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] roll initial velocity value:  %f\n", i, cp->rollInitialVelocity);
//				}
//			}
//			else {
//				Com_Printf("unknown initial velocity type '%s'\n", s);
//				Com_Printf("valid values:  origin angles xoffset yoffset zoffset fov roll\n");
//				Com_Printf("usage: ecam initialVelocity <type> <value or 'reset'>\n");
//				return;
//			}
//		}
//		else if (!Q_stricmp(s, "finalVelocity")) {
//			j++;
//			s = CG_Argv(j);
//			if (!Q_stricmp(s, "origin")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (!Q_stricmp(s, "reset")) {
//						cp->useOriginVelocity = qfalse;
//						continue;
//					}
//					else {
//						cp->useOriginVelocity = qtrue;
//					}
//					cp->originFinalVelocity = atof(s);
//					if (cp->originFinalVelocity <= 0.0) {
//						cp->originFinalVelocity = 0;
//					}
//					if (cp->originFinalVelocity > cp->originAvgVelocity * 2.0) {
//						cp->originFinalVelocity = cp->originAvgVelocity * 2.0;
//					}
//					cp->originInitialVelocity = 2.0 * cp->originDistance / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->originFinalVelocity;
//				}
//				else {  // not given
//					Com_Printf("[%d] origin final velocity value:  %f\n", i, cp->originFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "angles")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->viewPointPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->viewPointPassStart];
//						cend = &cg.cameraPoints[cp->viewPointPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useAnglesVelocity = qfalse;
//						if (cstart) {
//							cstart->useAnglesVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useAnglesVelocity = qtrue;
//					}
//					cp->anglesFinalVelocity = atof(s);
//					if (cp->anglesFinalVelocity <= 0.0) {
//						cp->anglesFinalVelocity = 0;
//					}
//					if (cp->viewType != CAMERA_ANGLES_INTERP_USE_PREVIOUS) {
//						if (cp->anglesFinalVelocity > cp->anglesAvgVelocity * 2.0) {
//							cp->anglesFinalVelocity = cp->anglesAvgVelocity * 2.0;
//						}
//					}
//					if (cstart) {
//						VectorCopy(cstart->viewPointOrigin, angs0);
//						VectorCopy(cg.cameraPoints[cp->viewPointPassEnd].viewPointOrigin, angs1);
//						cp->anglesInitialVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(angs0, angs1) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->anglesFinalVelocity;
//					}
//					else if (cp->viewType == CAMERA_ANGLES_VIEWPOINT_INTERP) {
//						cp->anglesInitialVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(cp->viewPointOrigin, cpnext->viewPointOrigin) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesFinalVelocity;
//					}
//					else {
//						VectorCopy(cp->angles, angs0);
//						VectorCopy(cpnext->angles, angs1);
//						angs0[ROLL] = 0;
//						angs1[ROLL] = 0;
//
//						cp->anglesInitialVelocity = 2.0 * CG_CameraAngleLongestDistanceNoRoll(angs0, angs1) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->anglesFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useAnglesVelocity = qtrue;
//						cstart->anglesInitialVelocity = cp->anglesInitialVelocity;
//						cstart->anglesFinalVelocity = cp->anglesFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] angles final velocity value:  %f\n", i, cp->anglesFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "xoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useXoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useXoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useXoffsetVelocity = qtrue;
//					}
//					cp->xoffsetFinalVelocity = atof(s);
//					if (cp->xoffsetFinalVelocity <= 0.0) {
//						cp->xoffsetFinalVelocity = 0;
//					}
//					if (cp->xoffsetFinalVelocity > cp->xoffsetAvgVelocity * 2.0) {
//						cp->xoffsetFinalVelocity = cp->xoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->xoffsetInitialVelocity = 2.0 * fabs(cend->xoffset - cstart->xoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->xoffsetFinalVelocity;
//					}
//					else {
//						cp->xoffsetInitialVelocity = 2.0 * fabs(cpnext->xoffset - cp->xoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->xoffsetFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useXoffsetVelocity = qtrue;
//						cstart->xoffsetInitialVelocity = cp->xoffsetInitialVelocity;
//						cstart->xoffsetFinalVelocity = cp->xoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] xoffset final velocity value:  %f\n", i, cp->xoffsetFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "yoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useYoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useYoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useYoffsetVelocity = qtrue;
//					}
//					cp->yoffsetFinalVelocity = atof(s);
//					if (cp->yoffsetFinalVelocity <= 0.0) {
//						cp->yoffsetFinalVelocity = 0;
//					}
//					if (cp->yoffsetFinalVelocity > cp->yoffsetAvgVelocity * 2.0) {
//						cp->yoffsetFinalVelocity = cp->yoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->yoffsetInitialVelocity = 2.0 * fabs(cend->yoffset - cstart->yoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->yoffsetFinalVelocity;
//					}
//					else {
//						cp->yoffsetInitialVelocity = 2.0 * fabs(cpnext->yoffset - cp->yoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->yoffsetFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useYoffsetVelocity = qtrue;
//						cstart->yoffsetInitialVelocity = cp->yoffsetInitialVelocity;
//						cstart->yoffsetFinalVelocity = cp->yoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] yoffset final velocity value:  %f\n", i, cp->yoffsetFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "zoffset")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->offsetPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->offsetPassStart];
//						cend = &cg.cameraPoints[cp->offsetPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useZoffsetVelocity = qfalse;
//						if (cstart) {
//							cstart->useZoffsetVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useZoffsetVelocity = qtrue;
//					}
//					cp->zoffsetFinalVelocity = atof(s);
//					if (cp->zoffsetFinalVelocity <= 0.0) {
//						cp->zoffsetFinalVelocity = 0;
//					}
//					if (cp->zoffsetFinalVelocity > cp->zoffsetAvgVelocity * 2.0) {
//						cp->zoffsetFinalVelocity = cp->zoffsetAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->zoffsetInitialVelocity = 2.0 * fabs(cend->zoffset - cstart->zoffset) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->zoffsetFinalVelocity;
//					}
//					else {
//						cp->zoffsetInitialVelocity = 2.0 * fabs(cpnext->zoffset - cp->zoffset) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->zoffsetFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useZoffsetVelocity = qtrue;
//						cstart->zoffsetInitialVelocity = cp->zoffsetInitialVelocity;
//						cstart->zoffsetFinalVelocity = cp->zoffsetFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] zoffset final velocity value:  %f\n", i, cp->zoffsetFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "fov")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->fovPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->fovPassStart];
//						cend = &cg.cameraPoints[cp->fovPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useFovVelocity = qfalse;
//						if (cstart) {
//							cstart->useFovVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useFovVelocity = qtrue;
//					}
//					cp->fovFinalVelocity = atof(s);
//					if (cp->fovFinalVelocity <= 0.0) {
//						cp->fovFinalVelocity = 0;
//					}
//					if (cp->fovFinalVelocity > cp->fovAvgVelocity * 2.0) {
//						cp->fovFinalVelocity = cp->fovAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->fovInitialVelocity = 2.0 * fabs(cend->fov - cstart->fov) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->fovFinalVelocity;
//					}
//					else {
//						cp->fovInitialVelocity = 2.0 * fabs(cpnext->fov - cp->fov) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->fovFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useFovVelocity = qtrue;
//						cstart->fovInitialVelocity = cp->fovInitialVelocity;
//						cstart->fovFinalVelocity = cp->fovFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] fov final velocity value:  %f\n", i, cp->fovFinalVelocity);
//				}
//			}
//			else if (!Q_stricmp(s, "roll")) {
//				j++;
//				s = CG_Argv(j);
//				if (*s && cpnext) {
//					if (cp->rollPassStart > -1) {
//						cstart = &cg.cameraPoints[cp->rollPassStart];
//						cend = &cg.cameraPoints[cp->rollPassEnd];
//					}
//					else {
//						cstart = NULL;
//						cend = NULL;
//					}
//					if (!Q_stricmp(s, "reset")) {
//						cp->useRollVelocity = qfalse;
//						if (cstart) {
//							cstart->useRollVelocity = qfalse;
//						}
//						continue;
//					}
//					else {
//						cp->useRollVelocity = qtrue;
//					}
//					cp->rollFinalVelocity = atof(s);
//					if (cp->rollFinalVelocity <= 0.0) {
//						cp->rollFinalVelocity = 0;
//					}
//					if (cp->rollFinalVelocity > cp->rollAvgVelocity * 2.0) {
//						cp->rollFinalVelocity = cp->rollAvgVelocity * 2.0;
//					}
//					if (cstart) {
//						cp->rollInitialVelocity = 2.0 * fabs(AngleSubtract(cend->angles[ROLL], cstart->angles[ROLL])) / ((cend->cgtime - cstart->cgtime) / 1000.0) - cp->rollFinalVelocity;
//					}
//					else {
//						cp->rollInitialVelocity = 2.0 * fabs(AngleSubtract(cpnext->angles[ROLL], cp->angles[ROLL])) / ((cpnext->cgtime - cp->cgtime) / 1000.0) - cp->rollFinalVelocity;
//					}
//					if (cstart) {
//						cstart->useRollVelocity = qtrue;
//						cstart->rollInitialVelocity = cp->rollInitialVelocity;
//						cstart->rollFinalVelocity = cp->rollFinalVelocity;
//					}
//				}
//				else {  // not given
//					Com_Printf("[%d] roll final velocity value:  %f\n", i, cp->rollFinalVelocity);
//				}
//			}
//			else {
//				Com_Printf("unknown final velocity type '%s'\n", s);
//				Com_Printf("valid values:  origin angles xoffset yoffset zoffset fov roll\n");
//				Com_Printf("usage: ecam finalVelocity <type> <value or 'reset'>\n");
//				return;
//			}
//		}
//		else {
//			Com_Printf("unknown setting:  '%s'\n", s);
//			return;
//		}
//	}
//
//	CG_CameraResetInternalLengths();
//	CG_UpdateCameraInfo();
//}
//
//static void CG_SetViewPointMark_f(void)
//{
//	cg.viewPointMarkSet = qtrue;
//	VectorCopy(cg.refdef.vieworg, cg.viewPointMarkOrigin);
//}
//
////static void CG_DeleteViewPointMark_f (void)
////{
////	cg.useViewPointMark = qfalse;
////}
//
//static void CG_GotoViewPointMark_f(void)
//{
//	if (!cg.viewPointMarkSet) {
//		Com_Printf("no viewpoint mark set\n");
//		return;
//	}
//
//	VectorCopy(cg.viewPointMarkOrigin, cg.fpos);
//	VectorCopy(cg.viewPointMarkOrigin, cg.freecamPlayerState.origin);
//	cg.freecamPlayerState.origin[2] -= DEFAULT_VIEWHEIGHT;
//	VectorSet(cg.freecamPlayerState.velocity, 0, 0, 0);
//}
//
//static void CG_PlayQ3mmeCamera_f(void)
//{
//	double extraTime;
//
//	if (cg.playPath) {
//		Com_Printf("can't play camera, path is playing\n");
//		return;
//	}
//
//	if (!demo.camera.points || !(demo.camera.points->next)) {
//		Com_Printf("can't play q3mme camera, need at least 2 camera points\n");
//		return;
//	}
//
//	//FIXME cameraque ?
//	if (1) {  //(cg_cameraQue.integer) {
//		extraTime = 1000.0 * cg_cameraRewindTime.value;
//		if (extraTime < 0) {
//			extraTime = 0;
//		}
//		trap_SendConsoleCommandNow(va("seekservertime %f\n", demo.camera.points->time - extraTime));
//	}
//
//	cg.cameraQ3mmePlaying = qtrue;
//	cg.playQ3mmeCameraCommandIssued = qtrue;
//	//cg.cameraPlaying = qtrue;
//	///cg.cameraPlayedLastFrame = qfalse;
//
//	cg.currentCameraPoint = 0;
//	//cg.cameraWaitToSync = qfalse;  //FIXME stupid
//	cg.playCameraCommandIssued = qtrue;
//
//}
//
//static void CG_StopQ3mmeCamera_f(void)
//{
//	cg.cameraQ3mmePlaying = qfalse;
//	cg.playQ3mmeCameraCommandIssued = qfalse;
//	Com_Printf("stopping q3mme camera\n");
//}
//
//static void CG_SaveQ3mmeCamera_f(void)
//{
//	fileHandle_t f;
//	int i;
//	const char* fname;
//	qboolean useDefaultFolder = qtrue;
//
//	if (CG_Argc() < 2) {
//		Com_Printf("usage: saveq3mmecamera <filename>\n");
//		return;
//	}
//
//	if (!demo.camera.points) {
//		Com_Printf("need at least one camera point\n");
//		return;
//	}
//
//	fname = CG_Argv(1);
//	for (i = 0; i < strlen(fname); i++) {
//		if (fname[i] == '/') {
//			useDefaultFolder = qfalse;
//			break;
//		}
//	}
//
//	if (useDefaultFolder) {
//		trap_FS_FOpenFile(va("cameras/%s.q3mmeCam", CG_Argv(1)), &f, FS_WRITE);
//	}
//	else {
//		trap_FS_FOpenFile(va("%s.q3mmeCam", fname), &f, FS_WRITE);
//	}
//
//	if (!f) {
//		Com_Printf("^1couldn't create %s.q3mmeCam\n", CG_Argv(1));
//		return;
//	}
//
//	CG_Q3mmeCameraSave(f);
//	trap_FS_FCloseFile(f);
//}
//
//static void CG_LoadQ3mmeCamera_f(void)
//{
//	qboolean useDefaultFolder = qtrue;
//	const char* fname;
//	int i;
//	BG_XMLParse_t xmlParse;
//	char filename[MAX_OSPATH];
//	BG_XMLParseBlock_t loadBlock[] = {
//		{ "camera", CG_Q3mmeCameraParse, 0 },
//		{ 0, 0, 0 },
//	};
//
//	if (CG_Argc() < 2) {
//		Com_Printf("usage: loadq3mmecamera <camera name>\n");
//		return;
//	}
//
//	fname = CG_Argv(1);
//	for (i = 0; i < strlen(fname); i++) {
//		if (fname[i] == '/') {
//			useDefaultFolder = qfalse;
//			break;
//		}
//	}
//
//	if (useDefaultFolder) {
//		//trap_FS_FOpenFile(va("cameras/%s.q3mmeCam", CG_Argv(1)), &f, FS_READ);
//		//ret = BG_XMLOpen(&xmlParse, va("cameras/%s.q3mmeCam", CG_Argv(1)));
//		Com_sprintf(filename, sizeof(filename), "cameras/%s.q3mmeCam", CG_Argv(1));
//	}
//	else {
//		//trap_FS_FOpenFile(va("%s.q3mmeCam", fname), &f, FS_READ);
//		Com_sprintf(filename, sizeof(filename), "%s.q3mmeCam", CG_Argv(1));
//	}
//
//	if (!BG_XMLOpen(&xmlParse, filename)) {
//		Com_Printf("^1couldn't open %s\n", CG_Argv(1));
//		return;
//	}
//
//	if (!BG_XMLParse(&xmlParse, 0, loadBlock, 0)) {
//		Com_Printf("^1errors while loading q3mme camera\n");
//		return;
//	}
//}

typedef struct {
	char    *cmd;
	void ( *function )( void );
} consoleCommand_t;

static consoleCommand_t commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+inventory", CG_InventoryDown_f },
	{ "-inventory", CG_InventoryUp_f },
//	{ "+zoom", CG_ZoomDown_f },		// (SA) zoom moved to a wbutton so server can determine weapon firing based on zoom status
//	{ "-zoom", CG_ZoomUp_f },
	{ "zoomin", CG_ZoomIn_f },
	{ "zoomout", CG_ZoomOut_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weaplastused", CG_LastWeaponUsed_f },
	{ "weapnextinbank", CG_NextWeaponInBank_f },
	{ "weapprevinbank", CG_PrevWeaponInBank_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapalt", CG_AltWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponbank", CG_WeaponBank_f },
	{ "itemnext", CG_NextItem_f },
	{ "itemprev", CG_PrevItem_f },
	{ "item", CG_Item_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "loadhud", CG_LoadHud_f },
	{ "loaddeferred", CG_LoadDeferredPlayers },  // spelling fixed (SA)
//	{ "camera", CG_Camera_f },	// duffy
	{ "fade", CG_Fade_f },   // duffy
	{ "loadhud", CG_LoadHud_f },

	// NERVE - SMF
	{ "mp_QuickMessage", CG_QuickMessage_f },
	{ "OpenLimboMenu", CG_OpenLimbo_f },
	{ "CloseLimboMenu", CG_CloseLimbo_f },
	{ "LimboMessage", CG_LimboMessage_f },
	{ "VoiceChat", CG_VoiceChat_f },
	{ "VoiceTeamChat", CG_TeamVoiceChat_f },
	{ "SetWeaponCrosshair", CG_SetWeaponCrosshair_f },
	// -NERVE - SMF

	// Arnout
	{ "dumploc", CG_DumpLocation_f },

	//{ "recordpath", CG_RecordPath_f },
	//{ "playpath", CG_PlayPath_f },
	//{ "stopplaypath", CG_StopPlayPath_f },
	//{ "centerroll", CG_CenterRoll_f },
	//{ "drawrawpath", CG_DrawRawPath_f },
	//{ "chase", CG_Chase_f },
	//{ "addcamerapoint", CG_AddCameraPoint_f },
	//{ "clearcamerapoints", CG_ClearCameraPoints_f },
	//{ "playcamera", CG_PlayCamera_f },
	//{ "stopcamera", CG_StopCamera_f },
	//{ "savecamera", CG_SaveCamera_f },
	//{ "camtracesave", CG_CamtraceSave_f },
	//{ "loadcamera", CG_LoadCamera_f },
	//{ "selectcamerapoint", CG_SelectCameraPoint_f },
	//{ "deletecamerapoint", CG_DeleteCameraPoint_f },
	////{ "drawcamera", CG_DrawCamera_f },
	//{ "editcamerapoint", CG_EditCameraPoint_f },
	////{ "changecamerapoint", CG_ChangeThisCameraPoint_f },  //FIXME change name
	//{ "ecam", CG_ChangeSelectedCameraPoints_f },
	//{ "setviewpointmark", CG_SetViewPointMark_f },
	////{ "deleteviewpointmark", CG_DeleteViewPointMark_f },
	//{ "gotoviewpointmark", CG_GotoViewPointMark_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char  *cmd;
	int i;

	// Arnout - don't allow console commands until a snapshot is present
	if ( !cg.snap ) {
		return qfalse;
	}

	cmd = CG_Argv( 0 );

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int i;

	for ( i = 0 ; i < ARRAY_LEN( commands ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand( "kill" );
	trap_AddCommand( "say" );
	trap_AddCommand( "say_team" );
	trap_AddCommand( "say_limbo" );           // NERVE - SMF
	trap_AddCommand( "tell" );
//	trap_AddCommand( "vsay" );
//	trap_AddCommand( "vsay_team" );
//	trap_AddCommand( "vtell" );
//	trap_AddCommand( "vtaunt" );
//	trap_AddCommand( "vosay" );
//	trap_AddCommand( "vosay_team" );
//	trap_AddCommand( "votell" );
	trap_AddCommand( "give" );
	trap_AddCommand( "god" );
	trap_AddCommand( "notarget" );
	trap_AddCommand( "noclip" );
	trap_AddCommand( "where" );
	trap_AddCommand( "team" );
	trap_AddCommand( "follow" );
	trap_AddCommand( "follownext" );
	trap_AddCommand( "followprev" );
	trap_AddCommand( "levelshot" );
	trap_AddCommand( "addbot" );
	trap_AddCommand( "setviewpos" );
	trap_AddCommand( "callvote" );
	trap_AddCommand( "vote" );
//	trap_AddCommand( "callteamvote" );
//	trap_AddCommand( "teamvote" );
	trap_AddCommand( "stats" );
//	trap_AddCommand( "teamtask" );
	trap_AddCommand( "loaddeferred" );        // spelling fixed (SA)

//	trap_AddCommand( "startCamera" );
//	trap_AddCommand( "stopCamera" );
//	trap_AddCommand( "setCameraOrigin" );

	// Rafael
	trap_AddCommand( "nofatigue" );

	// NERVE - SMF
	trap_AddCommand( "setspawnpt" );
	trap_AddCommand( "follownext" );
	trap_AddCommand( "followprev" );

	trap_AddCommand( "start_match" );
	trap_AddCommand( "reset_match" );
	trap_AddCommand( "swap_teams" );
	// NERVE - SMF
}