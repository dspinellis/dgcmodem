
#ifndef __OSUNIQREDEF_H__
#define __OSUNIQREDEF_H__

/* prevent potential naming collisions with other modules. */
#define _OSUNIQDEF_PREFIX_STR "cnxtdgc_"
#define _OSUNIQDEF(f) cnxtdgc_##f
#define _OSUNIQDEF_STR(f) cnxtdgc_#f


#define DcpCreate _OSUNIQDEF(DcpCreate)
#define DcpDestroy _OSUNIQDEF(DcpDestroy)
#define DcpSetVolume _OSUNIQDEF(DcpSetVolume)
#define DcpCallback _OSUNIQDEF(DcpCallback)
#define OsDcpEnsureDaemonIsRunning _OSUNIQDEF(OsDcpEnsureDaemonIsRunning)

#define OsDcpInit _OSUNIQDEF(OsDcpInit)
#define OsDcpExit _OSUNIQDEF(OsDcpExit)

#endif /* __OSUNIQREDEF_H__ */

