
/* edit this file to make optimized versions without unnecessary parts */

#define VERS_STRING  "0.29b "   // version for monitor (exact 6 chars)
#define VERS_STRING_ "0.29b"    // version for startup string (any length)
#define VER_HL 0x001D           // major version
#define VER_A  0x00             // minor version and beta flag

#define MOD_GSZ80     // exact gs emulation through Z80/ROM/DACs
#define MOD_GSBASS    // fast gs emulation via sampleplayer and BASS mod-player
#define MOD_FASTCORE  // use optimized code for Z80 when no breakpoints set
#define MOD_SETTINGS  // win32 dialog with emulation settings and tape browser
#define MOD_MONITOR   // debugger

/* ************************************************************************* */
/* * don't edit below this line                                            * */
/* ************************************************************************* */

#if defined(MOD_GSBASS) || defined(MOD_GSZ80)
#define MOD_GS
#endif

#if defined(MOD_MONITOR)
#define MOD_DEBUGCORE
#endif

#if !defined(MOD_FASTCORE) && !defined(MOD_DEBUGCORE)
#define MOD_FASTCORE
#endif

/* ************************************************************************* */

