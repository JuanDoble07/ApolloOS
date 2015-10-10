/*
    Copyright � 2015, The AROS Development Team. All rights reserved.
    $Id$
*/

#include <config.h>

#include <devices/ahi.h>
#include <exec/execbase.h>
#include <libraries/ahi_sub.h>

#include "DriverData.h"
#include "library.h"

#include "alsa-bridge/alsa.h"

#define dd ((struct AlsaData*) AudioCtrl->ahiac_DriverData)

#define min(a,b) ( (a) < (b) ? (a) : (b) )

/******************************************************************************
** The slave process **********************************************************
******************************************************************************/

#undef SysBase

void Slave( struct ExecBase* SysBase );

#include <aros/asmcall.h>

AROS_UFH3(LONG, SlaveEntry,
      AROS_UFHA(STRPTR, argPtr, A0),
      AROS_UFHA(ULONG, argSize, D0),
      AROS_UFHA(struct ExecBase *, SysBase, A6))
{
   AROS_USERFUNC_INIT
   Slave( SysBase );
   AROS_USERFUNC_EXIT
}

void
Slave( struct ExecBase* SysBase )
{
  struct AHIAudioCtrlDrv* AudioCtrl;
  struct DriverBase*      AHIsubBase;
  struct AlsaBase*        AlsaBase;
  BOOL                    running;
  ULONG                   signals;
  LONG                    framesready = 0;
  APTR                    framesptr = NULL;

  AudioCtrl  = (struct AHIAudioCtrlDrv*) FindTask(NULL)->tc_UserData;
  AHIsubBase = (struct DriverBase*) dd->ahisubbase;
  AlsaBase   = (struct AlsaBase*) AHIsubBase;

  dd->slavesignal = AllocSignal( -1 );

  if( dd->slavesignal != -1 )
  {
    // Everything set up. Tell Master we're alive and healthy.

    Signal( (struct Task*) dd->mastertask,
            1L << dd->mastersignal );

    running = TRUE;

    while( running )
    {
      signals = SetSignal(0L,0L);

      if( signals & ( SIGBREAKF_CTRL_C | (1L << dd->slavesignal) ) )
      {
        running = FALSE;
      }
      else
      {
        LONG framesfree = 0;

        while(TRUE)
        {
          framesfree = ALSA_Avail(dd->alsahandle);
          if (framesfree == ALSA_XRUN)
          {
              ALSA_Prepare(dd->alsahandle);
              framesfree = ALSA_Avail(dd->alsahandle);
          }

          if (framesfree > 1024)
            break;
          Delay(1);
        }

        /* Loop until alsa buffer is filled */
        while (framesfree > 0)
        {
          LONG written;

          if (framesready == 0)
          {
            CallHookPkt(AudioCtrl->ahiac_PlayerFunc, AudioCtrl, NULL );
            CallHookPkt(AudioCtrl->ahiac_MixerFunc, AudioCtrl, dd->mixbuffer );
            framesready = AudioCtrl->ahiac_BuffSamples;
            framesptr = dd->mixbuffer;
          }

          written = ALSA_Write(dd->alsahandle, framesptr, min(framesready,
                  framesfree));
          if (written == ALSA_XRUN)
          {
              ALSA_Prepare(dd->alsahandle);
              written = ALSA_Write(dd->alsahandle, framesptr, min(framesready,
                      framesfree));
          }

          framesready -= written;
          framesfree  -= written;
          framesptr += written * 4;

          CallHookA(AudioCtrl->ahiac_PostTimerFunc, (Object*) AudioCtrl, 0);
        }
      }
    }
  }

  FreeSignal( dd->slavesignal );
  dd->slavesignal = -1;

  Forbid();

  // Tell the Master we're dying

  Signal( (struct Task*) dd->mastertask, 1L << dd->mastersignal );

  dd->slavetask = NULL;

  // Multitaking will resume when we are dead.
}
