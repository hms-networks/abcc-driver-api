/*******************************************************************************
** Copyright 2019-present HMS Industrial Networks AB.
** Licensed under the MIT License.
********************************************************************************
** File Description:
** Public header file for the Anybus File System Interface object.
**
** Before any filesystem operations can be performed a Filesystem Interface
** (FSI) instance has to be created. The created instance is provided in
** subsequent calls to the available functions. Several instances are supported
** in parallel by the CompactCom (typically 4 but may differ).
**
** This implementation supports up to ANB_FSI_MAX_CONCURRENT_OPERATIONS
** concurrent operations (outstanding commands waiting for a response). The
** internal resources tied to an operation can be re-used once the completion
** callback is invoked.
********************************************************************************
*/

#ifndef ANB_FSI_OBJ_H
#define ANB_FSI_OBJ_H

#include "abcc_object_config.h"

#if ANB_FSI_OBJ_ENABLE

#include "abcc.h"
#include "abp_fsi.h"

/*******************************************************************************
** Public constants
********************************************************************************
*/

/*******************************************************************************
** Public typedefs
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Directory entry information. Used by ANB_FSI_DirectoryRead().
**------------------------------------------------------------------------------
*/
typedef struct
{
   UINT32   lSize; /* Size of entry in octets. */
   UINT8    bFlags; /* See ABP_FSI_DIRECTORY_READ_* in abp_fsi.h for details. */
   char     acName[ ABP_FSI_MAX_PATH_LENGTH + 1 ]; /* '+1' for terminating NUL. */
}
ANB_FSI_DirEntryType;

/*------------------------------------------------------------------------------
** Completion callback for the FSI operations.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance that the FSI command targeted, if applicable.
**    eMsgResult - Result of the FSI command.
**    bFsiError  - Object-specific error code (ABP_FSI_ERR_...) if 'eMsgResult'
**                 was ABP_ERR_OBJ_SPECIFIC.
** Returns:
**    -
**------------------------------------------------------------------------------
*/
typedef void (*ANB_FSI_CompletionCbfType)( UINT16 iInstance, ABP_MsgErrorCodeType eMsgResult, UINT8 bFsiError );

/*******************************************************************************
** Public globals
********************************************************************************
*/

/*******************************************************************************
** Public services
********************************************************************************
*/

/*------------------------------------------------------------------------------
** Clear FSI object bookkeeping data at startup.
**------------------------------------------------------------------------------
** Arguments:
**    -
** Returns:
**    -
**------------------------------------------------------------------------------
*/
EXTFUNC void ANB_FSI_Init( void );

/*------------------------------------------------------------------------------
** Create an FSI instance. Must be performed before any other file/directory
** operations are possible. The instance returned in the callback is used in
** subsequent FSI calls.
**
** The instance has to be deleted to free up resources within the CompactCom.
**------------------------------------------------------------------------------
** Arguments:
**    pnCallback - Completion callback. If the command was successfull the
**                 instance number of the new instance is reported via the
**                 callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_Create( ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Deletes an FSI instance.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to delete.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_Delete( UINT16 iInstance, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Opens a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the file to open, relative to the current path.
**    bMode      - Mode, see "ABP_FSI_FILE_OPEN_..." in "abp_fsi.h".
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileOpen( UINT16 iInstance, char* pacName, UINT8 bMode, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Closes a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    plFileSize - Pointer to UINT32, the size of the file will be stored here
**                 on completion.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileClose( UINT16 iInstance, UINT32* plFileSize, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Delete a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the file to delete, relative to the current
**                 path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileDelete( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Copy a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacSrc     - Path + name of the source file, relative to the current path.
**    pacDest    - Path + name of the destination file, relative to the current
**                 path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileCopy( UINT16 iInstance, char* pacSrc, char* pacDest, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Rename a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacOld     - Old path + name, relative to the current path.
**    pacNew     - New path + name, relative to the current path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileRename( UINT16 iInstance, char* pacOld, char* pacNew, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Read data from a file.
**
** Note that multiple reads may be required to read the complete file. When
** the actual read size equals 0 EOF (end of file) has been reached.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pbDest     - Pointer to local destination buffer.
**    iReqSize   - Requested size to read.
**    piActSize  - Pointer to UINT16, actual size read will be stored here on
**                 completion.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileRead( UINT16 iInstance, UINT8* pbDest, UINT16 iReqSize, UINT16* piActSize, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Write data to a file.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pbSrc      - Pointer to local source buffer.
**    iReqSize   - Requested size to write.
**    piActSize  - Pointer to UINT16, actual size written will be stored here on
**                 completion.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_FileWrite( UINT16 iInstance, UINT8* pbSrc, UINT16 iReqSize, UINT16* piActSize, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Open a directory.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the directory, relative to the current path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryOpen( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Close a directory.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryClose( UINT16 iInstance, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Delete a directory
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the directory to open, relative to the current
**                 path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryDelete( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Read a directory entry.
**
** A directory may contain multiple entries with the next entry being returned
** in the completion callback for each call to this function. As long as the
** name in the directory entry has a non-zero length there are more entries in
** the directory.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    psDest     - Pointer to "ANB_FSI_DirEntryType" structure, directory data
**                 will be stored here on completion.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryRead( UINT16 iInstance, ANB_FSI_DirEntryType* psDest, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Create a directory.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the directory to create, relative to the
**                 current path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryCreate( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback );

/*------------------------------------------------------------------------------
** Change the current directory.
**------------------------------------------------------------------------------
** Arguments:
**    iInstance  - FSI instance number to use.
**    pacName    - Path + name of the directory to change to, relative to the
**                 current path.
**    pnCallback - Completion callback.
** Returns:
**    ABCC_EC_NO_ERROR on success.
**    ABCC_EC_PARAMETER_NOT_VALID, ABCC_EC_NO_RESOURCES or
**    ABCC_EC_OUT_OF_MSG_BUFFERS on failure.
**------------------------------------------------------------------------------
*/
EXTFUNC ABCC_ErrorCodeType ANB_FSI_DirectoryChange( UINT16 iInstance, char* pacName, ANB_FSI_CompletionCbfType pnCallback );

#endif /* ANB_FSI_OBJ_ENABLE */

#endif  /* inclusion lock */
