#pragma once

#define WEAK __declspec(selectany)

namespace game
{
  // Functions
  WEAK symbol<void(int type, VariableUnion u)> AddRefToValue{0x5656E0};
  WEAK symbol<void(unsigned int id)> AddRefToObject{0x5655F0};
  WEAK symbol<unsigned int(unsigned int id)> AllocThread{0x565580};
  WEAK symbol<void(int type, VariableUnion u)> RemoveRefToValue{0x565730};
  WEAK symbol<void(unsigned int id)> RemoveRefToObject{0x5681E0};
  WEAK symbol<int(void*)> BG_GetMaxSprintTime{0x41BA70};
  WEAK symbol<void(gentity_s* ent, float* position, float* orientation)>
      TeleportPlayer{0x50D840};

  WEAK symbol<void(pmove_t*, trace_t*, const float*, const float*,
                   const Bounds*, int, int)>
      PM_playerTrace{0x421F00};
  WEAK symbol<void(const pmove_t*, trace_t*, const float*, const float*,
                   const Bounds*, int, int)>
      PM_trace{0x41CEB0};
  WEAK symbol<EffectiveStance(const playerState_s*)> PM_GetEffectiveStance{
      0x41D1E0};
  WEAK symbol<void(playerState_s*)> Jump_ClearState{0x4160F0};
  WEAK symbol<void(playerState_s*)> PM_ExitAimDownSight{0x428050};
  WEAK symbol<void(unsigned int weapon, bool isAlternate, char* output,
                   unsigned int maxStringLen)>
      BG_GetWeaponNameComplete{0x42F760};
  WEAK symbol<weapType_t(Weapon, bool)> BG_GetWeaponType{0x436E30};
  WEAK symbol<void(unsigned int*)> BG_srand{0x4237A0};
  WEAK symbol<bool(const unsigned int* const, Weapon, bool)>
      BG_WeaponBulletFire_ShouldPenetrate{0x431250};
  WEAK symbol<bool(const unsigned int* const, Weapon, bool)>
      BG_WeaponBulletFire_ShouldSpread{0x438F00};
  WEAK symbol<void(gentity_s*, const gentity_s*, gentity_s*, int, int, Weapon,
                   bool, const float*, hitLocation_t, int)>
      player_die{0x503460};

  WEAK symbol<void(errorParm_t, const char* fmt, ...)> Com_Error{0x555450};
  WEAK symbol<const char*(int index)> ConcatArgs{0x502150};
  WEAK symbol<void(int localClientNum, const char* text)> Cbuf_AddText{
      0x545680};
  WEAK symbol<void(int localClientNum, const char* text)> Cbuf_InsertText{
      0x545770};
  WEAK symbol<void(int localClientNum, int a2, const char* message, int a4,
                   int a5, int a6)>
      Client_Print{0x487B00};
  WEAK symbol<void(const char* cmdName, void(*function),
                   cmd_function_t* allocedCmd)>
      Cmd_AddCommandInternal{0x545DF0};
  WEAK symbol<void(const char* cmdName, void(*function),
                   cmd_function_t* allocedCmd)>
      Cmd_AddServerCommandInternal{0x545800};
  WEAK symbol<void()> Cbuf_AddServerText_f{0x71D130};
  WEAK symbol<void(const char* cmdName)> Cmd_RemoveCommand{0x545E20};
  WEAK symbol<const char*(int index)> Cmd_Argv{0x467600};
  WEAK symbol<void()> SV_DisconnectAllClients{0x05749E0};
  WEAK symbol<void(client_s* drop, const char* reason, int notifyOthers)>
      SV_DropClientInternal{0x570980};
  WEAK symbol<void(client_s* drop, const char* reason)> SV_DelayDropClient{
      0x570BB0};

  WEAK symbol<dvar_t*(const char*)> Dvar_FindVar{0x5BDCC0};
  WEAK symbol<dvar_t*(const char* dvarName, int value, int min, int max,
                      unsigned __int16 flags, const char* desc)>
      Dvar_RegisterInt{0x5BEA40};
  WEAK symbol<dvar_t*(const char* dvarName, bool value, unsigned __int16 flags,
                      const char* description)>
      Dvar_RegisterBool{0x5BE9F0};
  WEAK symbol<dvar_t*(const char* dvarName, const char* value,
                      unsigned __int16 flags, const char* description)>
      Dvar_RegisterString{0x5BEC90};
  WEAK symbol<dvar_t*(const char* dvarName, float value, float min, float max,
                      unsigned __int16 flags, const char* description)>
      Dvar_RegisterFloat{0x5BEA80};
  WEAK symbol<void(const char* name, const char* value)> Dvar_SetStringByName{
      0x5BF660};

  WEAK symbol<const char*(const char* s, const char* key)> Info_ValueForKey{
      0x5C2DB0};
  WEAK symbol<void(char* s, const char* key)> Info_RemoveKey{0x5C2F50};
  WEAK symbol<void(char* s, const char* key)> Info_RemoveKey_Big{0x5C3040};
  WEAK symbol<void(char* s, const char* key, const char* value)>
      Info_SetValueForKey{0x5C3170};

  WEAK symbol<void(const char*)> I_CleanStr{0x5C2C60};

  WEAK symbol<VariableValue(unsigned int classnum, int entnum, int offset)>
      GetEntityFieldValue{0x56AF20};
  WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)>
      FindVariable{0x5651F0};
  WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)>
      FindObject{0x565BD0};
  WEAK symbol<unsigned int(unsigned int parentId, unsigned int name)>
      GetVariable{0x5663E0};

  WEAK symbol<int(const char* cmd)> Key_GetBindingForCmd{0x48C1C0};

  WEAK symbol<const float*(const float* v)> Scr_AllocVector{0x565680};
  WEAK symbol<void()> Scr_ClearOutParams{0x569010};
  WEAK symbol<scr_entref_t(unsigned int entId)> Scr_GetEntityIdRef{0x565F60};
  WEAK symbol<void(unsigned int classnum, int entnum, int offset)>
      Scr_SetObjectField{0x52BCC0};
  WEAK symbol<void(int id, unsigned int stringValue, unsigned int paramcount)>
      Scr_NotifyId{0x56B5E0};
  WEAK symbol<int(const char* filename, unsigned int str)>
      Scr_GetFunctionHandle{0x5618A0};
  WEAK symbol<unsigned int(int handle, unsigned int objId,
                           unsigned int paramcount)>
      Scr_ExecThreadInternal{0x56E1C0};

  WEAK symbol<unsigned int(const char* str, unsigned int user)> SL_GetString{
      0x5649E0};
  WEAK symbol<const char*(unsigned int stringValue)> SL_ConvertToString{
      0x564270};

  WEAK symbol<void(int clientNum, int type, const char* command)>
      SV_GameSendServerCommand{0x573220};
  WEAK symbol<void(int arg, char* buffer, int bufferLength)> SV_Cmd_ArgvBuffer{
      0x5459F0};

  WEAK symbol<void(unsigned int notifyListOwnerId, unsigned int stringValue,
                   VariableValue* top)>
      VM_Notify{0x569720};
  WEAK symbol<unsigned int(unsigned int localId, unsigned int pos,
                           unsigned int paramcount)>
      VM_Execute{0x56DFE0};

  WEAK symbol<bool(netsrc_t, netadr_s dest, const char* message)>
      NET_OutOfBandPrint{0x55C830};
  WEAK symbol<void(gentity_s* ent, int a2, int which_chat, const char* message)>
      Cmd_Say_f{0x502B60};
  WEAK symbol<void()> Com_Quit_f{0x5556B0};
  WEAK symbol<void(int index, char* buffer, int bufferSize)> SV_GetUserinfo{
      0x573E00};
  WEAK symbol<bool(const char* s)> Info_Validate{0x5C3140};
  WEAK symbol<void(const char*, const StringTable**)> StringTable_GetAsset{
      0x5B87D0};
  WEAK symbol<int(const StringTable*, int, const char*)>
      StringTable_LookupRowNumForValue{0x5B8610};
  WEAK symbol<const char*(const StringTable*, int, int)>
      StringTable_GetColumnValueForRow{0x5B86A0};

  WEAK symbol<void*(int, const char*, int)> DB_FindXAssetHeader{0x4CA620};

  WEAK symbol<FILE*()> __iob_func{0x739462};
  WEAK symbol<int(FILE* stream, const char* format, ...)> _fprintf{0x738E95};

  // Variables
  WEAK symbol<CmdArgs> cmd_args{0x1C978D0};
  WEAK symbol<CmdArgs> sv_cmd_args{0x1CAA998};

  WEAK symbol<int> g_script_error_level{0x20B21FC};

  WEAK symbol<scrVmPub_t> scr_VmPub{0x20B4A80};
  WEAK symbol<scrVarGlob_t> scr_VarGlob{0x1E72180};
  WEAK symbol<function_stack_t> scr_function_stack{0x20B4A5C};

  WEAK symbol<scr_classStruct_t*> g_classMap{0x8B4300};

  WEAK symbol<gentity_s> g_entities{0x1A66E28};
  WEAK symbol<unsigned int> levelEntityId{0x208E1A4};
  WEAK symbol<unsigned int> gameEntityId{0x208E1A8};

  WEAK symbol<int> svs_clientCount{0x4B5CF8C};
  WEAK symbol<client_s> svs_clients{0x4B5CF90};

  namespace plutonium
  {
    WEAK symbol<int(const char* format, ...)> printf{0x0};
  }
} // namespace game
