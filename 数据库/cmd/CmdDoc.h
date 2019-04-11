#ifndef __CMD_DOC_H__
#define __CMD_DOC_H__

#ifdef CMD_NEWVER
#ifdef CMD_VER
#undef CMD_VER
#endif
#endif

#ifndef CMD_VER
#define CMD_VER		0x0200
#endif

#if CMD_VER < 0x0200
#include "cmddoc1.h"
#else
#include "cmddoc2.h"
#endif
#endif