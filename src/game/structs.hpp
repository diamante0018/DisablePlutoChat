#pragma once

namespace game
{
	typedef float vec_t;
	typedef vec_t vec2_t[2];
	typedef vec_t vec3_t[3];
	typedef vec_t vec4_t[4];

	struct StringTableCell
	{
		const char* string;
		int hash;
	};

	struct StringTable
	{
		const char* name;
		int columnCount;
		int rowCount;
		StringTableCell* values;
	};

	struct cmd_function_t
	{
		cmd_function_t* next;
		const char* name;
		const char* autoCompleteDir;
		const char* autoCompleteExt;
		void(__cdecl* function)();
		int flags;
	};

	struct CmdArgs
	{
		int nesting;
		int localClientNum[8];
		int controllerIndex[8];
		int argc[8];
		const char** argv[8];
	};

	typedef enum
	{
		HITLOC_NONE,
		HITLOC_HELMET,
		HITLOC_HEAD,
		HITLOC_NECK,
		HITLOC_TORSO_UPR,
		HITLOC_TORSO_LWR,
		HITLOC_R_ARM_UPR,
		HITLOC_L_ARM_UPR,
		HITLOC_R_ARM_LWR,
		HITLOC_L_ARM_LWR,
		HITLOC_R_HAND,
		HITLOC_L_HAND,
		HITLOC_R_LEG_UPR,
		HITLOC_L_LEG_UPR,
		HITLOC_R_LEG_LWR,
		HITLOC_L_LEG_LWR,
		HITLOC_R_FOOT,
		HITLOC_L_FOOT,
		HITLOC_GUN,
		HITLOC_SHIELD,
		HITLOC_NUM
	} hitLocation_t;

	typedef enum
	{
		NS_CLIENT1 = 0,
		NS_CLIENT2 = 1,
		NS_CLIENT3 = 2,
		NS_CLIENT4 = 3,
		NS_MAXCLIENTS = 4,
		NS_SERVER = 4,
		NS_PACKET = 5,
		NS_INVALID_NETSRC = 6
	} netsrc_t;

	enum netadrtype_t
	{
		NA_BOT = 0x0,
		NA_BAD = 0x1,
		NA_LOOPBACK = 0x2,
		NA_BROADCAST = 0x3,
		NA_IP = 0x4
	};

	struct netadr_s
	{
		netadrtype_t type;
		unsigned char ip[4];
		unsigned __int16 port;
		netsrc_t localNetID;
		char __pad0[4];
		unsigned int index;
	};

	static_assert(sizeof(netadr_s) == 24);

	struct msg_t
	{
		int overflowed;
		int readOnly;
		char* data;
		char* splitData;
		int maxsize;
		int cursize;
		int splitSize;
		int readcount;
		int bit;
		int lastEntityRef;
	};

	struct XZoneInfo
	{
		const char* name;
		int allocFlags;
		int freeFlags;
	};

	struct scr_entref_t
	{
		unsigned __int16 entnum;
		unsigned __int16 classnum;
	};

	typedef void(__cdecl* scr_call_t)(int entref);

	typedef enum
	{
		ANIM_ET_PAIN,
		ANIM_ET_DEATH,
		ANIM_ET_FIREWEAPON,
		ANIM_ET_JUMP,
		ANIM_ET_JUMPBK,
		ANIM_ET_LAND,
		ANIM_ET_DROPWEAPON,
		ANIM_ET_RAISEWEAPON,
		ANIM_ET_CLIMB_MOUNT,
		ANIM_ET_CLIMB_DISMOUNT,
		ANIM_ET_RELOAD,
		ANIM_ET_CROUCH_TO_PRONE,
		ANIM_ET_PRONE_TO_CROUCH,
		ANIM_ET_STAND_TO_CROUCH,
		ANIM_ET_CROUCH_TO_STAND,
		ANIM_ET_STAND_TO_PRONE,
		ANIM_ET_PRONE_TO_STAND,
		ANIM_ET_MELEEATTACK,
		ANIM_ET_KNIFE_MELEE,
		ANIM_ET_KNIFE_MELEE_CHARGE,
		ANIM_ET_SHELLSHOCK,
		ANIM_ET_STUNNED,
		NUM_ANIM_EVENTTYPES
	} scriptAnimEventTypes_t;

	typedef enum
	{
		MOD_UNKNOWN = 0,
		MOD_PISTOL_BULLET = 1,
		MOD_RIFLE_BULLET = 2,
		MOD_EXPLOSIVE_BULLET = 3,
		MOD_GRENADE = 4,
		MOD_GRENADE_SPLASH = 5,
		MOD_PROJECTILE = 6,
		MOD_PROJECTILE_SPLASH = 7,
		MOD_MELEE = 8,
		MOD_HEAD_SHOT = 9,
		MOD_CRUSH = 10,
		MOD_FALLING = 11,
		MOD_SUICIDE = 12,
		MOD_TRIGGER_HURT = 13,
		MOD_EXPLOSIVE = 14,
		MOD_IMPACT = 15,
		MOD_NUM = 16
	} meansOfDeath_t;

	enum scriptType_e
	{
		SCRIPT_NONE = 0,
		SCRIPT_OBJECT = 1,
		SCRIPT_STRING = 2,
		SCRIPT_ISTRING = 3,
		SCRIPT_VECTOR = 4,
		SCRIPT_FLOAT = 5,
		SCRIPT_INTEGER = 6,
		SCRIPT_END = 8,
		SCRIPT_FUNCTION = 9,
		SCRIPT_STRUCT = 19,
		SCRIPT_ARRAY = 22,
	};

	struct VariableStackBuffer
	{
		const char* pos;
		unsigned __int16 size;
		unsigned __int16 bufLen;
		unsigned __int16 localId;
		char time;
		char buf[1];
	};

	union VariableUnion
	{
		int intValue;
		float floatValue;
		unsigned int stringValue;
		const float* vectorValue;
		const char* codePosValue;
		unsigned int pointerValue;
		VariableStackBuffer* stackValue;
		unsigned int entityId;
		unsigned int uintValue;
	};

	struct VariableValue
	{
		VariableUnion u;
		scriptType_e type;
	};

	struct function_stack_t
	{
		const char* pos;
		unsigned int localId;
		unsigned int localVarCount;
		VariableValue* top;
		VariableValue* startTop;
	};

	struct function_frame_t
	{
		function_stack_t fs;
		int topType;
	};

	struct scrVmPub_t
	{
		unsigned int* localVars;
		VariableValue* maxstack;
		int function_count;
		function_frame_t* function_frame;
		VariableValue* top;
		/*bool debugCode;
		bool abort_on_error;
		bool terminal_error;
		bool block_execution;*/
		unsigned int inparamcount;
		unsigned int outparamcount;
		unsigned int breakpointOutparamcount;
		bool showError;
		function_frame_t function_frame_start[32];
		VariableValue stack[2048];
	};

	struct scr_classStruct_t
	{
		unsigned __int16 id;
		unsigned __int16 entArrayId;
		char charId;
		const char* name;
	};

	struct ObjectVariableChildren
	{
		unsigned __int16 firstChild;
		unsigned __int16 lastChild;
	};

	struct ObjectVariableValue_u_f
	{
		unsigned __int16 prev;
		unsigned __int16 next;
	};

	union ObjectVariableValue_u_o_u
	{
		unsigned __int16 size;
		unsigned __int16 entnum;
		unsigned __int16 nextEntId;
		unsigned __int16 self;
	};

	struct	ObjectVariableValue_u_o
	{
		unsigned __int16 refCount;
		ObjectVariableValue_u_o_u u;
	};

	union ObjectVariableValue_w
	{
		unsigned int type;
		unsigned int classnum;
		unsigned int notifyName;
		unsigned int waitTime;
		unsigned int parentLocalId;
	};

	struct ChildVariableValue_u_f
	{
		unsigned __int16 prev;
		unsigned __int16 next;
	};

	union ChildVariableValue_u
	{
		ChildVariableValue_u_f f;
		VariableUnion u;
	};

	struct ChildBucketMatchKeys_keys
	{
		unsigned __int16 name_hi;
		unsigned __int16 parentId;
	};

	union ChildBucketMatchKeys
	{
		ChildBucketMatchKeys_keys keys;
		unsigned int match;
	};

	struct	ChildVariableValue
	{
		ChildVariableValue_u u;
		unsigned __int16 next;
		char type;
		char name_lo;
		ChildBucketMatchKeys k;
		unsigned __int16 nextSibling;
		unsigned __int16 prevSibling;
	};

	union ObjectVariableValue_u
	{
		ObjectVariableValue_u_f f;
		ObjectVariableValue_u_o o;
	};

	struct ObjectVariableValue
	{
		ObjectVariableValue_u u;
		ObjectVariableValue_w w;
	};

	struct scrVarGlob_t
	{
		ObjectVariableValue objectVariableValue[36864];
		ObjectVariableChildren objectVariableChildren[36864];
		unsigned __int16 childVariableBucket[65536];
		ChildVariableValue childVariableValue[102400];
	};

	enum dvar_flags : std::uint16_t
	{
		DVAR_FLAG_NONE = 0,
		DVAR_FLAG_SAVED = 1,
		DVAR_FLAG_LATCHED = 2,
		DVAR_FLAG_CHEAT = 4,
		DVAR_FLAG_REPLICATED = 0x8,
		DVAR_FLAG_UNK1 = 0x40,
		DVAR_FLAG_UNK2 = 0x200,
		DVAR_FLAG_SYSTEM = 0x400,
		DVAR_FLAG_WRITE = 0x800,
		DVAR_FLAG_READONLY = 0x2000,
	};

	union DvarValue
	{
		bool enabled;
		int integer;
		unsigned int unsignedInt;
		float value;
		float vector[4];
		const char* string;
		char color[4];
	};

	struct enum_limit
	{
		int stringCount;
		const char** strings;
	};

	struct int_limit
	{
		int min;
		int max;
	};

	struct float_limit
	{
		float min;
		float max;
	};

	union DvarLimits
	{
		enum_limit enumeration;
		int_limit integer;
		float_limit value;
		float_limit vector;
	};

	struct dvar_t
	{
		const char* name;
		unsigned int flags;
		char type;
		bool modified;
		DvarValue current;
		DvarValue latched;
		DvarValue reset;
		DvarLimits domain;
		bool(__cdecl* domainFunc)(dvar_t*, DvarValue);
		dvar_t* hashNext;
	};

	struct usercmd_s
	{
		int serverTime;
		int buttons;
		int angles[3];
		unsigned int weapon;
		unsigned int offHand;
		char forwardmove;
		char rightmove;
		unsigned __int16 airburstMarkDistance;
		unsigned __int16 meleeChargeEnt;
		unsigned char meleeChargeDist;
		char selectedLoc[2];
		char selectedLocAngle;
		char remoteControlAngles[2];
		int remoteControlMove;
	};

	typedef enum
	{
		WEAPTYPE_NONE = 0x0,
		WEAPTYPE_BULLET = 0x1,
		WEAPTYPE_GRENADE = 0x2,
		WEAPTYPE_PROJECTILE = 0x3,
		WEAPTYPE_RIOTSHIELD = 0x4,
		WEAPTYPE_NUM = 0x5
	} weapType_t;

	enum PlayerHandIndex
	{
		WEAPON_HAND_RIGHT = 0x0,
		WEAPON_HAND_LEFT = 0x1,
		NUM_WEAPON_HANDS = 0x2,
		WEAPON_HAND_DEFAULT = 0x0
	};

	struct Weapon_s
	{
		unsigned int padding : 8;
		unsigned int scopeVariation : 3;
		unsigned int weaponOthers : 4;
		unsigned int weaponUnderBarrels : 2;
		unsigned int weaponScopes : 3;
		unsigned int weaponIdx : 8;
		unsigned int weaponVariation : 4;
	};

	union Weapon
	{
		Weapon_s _s_0;
		unsigned int data;
	};

	struct weaponParms
	{
		float forward[3];
		float right[3];
		float up[3];
		float muzzleTrace[3];
		float gunForward[3];
		Weapon weapon;
		bool isAlternate;
		void* weapDef;
		void* weapCompleteDef;
	};

	struct BulletFireParams
	{
		int weaponEntIndex;
		int ignoreEntIndex;
		float damageMultiplier;
		meansOfDeath_t methodOfDeath;
		bool noRicochet;
		float origStart[3];
		float start[3];
		float end[3];
		float dir[3];
	};

	static_assert(sizeof(BulletFireParams) == 0x44);

	enum EffectiveStance
	{
		PM_EFF_STANCE_DEFAULT = 0,
		PM_EFF_STANCE_PRONE = 1,
		PM_EFF_STANCE_DUCKED = 2,
		PM_EFF_STANCE_LASTSTANDCRAWL = 3,
		PM_EFF_STANCE_COUNT = 4
	};

	enum clientState_t : std::int32_t
	{
		CS_FREE = 0,
		CS_ZOMBIE = 1,
		CS_UNKNOWN = 2,
		CS_CONNECTED = 3,
		CS_PRIMED = 4,
		CS_ACTIVE = 5
	};

	struct MantleState
	{
		float yaw;
		int timer;
		int transIndex;
		int flags;
	};

	struct SprintState_s
	{
		int sprintButtonUpRequired; // 0x20C
		int sprintDelay; // 0x210
		int lastSprintStart;
		int lastSprintEnd; // 0x218
		int sprintStartMaxLength;
	};

	struct PlayerVehicleState
	{
		int entity;
		int flags;
		float origin[3];
		float angles[3];
		float velocity[3];
		float angVelocity[3];
		float tilt[2];
		float tiltVelocity[2];
	};

	struct PlayerActiveWeaponState
	{
		int weapAnim;
		int weaponTime;
		int weaponDelay;
		int weaponRestrictKickTime;
		int weaponState;
		int weaponHandFlags;
		int weaponShotCount;
	};

	struct PlayerEquippedWeaponState
	{
		bool usedBefore;
		bool dualWielding;
		bool inAltMode;
		bool needsRechamber[2];
		int zoomLevelIndex;
	};

	static_assert(sizeof(PlayerEquippedWeaponState) == 12);

	struct EntityEvent
	{
		int eventType;
		int eventParm;
	};

	struct playerEvents_t
	{
		int eventSequence;
		EntityEvent events[4];
		int oldEventSequence;
		int timeADSCameUp;
	};

	static_assert(sizeof(playerEvents_t) == 44);

	enum ViewLockTypes
	{
		PLAYERVIEWLOCK_NONE = 0,
		PLAYERVIEWLOCK_FULL = 1,
		PLAYERVIEWLOCK_WEAPONJITTER = 2,
		PLAYERVIEWLOCKCOUNT = 3
	};

	enum TraceHitType
	{
		TRACE_HITTYPE_NONE = 0,
		TRACE_HITTYPE_ENTITY = 1,
		TRACE_HITTYPE_DYNENT_MODEL = 2,
		TRACE_HITTYPE_DYNENT_BRUSH = 3,
		TRACE_HITTYPE_GLASS = 4
	};

#pragma pack(push, 1)
	struct playerState_s
	{
		int commandTime;
		int pm_type;
		int pm_time;
		int pm_flags;
		int otherFlags;
		int linkFlags;
		int bobCycle;
		float origin[3];
		float velocity[3];
		int grenadeTimeLeft;
		int throwbackGrenadeOwner;
		int throwbackGrenadeTimeLeft;
		unsigned int throwbackWeapon;
		int remoteEyesEnt;
		int remoteEyesTagname;
		int remoteControlEnt;
		int remoteTurretEnt;
		int foliageSoundTime;
		int gravity;
		int speed;
		float delta_angles[3];
		int groundEntityNum;
		float vLadderVec[3];
		int jumpTime;
		float jumpOriginZ;
		int legsTimer;
		int legsAnim;
		int torsoTimer;
		int torsoAnim;
		int legsAnimDuration;
		int torsoAnimDuration;
		int damageTimer;
		int damageDuration;
		int flinchYawAnim;
		int corpseIndex;
		PlayerVehicleState vehicleState;
		int movementDir;
		int eFlags;
		playerEvents_t pe;
		int unpredictableEventSequence;
		int unpredictableEventSequenceOld;
		int unpredictableEvents[4];
		char unpredictableEventParms[16];
		int clientNum;
		int viewmodelIndex;
		float viewangles[3];
		int viewHeightTarget;
		float viewHeightCurrent;
		int viewHeightLerpTime;
		int viewHeightLerpTarget;
		int viewHeightLerpDown;
		char viewAngleClampBase[8];
		char viewAngleClampRange[8];
		int damageEvent;
		int damageYaw;
		int damagePitch;
		int damageCount;
		int damageFlags;
		int stats[4];
		float proneDirection;
		float proneDirectionPitch;
		float proneTorsoPitch;
		ViewLockTypes viewlocked;
		int viewlocked_entNum;
		float linkAngles[3];
		float linkWeaponAngles[3];
		int linkWeaponEnt;
		int loopSound;
		int cursorHint;
		int cursorHintString;
		int cursorHintEntIndex;
		int cursorHintDualWield;
		int iCompassPlayerInfo;
		int radarEnabled;
		int radarBlocked;
		int radarMode;
		int radarStrength;
		int radarShowEnemyDirection;
		int locationSelectionInfo;
		SprintState_s sprintState;
		float holdBreathScale;
		int holdBreathTimer;
		float moveSpeedScaleMultiplier;
		MantleState mantleState;
		PlayerActiveWeaponState weapState[2];
		unsigned int weaponsEquipped[15];
		PlayerEquippedWeaponState weapEquippedData[15];
		char weapCommon[376];
		int meleeChargeDist;
		int meleeChargeTime;
		int meleeChargeEnt;
		int airburstMarkDistance;
		unsigned int perks[2];
		unsigned int perkSlots[9];
		char __pad0[11752];
	};

	struct Bounds
	{
		float midPoint[3];
		float halfSize[3];
	};

	static_assert(sizeof(Bounds) == 24);

	struct pmove_t
	{
		playerState_s* ps;
		usercmd_s cmd;
		usercmd_s oldcmd;
		int tracemask;
		int numtouch;
		int touchents[32];
		Bounds bounds;
		float speed;
		int proneChange;
		float maxSprintTimeMultiplier;
		bool mantleStarted;
		float mantleEndPos[3];
		int mantleDuration;
		int viewChangeTime;
		float viewChange;
		float fTorsoPitch;
		float fWaistPitch;
		int remoteTurretFireTime;
		int lastUpdateCMDServerTime;
		unsigned char handler;
	};
#pragma pack(pop)

	static_assert(sizeof(playerState_s) == 13056);

	enum entityFlag
	{
		FL_GODMODE = 0x1,
		FL_DEMI_GODMODE = 0x2,
		FL_NOTARGET = 0x4,
		FL_NO_KNOCKBACK = 0x8,
		FL_NO_RADIUS_DAMAGE = 0x10,
		FL_SUPPORTS_LINKTO = 0x1000,
		FL_NO_AUTO_ANIM_UPDATE = 0x2000,
		FL_GRENADE_TOUCH_DAMAGE = 0x4000,
		FL_STABLE_MISSILES = 0x20000,
		FL_REPEAT_ANIM_UPDATE = 0x40000,
		FL_VEHICLE_TARGET = 0x80000,
		FL_GROUND_ENT = 0x100000,
		FL_CURSOR_HINT = 0x200000,
		FL_MISSILE_ATTRACTOR = 0x800000,
		FL_WEAPON_BEING_GRABBED = 0x1000000,
		FL_DELETE = 0x2000000,
		FL_BOUNCE = 0x4000000,
		FL_MOVER_SLIDE = 0x8000000
	};

	typedef enum
	{
		SESS_STATE_PLAYING = 0x0,
		SESS_STATE_DEAD = 0x1,
		SESS_STATE_SPECTATOR = 0x2,
		SESS_STATE_INTERMISSION = 0x3
	} sessionState_t;

	typedef enum
	{
		CON_DISCONNECTED = 0x0,
		CON_CONNECTING = 0x1,
		CON_CONNECTED = 0x2
	} clientConnected_t;

	struct gclient_s
	{
		playerState_s ps;
		char __pad0[12];
		sessionState_t sessionState; // 13068
		char __pad1[40];
		clientConnected_t connected; // 13112
		char __pad2[656]; // 13056
		int flags; // 13772
		int spectatorClient;
		int lastCmdTime;
//		int mpviewer; // Debug ?
		int buttons;
		int oldButtons;
		int latched_buttons;
		int buttonsSinceLastFrame;
		float oldOrigin[3];
		float fGunPitch;
		float fGunYaw;
		int damage_blood;
		int damage_stun;
		float damage_from[3];
		int damage_fromWorld;
		int accurateCount;
		int accuracy_shots;
		int accuracy_hits;
		int inactivityTime;
		int inactivityWarning;
		int lastVoiceTime;
		int switchTeamTime;
		float currentAimSpreadScale;
		float prevLinkedInvQuat[4];
		bool prevLinkAnglesSet;
		bool link_rotationMovesEyePos;
		bool link_doCollision;
		bool link_useTagAnglesForViewAngles;
		bool link_useBaseAnglesForViewClamp;
		float linkAnglesFrac;
		char link_viewClamp[64];
		char persistantPowerup[4];
		int portalID;
		int dropWeaponTime;
		int sniperRifleFiredTime;
		float sniperRifleMuzzleYaw;
		int PCSpecialPickedUpCount;
		int useHoldEntity;
		int useHoldTime;
		int useButtonDone;
		int iLastCompassPlayerInfoEnt;
		int compassPingTime;
		int damageTime;
		float v_dmg_roll;
		float v_dmg_pitch;
		float baseAngles[3];
		float baseOrigin[3];
		float swayViewAngles[3];
		float swayOffset[3];
		float swayAngles[3];
		float recoilAngles[3];
		float recoilSpeed[3];
		float fLastIdleFactor;
		int lastServerTime;
		int lastWeapon;
		bool lastWeaponAltStatus;
		bool previouslyFiring;
		bool previouslyFiringLeftHand;
		bool previouslyUsingNightVision;
		bool previouslySprinting;
		int visionDuration[6];
		char visionName[384];
		int lastStand;
		int lastStandTime;
		int hudElemLastAssignedSoundID;
		float lockedTargetOffset[3];
		unsigned __int16 attachShieldTagName;
		int hintForcedType;
		int hintForcedString;
	};

#pragma pack(push, 1)
	struct client_s
	{
		clientState_t state;
		char __pad0[0x24];
		netadr_s remote;
		char __pad1[0x78658];
	};

	static_assert(sizeof(client_s) == 0x78698);

	struct EntHandle
	{
		unsigned __int16 number;
		unsigned __int16 infoIndex;
	};

	struct gentity_s
	{
		int entnum;
		char __pad0[340];
		gclient_s* client; // 344
		char __pad1[40];
		int flags;
		int eventTime;
		int clipmask;
		int processedFrame;
		EntHandle parent;
		int nextthink;
		int health;
		char __pad2[212];
	};

#pragma pack(pop)

	static_assert(sizeof(gentity_s) == 628);

	struct TraceExtents
	{
		float midPoint[3];
		float midPointPad;
		float halfDelta[3];
		float halfDeltaPad;
		float halfDeltaAbs[3];
		float halfDeltaAbsPad;
		float invDeltaAbs[3];
		float invDeltaAbsPad;
		float start[3];
		float startPad;
		float end[3];
		float endPad;
	};

	static_assert(sizeof(TraceExtents) == 96);

	struct trace_t
	{
		float fraction;
		float normal[3];
		int surfaceFlags;
		int contents;
		const char* material;
		TraceHitType hitType;
		unsigned __int16 hitId;
		unsigned __int16 modelIndex;
		unsigned __int16 partName;
		unsigned __int16 partGroup;
		bool allsolid;
		bool startsolid;
		bool walkable;
	};

	static_assert(sizeof(trace_t) == 0x2C);

	struct BulletTraceResults
	{
		trace_t trace;
		gentity_s* hitEnt;
		float hitPos[3];
		bool ignoreHitEnt;
		int depthSurfaceType;
		int hitClientNum;
	};

	struct pml_t
	{
		float forward[3];
		float right[3];
		float up[3];
		float frametime;
		int msec;
		int walking;
		int groundPlane;
		int almostGroundPlane;
		trace_t groundTrace;
		float impactSpeed;
		float previous_origin[3];
		float previous_velocity[3];
		int holdrand;
	};

	struct bgs_t
	{
	};
}
