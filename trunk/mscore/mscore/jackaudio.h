//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: jackaudio.h,v 1.3 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __JACKAUDIO_H__
#define __JACKAUDIO_H__

#include "config.h"
#include "audio.h"

#ifdef USE_JACK
#include <jack/jack.h>

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

class JackAudio : public Audio {
      int _sampleRate;
      int _segmentSize;

      jack_client_t* client;
      char _jackName[8];
      jack_port_t* portR;
      jack_port_t* portL;

   public:
      JackAudio();
      virtual ~JackAudio();
      virtual bool init();
      void* registerPort(const char* name);
      void unregisterPort(void* p);
      virtual std::list<QString> inputPorts();
      virtual bool start();
      virtual bool stop();
      int framePos() const;
      void connect(void*, void*);
      void disconnect(void* src, void* dst);
      float* getLBuffer(long n) { return (float*)jack_port_get_buffer(portL, n); }
      float* getRBuffer(long n) { return (float*)jack_port_get_buffer(portR, n); }
      virtual bool isRealtime() const   { return jack_is_realtime(client); }
      virtual void startTransport();
      virtual void stopTransport();
      virtual int getState();
      virtual int sampleRate() const { return _sampleRate; }
      };

#else

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

class JackAudio : public Audio {
   public:
      JackAudio() {}
      virtual ~JackAudio() {}
      virtual bool init() { return true; }
      void* registerPort(const char*) { return 0; }
      void unregisterPort(void*) {}
      virtual std::list<QString> inputPorts() {
            std::list<QString> a;
            return a;
            }
      virtual bool start() { return false; }
      virtual bool stop()  { return false; }
      int framePos() const { return 0; }
      void connect(void*, void*) {}
      void disconnect(void*, void*) {}
      float* getLBuffer(long) { return 0; }
      float* getRBuffer(long) { return 0; }
      virtual bool isRealtime() const   { return false; }
      virtual void startTransport() {}
      virtual void stopTransport()  {}
      virtual int getState()        { return 0; }
      virtual int sampleRate() const { return 48000; }
      };

#endif
#endif



