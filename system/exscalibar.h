#ifndef __EXSCALIBAR_H
#define __EXSCALIBAR_H

#define EXSCALIBAR_VERSION "1.1.1"

#ifdef _MSC_VER
  #ifdef BUILDING_DLL
	#define DLLEXPORT __declspec(dllexport)
  #else
	#define DLLEXPORT __declspec(dllimport)
  #endif
  #define DLLLOCAL
#else
  #if 1 || defined(HAVE_GCCVISIBILITYPATCH)
	#define DLLEXPORT __attribute__ ((visibility("default")))
	#define DLLLOCAL __attribute__ ((visibility("hidden")))
  #else
	#define DLLEXPORT
	#define DLLLOCAL
  #endif
#endif

#endif
