/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\
 Platform description
 $Header:$
\*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#ifndef H_PLATFORM_H
#define H_PLATFORM_H

#ifndef __DOSLIKE__
#if defined(__WIN32__) || defined(__ST32__)
    #define __DOSLIKE__
#endif
#endif

#if defined(__DJGPP__)
    #define __DOS16__
#endif

#if defined(__linux__) || defined(__QNX__) || defined(__LINUX__) || defined(__APPLE__) 
    #define __UNIXLIKE__
    #define _UNIXSTYLE_PATHS
#endif

#if defined(__WATCOMC__) || defined(__APPLE__)
    #define _NEED_STRING_H 
#endif	



#endif

// $History:$



