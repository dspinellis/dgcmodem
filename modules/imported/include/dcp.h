#ifndef __DCP_H__
#define __DCP_H__

#include "typedefs.h"
#include "comtypes.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined NO_DCP_SUPPORT
__shimcall__
HANDLE DcpCreate(HANDLE hDevNode);
__shimcall__
VOID DcpDestroy  (HANDLE hDcp);
__shimcall__
VOID DcpSetVolume(HANDLE hDcp, int nVolume);
__shimcall__
VOID DcpCallback(HANDLE hDcp, PVOID pData, UINT32 dwSize);
#endif  /* !defined NO_DCP_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif      /* #ifndef __DCP_H__ */
