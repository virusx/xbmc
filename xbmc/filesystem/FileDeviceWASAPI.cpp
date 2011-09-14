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

#include "system.h"
#include "FileDeviceWASAPI.h"
#include <Functiondiscoverykeys_devpkey.h>
#include "utils/CharsetConverter.h"
#include "utils/EndianSwap.h"
#include "utils/log.h"
#include "utils/SystemInfo.h"

using namespace XFILE;

const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

#define BUFFER_LENGTH 50 // milliseconds (minimum value is about 20ms, depending on the audio card)

#define EXIT_ON_FAILURE(hr, reason, ...) if(FAILED(hr)) {CLog::Log(LOGERROR, reason, __VA_ARGS__); goto failed;}

CFileDeviceWASAPI::CFileDeviceWASAPI() : m_pDevice(NULL), m_pAudioClient(NULL), m_pCaptureClient(NULL), m_pos(0)
{
  // Fill in wave header data
  memcpy(&m_waveHeader.riff, "RIFF", 4); // File structure
  m_waveHeader.filesize = 0xFFFFFFFF - 8; // Max RIFF chunk size = 4 GB (max wave file size) minus 8-byte RIFF header
  m_waveHeader.filesize = Endian_SwapLE32(m_waveHeader.filesize); // Data sizes are little endian
  memcpy(&m_waveHeader.rifftype, "WAVE", 4);
  memcpy(&m_waveHeader.fmt_chunk_id, "fmt ", 4);
  m_waveHeader.fmt_chunksize = sizeof(WAVEFORMATEXTENSIBLE); // Format data
  m_waveHeader.fmt_chunksize = Endian_SwapLE32(m_waveHeader.fmt_chunksize);
  memset(&m_waveHeader.wfex, 0, sizeof(WAVEFORMATEXTENSIBLE)); // m_wavHeaderData.wfex is the only variant header data
  memcpy(&m_waveHeader.data_chunk_id, "data", 4); // ID indicating the start of the data chunk
  m_waveHeader.data_chunksize = 0xFFFFFFFF - sizeof(WAVE_HEADER_DATA); // 4 GB minus everything here
  m_waveHeader.data_chunksize = Endian_SwapLE32(m_waveHeader.data_chunksize);
}

CFileDeviceWASAPI::~CFileDeviceWASAPI()
{
  Close();
}

void CFileDeviceWASAPI::Close()
{
  if (!m_pDevice && !m_pAudioClient && !m_pCaptureClient)
    return;

  // Stop recording
  if (m_pAudioClient && FAILED(m_pAudioClient->Stop()))
    CLog::Log(LOGERROR, __FUNCTION__": Count not stop recording from the WASAPI audio client. Cleaning up anyways.");

  SAFE_RELEASE(m_pCaptureClient);
  SAFE_RELEASE(m_pAudioClient);
  SAFE_RELEASE(m_pDevice);
}

bool CFileDeviceWASAPI::Open(const CURL &url)
{
  Close();

  IMMDeviceEnumerator *pEnumerator = NULL;
  IPropertyStore *pProperty = NULL;

  CStdString strDevice(url.GetFileName());
  if (!strDevice.Equals("default"))
  {
    m_pDevice = GetDevice(strDevice);
    if (!m_pDevice)
    {
      CLog::Log(LOGERROR, __FUNCTION__": Could not locate the device named \"%s\" in the list of WASAPI capture devices.", strDevice.c_str());
      return false;
    }
  }
  else /* using device://default */
  {
    PROPVARIANT varName;

    HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Could not allocate WASAPI device enumerator. CoCreateInstance error code: %li", hr)

    hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &m_pDevice);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Could not retrieve the default WASAPI capture device.")

    hr = m_pDevice->OpenPropertyStore(STGM_READ, &pProperty);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of WASAPI capture device properties failed.")

    hr = pProperty->GetValue(PKEY_Device_FriendlyName, &varName);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of WASAPI capture device name failed.")

    // Initialization of CStdStringW might be skipped by insidious goto statements
    {
      CStdStringW strRawDevName(varName.pwszVal);
      g_charsetConverter.ucs2CharsetToStringCharset(strRawDevName, strDevice);
    }
    
    CLog::Log(LOGINFO, "Using default capture device \"%s\".", strDevice.c_str());
    PropVariantClear(&varName);
    SAFE_RELEASE(pProperty);
    SAFE_RELEASE(pEnumerator);
  }

  // Now that we have a capture device...

  WAVEFORMATEXTENSIBLE *wfex = NULL;
  unsigned int bufferFrameCount;
  HRESULT hr;

  hr = m_pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Activating the WASAPI capture device failed.")

  hr = m_pAudioClient->GetMixFormat((WAVEFORMATEX **)&wfex);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of WASAPI mix format failed.")

  memcpy(&m_waveHeader.wfex, wfex, sizeof(WAVEFORMATEXTENSIBLE));
  CoTaskMemFree(wfex);
  wfex = NULL;

  // Allocate the audio capture buffer
  hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, BUFFER_LENGTH * 10000, 0, &m_waveHeader.wfex.Format, NULL);
  EXIT_ON_FAILURE(hr, __FUNCTION__": WASAPI initialization failed.")

  hr = m_pAudioClient->GetBufferSize(&bufferFrameCount);
  EXIT_ON_FAILURE(hr, __FUNCTION__": GetBufferSize() failed.")
  CLog::Log(LOGDEBUG, "Using capture device \"%s\" with %ims buffer.", strDevice.c_str(), 1000 * bufferFrameCount / m_waveHeader.wfex.Format.nSamplesPerSec);

  hr = m_pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&m_pCaptureClient);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Could not initialize the WASAPI capture client interface.")

  hr = m_pAudioClient->Start(); // Start recording
  EXIT_ON_FAILURE(hr, __FUNCTION__": Could not start recording from the WASAPI audio client.")
  return true;

failed:
  CoTaskMemFree(wfex);
  SAFE_RELEASE(pProperty);
  SAFE_RELEASE(pEnumerator);
  SAFE_RELEASE(m_pCaptureClient);
  SAFE_RELEASE(m_pAudioClient);
  SAFE_RELEASE(m_pDevice);
  return false;
}

bool CFileDeviceWASAPI::Exists(const CURL &url)
{
  CStdString strDevice(url.GetFileNameWithoutPath());
  IMMDevice *pDevice = GetDevice(strDevice);
  if (pDevice != NULL)
  {
    SAFE_RELEASE(pDevice);
    return true;
  }
  return false;
}

IMMDevice *CFileDeviceWASAPI::GetDevice(const CStdString &strDevice)
{
  CLog::Log(LOGDEBUG, __FUNCTION__": searching for capture device \"%s\"", strDevice.c_str());

  // First check if the version of Windows we are running on even supports WASAPI
  if (!g_sysinfo.IsVistaOrHigher())
  {
    CLog::Log(LOGERROR, __FUNCTION__": WASAPI capture requires Vista or higher.");
    return NULL;
  }

  IMMDeviceEnumerator *pEnumerator = NULL;
  IMMDeviceCollection *pEnumDevices = NULL;
  IMMDevice *pDevice = NULL;
  UINT uiCount = 0;

  HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Could not allocate WASAPI device enumerator. CoCreateInstance error code: %li", hr)

  hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pEnumDevices);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of audio capture enumeration failed.")

  hr = pEnumDevices->GetCount(&uiCount);
  EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of audio capture count failed.")

  for (UINT i = 0; i < uiCount; i++)
  {
    IPropertyStore *pProperty = NULL;
    PROPVARIANT varName;

    pEnumDevices->Item(i, &pDevice);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Retrieval of WASAPI capture failed.")

    hr = pDevice->OpenPropertyStore(STGM_READ, &pProperty);
    if (FAILED(hr))
    {
      CLog::Log(LOGERROR, __FUNCTION__": Retrieval of WASAPI capture properties failed.");
      SAFE_RELEASE(pProperty);
      goto failed;
    }

    hr = pProperty->GetValue(PKEY_Device_FriendlyName, &varName);
    if (FAILED(hr))
    {
      CLog::Log(LOGERROR, __FUNCTION__": Retrieval of WASAPI capture device name failed.");
      SAFE_RELEASE(pProperty);
      goto failed;
    }

    CStdStringW strRawDevName(varName.pwszVal);
    CStdString strDevName;
    g_charsetConverter.wToUTF8(strRawDevName, strDevName);

    if (strDevice == strDevName)
      i = uiCount; // exit the for loop
    else
      SAFE_RELEASE(pDevice); // prepare for next iteration

    CLog::Log(LOGDEBUG, __FUNCTION__": found capture device \"%s\"", strDevName.c_str());
    
    PropVariantClear(&varName);
    SAFE_RELEASE(pProperty);
  }

  SAFE_RELEASE(pEnumDevices);
  SAFE_RELEASE(pEnumerator);
  return pDevice;

failed:

  if (FAILED(hr))
    CLog::Log(LOGERROR, __FUNCTION__": Failed to enumerate WASAPI capture devices.");

  SAFE_RELEASE(pDevice);
  SAFE_RELEASE(pEnumDevices);
  SAFE_RELEASE(pEnumerator);
  return NULL;
}

unsigned int CFileDeviceWASAPI::Read(void* lpBuf, int64_t uiBufSize)
{
  int count = 0;
  int pos_offset = sizeof(WAVE_HEADER_DATA);

  // Read() is trying to read the wav header. Read from our generated header data
  // If the amount to read passes the end of the header data, we continue reading from the device
  if (m_pos < sizeof(WAVE_HEADER_DATA))
  {
    // (pos_offset - m_pos) is the number of bytes until we start reading device data
    count = (int)std::min(uiBufSize, pos_offset - m_pos);
    memcpy(lpBuf, (BYTE*)&m_waveHeader + m_pos, count);
    lpBuf = (BYTE*)lpBuf + count;
    uiBufSize -= count;
    m_pos += count;
  }

  BYTE *pSharedBuffer;
  // If the amount to read passes the end of the header data, we continue reading from the device
  while (uiBufSize > 0)
  {
    DWORD flags;
    unsigned int packetLength, numFramesAvailable;
    HRESULT hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Could not get the packet size from the WASAPI capture client.")
    if (!packetLength)
    {
      // Sleep until we have data to return (returning 0 signifies a read error and closes the file)
      if (count)
        break;
      Sleep(1);
      continue;
    }

    // Get the available data in the shared buffer
    hr = m_pCaptureClient->GetBuffer(&pSharedBuffer, &numFramesAvailable, &flags, NULL, NULL);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Could not read the buffer from the WASAPI capture client.")
    int iAvailable = numFramesAvailable * (m_waveHeader.wfex.Format.wBitsPerSample >> 3) * m_waveHeader.wfex.Format.nChannels;

    // Check to see if requested data is than less available data. m_pos is used to mark where we last read from
    if (uiBufSize < iAvailable - (m_pos - pos_offset))
    {
      // Consume the whole buffer and increase m_pos
      memcpy(lpBuf, pSharedBuffer + m_pos - pos_offset, (size_t)uiBufSize);
      count += (int)uiBufSize;
      m_pos += uiBufSize;
      // The ReleaseBuffer's argument can be 0 or numFramesAvailable. 0 means leave the data for next time
      hr = m_pCaptureClient->ReleaseBuffer(0);
      EXIT_ON_FAILURE(hr, __FUNCTION__": Could not release the buffer from the WASAPI capture client.")
      break;
    }
    else if (m_pos - pos_offset < iAvailable)
    {
      // Requested more data than is available
      // Consume whatever is available and advance the buffer to the next packet
      int packetsize = iAvailable - ((int)m_pos - pos_offset);
      if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT))
        memcpy(lpBuf, pSharedBuffer + m_pos - pos_offset, packetsize);
      else
        memset(lpBuf, 0, packetsize); // Write silence if the silent flag is set
      count += packetsize;
      lpBuf = (BYTE*)lpBuf + packetsize;
      uiBufSize -= packetsize;
    }
    else
    {
      CLog::Log(LOGERROR, __FUNCTION__": buffer position exceeds packet size, skipping packet.");
    }

    // Advance to the next packet
    hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
    EXIT_ON_FAILURE(hr, __FUNCTION__": Could not advance the WASAPI buffer to the next audio packet.")

    // Realign m_pos with the beginning of the device data (in other words,
    // simulate next packet beginning immediately after wave header data)
    m_pos = pos_offset;
  }

  return count;
failed:
  // If reading the device data failed, this will be zero (unless we succeeded in reading from the header data)
  return count;
}

int64_t CFileDeviceWASAPI::Seek(int64_t iFilePosition, int iWhence)
{
  // Only perform the seek if it puts us in the header or at the start of the data section
  int64_t new_pos = m_pos;
  switch (iWhence)
  {
  case SEEK_SET:
    new_pos = iFilePosition;
    break;

  case SEEK_CUR:
    new_pos += iFilePosition;
    break;

  case SEEK_END:
    new_pos = GetLength() + iFilePosition;
    break;

  default:
    break;
  }
  if (new_pos <= sizeof(WAVE_HEADER_DATA))
    m_pos = new_pos;
  return m_pos;
}

int CFileDeviceWASAPI::IoControl(EIoControl request, void* param)
{
  if (request == IOCTRL_SEEK_POSSIBLE)
    return 1;

  return -1;
}
