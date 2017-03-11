/*___________________________________________________________________________
  |
  |   File: IOutils.h
  |
  |   This file is the Adaptive Digital Technologies, Inc.
  |   header file containing environment defines, macros, and
  |   I/O utilities.
  |
  |   This software is the property of Adaptive Digital Technologies, Inc.
  |   Unauthorized use is prohibited.
  |
  |   Copyright (c) 2000, Adaptive Digital Technologies, Inc.
  |
  |   Version 1.1
  |   February 7, 2000
  |
  |   www.adt-inc.com
  |   610-825-0182
  |___________________________________________________________________________
 */
#ifndef IOUTILS_H
#define IOUTILS_H
//---------------------------------------------------------
// The demo can be run in one of the following environments
// In order to select the correct environment, this file
// should be compiled with the -dRUNTIME_ENVIRONMENT compiler switch set
// to one of the following 3 defines.
// For example:
//   CL500 -dRTE=CCS DEMO.C
//---------------------------------------------------------

#define CCS         0   // Code Composer Debugger with File I/O
#define QS_FF       1   // DSP Research Quick-Start with file I/O
#define QS_RT       2   // DSP Research Quick-Start with Analog I/O
#define MSC_        3   // Microsft C
#define APPLE       4

#ifdef _TMS320C6X
#define RTE 0 // CCS
#elif defined(__APPLE_CC__) || defined(TEST_VFILE)
#define RTE APPLE
#else
#define RTE 3 // _MSC
#endif

//---------------------------------------------------------
// Include the correct files based upon environment
//---------------------------------------------------------
#if ((RTE == CCS) || (RTE == MSC_) || (RTE==APPLE))
#include <stdio.h>
#else
#include <qs.h>
#include <string.h>
#endif

//#include <math.h>
#include <stdlib.h>

#if RTE == MSC_
#define _OPEN_READ(File, Disposition)       fopen(File, "rb")
#define _OPEN_WRITE(File, Disposition)      fopen(File, "wb")
#define _READ_WORDS(Buff, Num, File)        fread(Buff, 2, Num, File)
#define _WRITE_WORDS(Buff, Num, File)       fwrite(Buff, 2, Num, File)
#define _FILE FILE
#define _CLOSE fclose
#endif

//---------------------------------------------------------
// Define the generic I/O functions as a function of the RTE
//---------------------------------------------------------
#if RTE == CCS
#define _OPEN_READ(File, Disposition)       fopen(File, "rb")
#define _OPEN_WRITE(File, Disposition)      fopen(File, "wb")
#define _READ_WORDS(Buff, Num, File)        fread(Buff, 2, Num, File)
#define _WRITE_WORDS(Buff, Num, File)       fwrite(Buff, 2, Num, File)
#endif
#if RTE == APPLE
#include "../test/vfile.h"
#define _OPEN_READ(File, Disposition)       VF_open(File, "rb")
#define _OPEN_WRITE(File, Disposition)      VF_open(File, "wb")
#define _READ_WORDS(Buff, Num, File)        VF_read_words(Buff, Num, File)
#define _WRITE_WORDS(Buff, Num, File)       VF_write_words(Buff, Num, File)
#define _FILE VFile_t
#define _CLOSE VF_close
#endif

/*
#if RTE == CCS
#define _ENVIRONMENT_NAME                   "Code Composer"
#define _INIT_ENVIRONMENT(a)                minit()
#define _OPEN_READ(File, Disposition)       fopen(File, "rb")
#define _OPEN_WRITE(File, Disposition)      fopen(File, "wb")
#define _OPEN_READ_WRITE(File, Disposition) fopen(File, "rb")
#define _CLOSE(File)                        fclose(File)
#define _PRINTF(a, b)                       printf(a, b)
#define _EXIT(a)                            exit(a)
#define _FEOF(File)                         feof(File)
#define _FILE FILE

short int _READ_WORDS(short int Buffer[], short int Count,_FILE *File)
{
short int HB, LB;
short int i = 0;

while ((i < Count) && !_FEOF(File))
{
fread(&LB, 1, 1, File);
fread(&HB, 1, 1, File);
Buffer[i] = (HB << 8) + (LB & 0xff);
i += 1;
}

return (i);
}

short int _WRITE_WORDS(short int Buffer[], short int Count, _FILE *File)
{
short int HB, LB;
short int i = 0;

for (i=0; i<Count; i++)
{
LB = Buffer[i] >> 8;
HB = Buffer[i] & 0xff;
fwrite(&HB, 1, 1, File);
fwrite(&LB, 1, 1, File);
}

return (i);
}
#endif
 */

#if RTE == QS_FF
#define _ENVIRONMENT_NAME                   "Quick Start File to File"
#define _INIT_ENVIRONMENT(a)                QS_init(a)
#define _OPEN_READ(File, Disposition)       QS_fopen(File, "rb")
#define _OPEN_WRITE(File, Disposition)      QS_fopen(File, "wb")
#define _OPEN_READ_WRITE(File, Disposition) QS_fopen(File, "rb")
#define _READ_WORDS(Buffer, Count, File)    QS_fread((Ptr) Buffer, 16, Count, File)
#define _WRITE_WORDS(Buffer, Count, File)   QS_fwrite((Ptr) Buffer, 16, Count, File)
#define _CLOSE(File)                        QS_fclose(File)
#define _PRINTF(a, b)                       QS_printf(a, b)
#define _EXIT(a)                            QS_exit(a)
#define _FEOF(File)                         QS_eof(File)
#define _FILE FILE
#endif

#if RTE == QS_RT
#define _ENVIRONMENT_NAME                   "Analog I/O in Real Time"
#define ANALOG_BUFFSIZE                     256
#define LEFT                                0
#define RIGHT                               1
#define _INIT_ENVIRONMENT(a)                QS_init(a)
#define _OPEN_READ(File, Disposition)       QS_aopen(8000, "r", ANALOG_BUFFSIZE, LEFT)
#define _OPEN_WRITE(File, Disposition)      QS_aopen(8000, "w", ANALOG_BUFFSIZE, LEFT)
#define _OPEN_READ_WRITE(File, Disposition) QS_aopen(8000,"rw", ANALOG_BUFFSIZE,LEFT)
#define _READ_WORDS(Buffer, Count, File)    QS_aread(Buffer,Count,File)
#define _WRITE_WORDS(Buffer, Count, File)   QS_awrite(Buffer,Count,File)
#define _CLOSE(File)                        QS_aclose(File)
#define _PRINTF(a, b)                       QS_printf(a,b)
#define _EXIT(a)                            QS_exit(a)
#define _FEOF(File)
#define _FILE                               AUDIO

#endif

//---------------------------------------------------------
// Define command line parameter order for Quick-Start invoke
//---------------------------------------------------------
#define PARAM_INFILE_NAME   1
#define PARAM_OUTFILE_NAME  2

#endif // ifndef SDKIO_H

/* ================================== end of file ===================================*/

