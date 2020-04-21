#ifndef OSFR_VERSION_INCLUDED
#define OSFR_VERSION_INCLUDED

#define MAKE_STR_HELPER(a_str) #a_str
#define MAKE_STR(a_str) MAKE_STR_HELPER(a_str)

#define OSFR_VERSION_MAJOR	1
#define OSFR_VERSION_MINOR	0
#define OSFR_VERSION_PATCH	0
#define OSFR_VERSION_BETA	0
#define OSFR_VERSION_VERSTRING	MAKE_STR(OSFR_VERSION_MAJOR) "." MAKE_STR(OSFR_VERSION_MINOR) "." MAKE_STR(OSFR_VERSION_PATCH) "." MAKE_STR(OSFR_VERSION_BETA)

#endif