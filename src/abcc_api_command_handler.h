/*******************************************************************************
** Copyright 2024-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Header file for ABCC command message handler.
********************************************************************************
*/

#ifndef ABCC_API_CMD_HANDLER_H_
#define ABCC_API_CMD_HANDLER_H_

#include "abcc_types.h"
#include "abp.h"
#include "abcc_api_config.h"

/*------------------------------------------------------------------------------
** Definitions and prototypes for attribute lookup table.
**------------------------------------------------------------------------------
*/
enum ABCC_ATTR_SERVICE_TAG
{
   SERVICE_BOOL8,
   SERVICE_UINT8,
   SERVICE_INT8,
   SERVICE_UINT16,
   SERVICE_INT16,
   SERVICE_UINT32,
   SERVICE_INT32,
   SERVICE_STR,
   SERVICE_BUFFER, // Array and struct
};

typedef BOOL8 (*ABCC_BOOL8_Get_AttrFuncType) (void);
typedef UINT8 (*ABCC_UINT8_Get_AttrFuncType) (void);
typedef INT8  (*ABCC_INT8_Get_AttrFuncType)  (void);
typedef UINT16(*ABCC_UINT16_Get_AttrFuncType)(void);
typedef INT16 (*ABCC_INT16_Get_AttrFuncType) (void);
typedef UINT32(*ABCC_UINT32_Get_AttrFuncType)(void);
typedef INT32 (*ABCC_INT32_Get_AttrFuncType) (void);
typedef UINT16(*ABCC_STR_Get_AttrFuncType)   (char* pPackedStrDest, UINT16 iBuffSizeBytes);
typedef UINT16(*ABCC_ARR_Get_AttrFuncType)   (void* pPackedArrDest, UINT16 iBuffSizeBytes);

typedef void (*ABCC_BOOL8_Set_AttrFuncType) (BOOL8);
typedef void (*ABCC_UINT8_Set_AttrFuncType) (UINT8);
typedef void (*ABCC_INT8_Set_AttrFuncType)  (INT8);
typedef void (*ABCC_UINT16_Set_AttrFuncType)(UINT16);
typedef void (*ABCC_INT16_Set_AttrFuncType) (INT16);
typedef void (*ABCC_UINT32_Set_AttrFuncType)(UINT32);
typedef void (*ABCC_INT32_Set_AttrFuncType) (INT32);
typedef void (*ABCC_STR_Set_AttrFuncType)   (char* pPackedStrSrc, UINT16 iBuffSizeBytes);
typedef void (*ABCC_ARR_Set_AttrFuncType)   (void* pPackedArrSrc, UINT16 iSizeBytes);

typedef void (*ABCC_Reset_ObjFuncType)(ABP_ResetType);

typedef BOOL8(*ABCC_Get_Enum_String_AttrFuncType)(UINT8 bEnumValue, char* pPackedStrDest, UINT16 iBuffSizeBytes);

#if APP_OBJ_ENABLE
typedef BOOL8(*ABCC_APP_ResetReq_ObjFuncType)(ABP_ResetType);
#endif

typedef struct
{
   UINT8 bObject;
   UINT8 bInstance;
   union
   {
      UINT8 bAttr;
   }uCmdExt;
   enum ABP_MsgCmdType bCommand;
   enum ABCC_ATTR_SERVICE_TAG eServiceTag;
   union
   {
      ABCC_BOOL8_Get_AttrFuncType       pnGetBool8Attr;
      ABCC_UINT8_Get_AttrFuncType       pnGetUint8Attr;
      ABCC_INT8_Get_AttrFuncType        pnGetInt8Attr;
      ABCC_UINT16_Get_AttrFuncType      pnGetUint16Attr;
      ABCC_INT16_Get_AttrFuncType       pnGetInt16Attr;
      ABCC_UINT32_Get_AttrFuncType      pnGetUint32Attr;
      ABCC_INT32_Get_AttrFuncType       pnGetInt32Attr;
      ABCC_STR_Get_AttrFuncType         pnGetStrAttr;
      ABCC_ARR_Get_AttrFuncType         pnGetArrAttr;
      ABCC_BOOL8_Set_AttrFuncType       pnSetBool8Attr;
      ABCC_UINT8_Set_AttrFuncType       pnSetUint8Attr;
      ABCC_INT8_Set_AttrFuncType        pnSetInt8Attr;
      ABCC_UINT16_Set_AttrFuncType      pnSetUint16Attr;
      ABCC_INT16_Set_AttrFuncType       pnSetInt16Attr;
      ABCC_UINT32_Set_AttrFuncType      pnSetUint32Attr;
      ABCC_INT32_Set_AttrFuncType       pnSetInt32Attr;
      ABCC_STR_Set_AttrFuncType         pnSetStrAttr;
      ABCC_ARR_Set_AttrFuncType         pnSetArrAttr;
      ABCC_Reset_ObjFuncType            pnResetObj;
      ABCC_Get_Enum_String_AttrFuncType pnGetEnumStrAttr;
#if APP_OBJ_ENABLE
      ABCC_APP_ResetReq_ObjFuncType pnResetReqObj;
#endif
      void* pxUnknownFuncPtr; // Do not use.
   }uCbx;
   union
   {
      BOOL8  fBool8;
      UINT8  bUnsigned8;
      INT8   bSigned8;
      UINT16 iUnsigned16;
      INT16  iSigned16;
      UINT32 lUnsigned32;
      INT32  lSigned32;
      char*  pacString;
      char*  pacStringBuffer; // Arrays and structs
   }uData;
   union
   {
      UINT16 iMaxDataSize; // Max size (array & string)
      UINT16 iDataSize;    // Fixed size (structs)
   }uAttrLength;
}Command_Handler_Lookup_Type;

#endif
