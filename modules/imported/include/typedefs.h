
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

#define OS_LINUX    0x00000080
#define OS_TYPE     OS_LINUX

#define TARGET_HCF_FAMILY 0

typedef void VOID, *PVOID;
typedef void *HANDLE;
typedef int BOOL;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef short     INT16;

#define FALSE 0
#define TRUE 1

#if ( OS_TYPE == OS_LINUX ) && defined(__i386__)
#define __shimcall__ __attribute__((regparm(0)))
#define __kernelcall__
#define __kernelcallregparm__ __attribute__((regparm(3)))
#define __kernelcallstkparm__ __attribute__((regparm(0)))
#else
#define __shimcall__
#define __kernelcall__
#define __kernelcallregparm__
#define __kernelcallstkparm__
#endif

#endif /* __TYPEDEFS_H__ */

