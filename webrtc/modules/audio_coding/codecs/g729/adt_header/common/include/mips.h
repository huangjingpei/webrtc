#ifndef MIPS_H
#define MIPS_H

//#include "common/private/include/adt_typedef.h"
#include "common/include/adt_typedef_user.h"


#define ENTER_FUNC(void)                                        \
{                                                               \
   printf("\nEnter the %s at %d\n",__FUNCTION__,__LINE__);      \
}
                                                                                                                             
#define LEAVE_FUNC(void)                                        \
{                                                               \
   printf("\nLeave the %s at %d\n",__FUNCTION__,__LINE__);      \
}
                                                                                                                             
#define PRINT_VAL(val)                                          \
{                                                               \
   printf("Value is %d at %d of %s\n",val,__LINE__,__FUNCTION__); \
}


#if defined (DSP_TYPE)  // NOTE: Does not work without BIOS
//====================================================================
//                       BIOS
//{====================================================================

#include <std.h>
#include <clk.h>
#include <hwi.h>
#include <swi.h>

#if (DSP_TYPE==54)
   #define TDDR  ((ADT_UInt16 *)(0x26))
   #define InstPerClk ((*TDDR&0xF)+1)
   #define disableIntrs
   #define enableIntrs
#else
   #define InstPerClk CLK_cpuCyclesPerHtime()
   #define disableIntrs  HWI_disable()
   #define enableIntrs   HWI_enable()
#endif

#define TimeToUs(hres) ((hres * 1000) / CLK_countspms())

#define ALLOC_MIPS_TIMER(name)      \
struct name {                       \
  unsigned long   min, accum, max, frameCnt; \
  float  sampsPerFrame, sampsPerSec; \
  unsigned long   start, end;       \
  int    mask;                      \
} name = { (unsigned long) -1, 0, 0, 0 };

#define MIPS(mipsTimer,clksPerFrame)  \
   (((InstPerClk * (clksPerFrame) * mipsTimer.sampsPerSec) / (mipsTimer.sampsPerFrame * 1000000)))

#define MIPS_PERCENT(mipsTimer, usecPerFrame)             \
  ((((float) (usecPerFrame) /(float) mipsTimer.sampsPerFrame) * mipsTimer.sampsPerSec) / 10000)

#define INIT_MIPS_TIMER(mipsTimer, sampPerFrame, sampPerSec) 	\
{                                \
   mipsTimer.accum = 0;          \
   mipsTimer.min = (unsigned long) -1;   \
   mipsTimer.max = 0;            \
   mipsTimer.frameCnt = 0;       \
   mipsTimer.sampsPerFrame = (float) (sampPerFrame);    \
   mipsTimer.sampsPerSec   = (float) (sampPerSec);      \
}

#define START_MIPS_TIMER(mipsTimer) 	\
{                                     \
  disableIntrs;                       \
  mipsTimer.start = CLK_gethtime ();  \
}

#define STOP_MIPS_TIMER(mipsTimer)   \
{                                    \
    unsigned long delta;             \
    mipsTimer.end = CLK_gethtime (); \
    enableIntrs;                     \
    delta = mipsTimer.end - mipsTimer.start; \
    if (delta < mipsTimer.min) mipsTimer.min = delta; \
    if (mipsTimer.max < delta) mipsTimer.max = delta; \
    mipsTimer.accum += delta; \
    mipsTimer.frameCnt++;   \
}

#define MIPS_REPORT(mipsTimer, oper) 		\
{                                         \
   printf ("\nTiming statistics for %s\n", oper);                     \
   printf ("Total number of frames: %u\n", mipsTimer.frameCnt);       \
   if (mipsTimer.frameCnt) {                                          \
      printf ("time (us).     Min: %6d,  Avg: %6d,  Max: %6d \n",     \
      (ADT_UInt16) TimeToUs(mipsTimer.min),                           \
      (ADT_UInt16) TimeToUs(mipsTimer.accum/mipsTimer.frameCnt),      \
      (ADT_UInt16) TimeToUs(mipsTimer.max));                          \
                                                                      \
      printf ("usage (%%CPU).  Min: %6.3f,  Avg: %6.3f,  Max: %6.3f \n",        \
      MIPS_PERCENT (mipsTimer, TimeToUs(mipsTimer.min)),                        \
      MIPS_PERCENT (mipsTimer, TimeToUs((mipsTimer.accum/mipsTimer.frameCnt))), \
      MIPS_PERCENT (mipsTimer, TimeToUs(mipsTimer.max)));                       \
                                                                         \
      printf ("MIPS.          Min: %6.3f,  Avg: %6.3f,  Max: %6.3f \n",  \
            MIPS (mipsTimer,mipsTimer.min),                              \
            MIPS (mipsTimer,(mipsTimer.accum / mipsTimer.frameCnt)),     \
            MIPS (mipsTimer,mipsTimer.max));                             \
  }                                                                      \
}

#define MIPS_REPORT_TO_FILE(file, mipsTimer, oper)  MIPS_REPORT(mipsTimer, oper) 

#elif defined (LINUX) || defined(__APPLE__) || defined(ANDROID)
//}====================================================================
//                       LINUX
//{====================================================================
#include <sys/time.h>
#include <time.h>

#ifdef ANDROID
#define printf(...) printLogs(__VA_ARGS__)
#endif

#define ALLOC_MIPS_TIMER(name)     \
struct name     {                  \
  long   min, accum, max, frameCnt;  \
  float  sampsPerFrame, sampsPerSec;\
  struct timeval start, end;       \
} name = { 0x7fffffff, 0, 0, 0 };

#define MIPS_PERCENT(mipsTimer, usecPerFrame)             \
  ((((float) (usecPerFrame) /(float) mipsTimer.sampsPerFrame) * mipsTimer.sampsPerSec) / 10000)


#define INIT_MIPS_TIMER(mipsTimer, smpsPerFrame, smpsPerSec)  \
{                                \
   mipsTimer.accum = 0;            \
   mipsTimer.min = 0x7fffffff;   \
   mipsTimer.max = 0;            \
   mipsTimer.frameCnt = 0;       \
   mipsTimer.sampsPerFrame =  (float) (smpsPerFrame);       \
   mipsTimer.sampsPerSec   =  (float) (smpsPerSec);       \
}

#define START_MIPS_TIMER(mipsTimer)  \
{                                \
  struct timespec ts;  \
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); \
  mipsTimer.start.tv_sec=ts.tv_sec; \
  mipsTimer.start.tv_usec=ts.tv_nsec/1000; \
}

#define STOP_MIPS_TIMER(mipsTimer)  \
{                                      \
    long delta;                          \
    struct timespec ts;  \
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts); \
	mipsTimer.end.tv_sec=ts.tv_sec; \
	mipsTimer.end.tv_usec=ts.tv_nsec/1000; \
    if (mipsTimer.end.tv_usec < mipsTimer.start.tv_usec) \
      delta = mipsTimer.end.tv_usec - mipsTimer.start.tv_usec + 1000000; \
    else \
      delta = mipsTimer.end.tv_usec - mipsTimer.start.tv_usec; \
    if (delta < mipsTimer.min) mipsTimer.min = delta; \
    if (delta > mipsTimer.max) mipsTimer.max = delta; \
    mipsTimer.accum += delta; \
    mipsTimer.frameCnt++;   \
}

#define MIPS_REPORT(mipsTimer, oper)  \
{                                                                       \
   printf ("\nStatistics Information for %s\n", oper);                  \
   printf ("Total number of frames: %d\n", (int) mipsTimer.frameCnt);         \
   printf ("%s time (us).          Min: %d,  Avg: %d, Max: %d \n", oper,  \
   (int) mipsTimer.min, (int) (mipsTimer.accum/mipsTimer.frameCnt), (int) mipsTimer.max); \
                                                                    \
   printf ("%s time (CPU Ratio).   Min: %f,  Avg: %f, Max: %f \n", oper,  \
   MIPS_PERCENT (mipsTimer, mipsTimer.min), MIPS_PERCENT (mipsTimer, mipsTimer.accum/mipsTimer.frameCnt),  \
   MIPS_PERCENT (mipsTimer, mipsTimer.max)); \
}
#ifdef ANDROID
#define MIPS_REPORT_TO_FILE(file, mipsTimer, oper)  MIPS_REPORT(mipsTimer, oper) 
#else
#define MIPS_REPORT_TO_FILE(file, mipsTimer, oper) \
{                                                                          \
    fprintf (file, "Total number of frames: %d\n", (int) mipsTimer.frameCnt);     \
    fprintf (file, "Statistics Information of %s\n",  oper);                  \
    fprintf (file, "%s time (us).          Min: %d,  Avg: %d, Max: %d \n", oper,  \
        (int)mipsTimer.min, (int)(mipsTimer.accum/mipsTimer.frameCnt), (int)mipsTimer.max); \
    fprintf (file, "%s time (CPU Ratio).   Min: %f,  Avg: %f, Max: %f \n", oper,  \
        MIPS_PERCENT (mipsTimer, mipsTimer.min), MIPS_PERCENT (mipsTimer, mipsTimer.accum/mipsTimer.frameCnt),  \
        MIPS_PERCENT (mipsTimer, mipsTimer.max)); \
}
#endif

#elif defined (WIN32)
//}====================================================================
//                       WINDOWS
//{====================================================================
/* Although Windows doesn't have a sufficiently high resolution timer, we can still try to use this 
   mechanism to compute average execution time 
 */

#include <windows.h>
#include <winreg.h>

#define ALLOC_MIPS_TIMER(name)     \
struct name {                      \
  ADT_UInt64  min, accum, max, frameCnt;  \
  float  cpuMHz, timerMHz, sampsPerFrame, sampsPerSec;          \
  LARGE_INTEGER start, end;                                     \
} name = { (ADT_UInt64) -1, 0, 0, 0 };  

static __inline ADT_UInt32 get_CPUMHz() {
   long lError;
   DWORD BufSize;
   DWORD Freq;
   HKEY hKey;

   // open the key where the proc speed is hidden:
   lError = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey); 
   if (lError != ERROR_SUCCESS) return 0;

   // query the key
   Freq = 0;
   BufSize = sizeof (Freq);
   RegQueryValueEx (hKey, "~MHz", NULL, NULL, (LPBYTE) &Freq, &BufSize);

   return (ADT_UInt32) Freq;
}

#define MICRO_SECS(mipsTimer,clks) \
   ((int) (((float)(clks) / mipsTimer.timerMHz) ))

#define MIPS(mipsTimer,usPerFrame)  \
   ((float) (((usPerFrame) * mipsTimer.cpuMHz) / 1000000.))

#define MIPS_PERCENT(mipsTimer, uSecsPerFrame)             \
  ((((float) (uSecsPerFrame) /(float) mipsTimer.sampsPerFrame) * mipsTimer.sampsPerSec) / 10000)

#define INIT_MIPS_TIMER(mipsTimer, sampPerFrame, sampPerSec) \
{                                \
   mipsTimer.accum = 0;          \
   mipsTimer.min = 0xffffffff;   \
   mipsTimer.max = 0;            \
   mipsTimer.frameCnt = 0;       \
   mipsTimer.sampsPerFrame = (float) (sampPerFrame);              \
   mipsTimer.sampsPerSec   = (float) (sampPerSec);                \
   mipsTimer.cpuMHz = (float) get_CPUMHz ();                      \
   QueryPerformanceFrequency (&mipsTimer.end);                    \
   mipsTimer.timerMHz = (float) ((float)mipsTimer.end.QuadPart / 1000000.);  \
}

#define START_MIPS_TIMER(mipsTimer) \
{                                \
  QueryPerformanceCounter (&mipsTimer.start);                    \
}

#define STOP_MIPS_TIMER(mipsTimer) 	\
{                                   \
    ADT_UInt64 delta;                          \
    QueryPerformanceCounter (&mipsTimer.end); \
    delta = (mipsTimer.end.QuadPart - mipsTimer.start.QuadPart); \
    if (delta < mipsTimer.min) mipsTimer.min = delta; \
    if (delta > mipsTimer.max) mipsTimer.max = delta; \
    mipsTimer.accum += delta; \
    mipsTimer.frameCnt++;     \
}
#define STOP_MIPS_TIMER_N(mipsTimer, nFrames) 	\
{                                             \
    ADT_UInt64 delta;                          \
    QueryPerformanceCounter (&mipsTimer.end); \
    delta = (mipsTimer.end.QuadPart - mipsTimer.start.QuadPart); \
    if (delta < mipsTimer.min) mipsTimer.min = delta; \
    if (delta > mipsTimer.max) mipsTimer.max = delta; \
    mipsTimer.accum += delta; \
    mipsTimer.frameCnt += nFrames;   \
}

#define MIPS_REPORT(mipsTimer, oper)                                \
{                                                                   \
   ADT_UInt64 avgTicks = mipsTimer.accum/mipsTimer.frameCnt;        \
   printf ("\nStatistics Information for %s\n", oper);              \
   printf ("Total number of frames: %d\n", (int) mipsTimer.frameCnt);     \
   printf ("  %s time (us).            Avg: %6u\n", oper,           \
      MICRO_SECS (mipsTimer, avgTicks));                            \
   printf ("  %s time (CPU Ratio %%).   Avg: %6.2f\n", oper,        \
      MIPS_PERCENT (mipsTimer, MICRO_SECS (mipsTimer, avgTicks))    \
   );                                                               \
   if (mipsTimer.cpuMHz < 1000)                                     \
      printf( "  CPU Speed: %3.2f MHz\n",mipsTimer.cpuMHz);         \
   else                                                             \
      printf( "  CPU Speed: %3.2f GHz\n",mipsTimer.cpuMHz/1000.0f); \
   printf ("  %s MIPS (MCPS). Avg: %3.2f\n\n",oper,                 \
         MIPS (mipsTimer, MICRO_SECS (mipsTimer, avgTicks)));       \
   if (mipsTimer.accum == 0)                                        \
   { \
     printf("  !! This operation seems to take less time than is measurable on this platform. \n"); \
     printf("  !! You may want to try operating with a larger frame size or making multiple calls between START_MIPS_TIMER and END_MIPS_TIMER\n"); \
   }        \
}


#define MIPS_REPORT_TO_FILE(file, mipsTimer, oper)                                 \
{                                                                                  \
   ADT_UInt64 avgTicks = mipsTimer.accum/mipsTimer.frameCnt;                       \
   fprintf (file, "\n \n----Statistics Information of %s ----\n", oper);           \
   fprintf (file, "  Total number of frames: %d\n", (int) mipsTimer.frameCnt);     \
   fprintf (file, "  %s time (us). Avg: %d\n", oper,                               \
      MICRO_SECS (mipsTimer, avgTicks));                                           \
   fprintf (file, "  %s time (CPU Ratio %%).   Avg: %f\n", oper,                   \
      MIPS_PERCENT (mipsTimer, MICRO_SECS (mipsTimer,avgTicks))                    \
   );                                                                              \
   if (mipsTimer.cpuMHz < 1000)                                                    \
      fprintf(file,  "  CPU Speed: %3.2f MHz\n",mipsTimer.cpuMHz);                 \
   else                                                                            \
      fprintf(file,  "  CPU Speed: %3.2f GHz\n",mipsTimer.cpuMHz/1000.0f);         \
   fprintf (file, "  %s MIPS (MCPS). Avg: %3.2f\n\n",oper,                         \
         MIPS (mipsTimer, MICRO_SECS (mipsTimer, avgTicks)));                      \
   if (mipsTimer.accum == 0)                                                       \
   {                         \
     fprintf(file, "  !! This operation seems to take less time than is measurable on this platform. \n"); \
     fprintf(file, "  !! You may want to try operating with a larger frame size or making multiple calls between START_MIPS_TIMER and END_MIPS_TIMER\n"); \
   }                         \
 }

#else

//}====================================================================
//                       UNAVAILABLE TIMING
//====================================================================

#define ALLOC_MIPS_TIMER(name) int name = 0;
#define INIT_MIPS_TIMER(mipsTimer,sampsPerFrame,sampsPerSec) 	//
#define START_MIPS_TIMER(mipsTimer) //
#define STOP_MIPS_TIMER(mipsTimer) 	mipsTimer++;
#define MIPS_REPORT(mipsTimer, oper) \
   printf ("Total number of frames: %d\n", (int) mipsTimer);     \
   printf ("\nUndefined platform\n");

#endif



#endif
