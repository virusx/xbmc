#pragma once
/*
 *      Copyright (C) 2011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "IFile.h"
#include <mmdeviceapi.h>
#include <Audioclient.h>

namespace XFILE
{
  typedef struct
  {
    char riff[4];
    DWORD filesize;
    char rifftype[4];
    char fmt_chunk_id[4];
    DWORD fmt_chunksize;
    WAVEFORMATEXTENSIBLE wfex;
    char data_chunk_id[4];
    DWORD data_chunksize;
  } WAVE_HEADER_DATA;

  class CFileDeviceWASAPI : public IFile
  {
    public:
      CFileDeviceWASAPI();
      virtual ~CFileDeviceWASAPI();
      virtual bool Open(const CURL& url);
      virtual void Close();
      virtual bool Exists(const CURL& url);
      virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
      virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
      virtual int IoControl(EIoControl request, void* param);
      virtual int64_t GetPosition()                              { return m_pos; };
      virtual int64_t GetLength()                                { return 0xFFFFFFFF; } /* Max wave(32) size, about 3 hours of audio */
      virtual int Stat(const CURL& url, struct __stat64* buffer) { errno = ENOENT; return -1; }
      virtual CStdString GetContent()                            { return "audio/wav"; }

    private:
      IMMDevice *GetDevice(const CStdString &strDevice);

      WAVE_HEADER_DATA     m_waveHeader;
      IMMDevice           *m_pDevice;
      IAudioClient        *m_pAudioClient;
      IAudioCaptureClient *m_pCaptureClient;

      int64_t m_pos;
  };
}
