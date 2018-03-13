#ifndef	_RFM2G_WINDOWS_H_
#define	_RFM2G_WINDOWS_H_

	#if defined(_WIN32)

		/*****************************************************************
		*	VS 2005 has a new security feature that complains on lots
		*	of stdio routines. Defining this constant will shut them up.
		******************************************************************/
		#if	(_MSC_VER == 1400)
			#define	_CRT_SECURE_NO_DEPRECATE
		#endif

		#include <windows.h>

	#endif

#endif
