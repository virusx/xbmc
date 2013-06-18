/*
 *      Copyright (C) 2013 Team XBMC
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ContentAddonCodec.h"

using namespace MUSIC_INFO;
using namespace ADDON;
using namespace std;

bool ContentAddonCodec::Init(const CStdString& strFile, unsigned int filecache)
{
  CLog::Log(LOGDEBUG, "ContentAddonCodec::Init(%s)", strFile.c_str());
  m_addon = CContentAddons::Get().GetAddonForPath(strFile);
  if (!m_addon || !m_addon->ProvidesMusicCodec())
  {
    CLog::Log(LOGERROR, "%s - cannot find any add-on to open '%s'", __FUNCTION__, strFile.c_str());
    return false;
  }

  if (!m_addon->MusicOpenFile(strFile.c_str()))
  {
    CLog::Log(LOGERROR, "%s - add-on '%s' failed to open '%s'", __FUNCTION__, m_addon->Name().c_str(), strFile.c_str());
    return false;
  }

  CONTENT_ADDON_CODEC_INFO info;
  if (m_addon->MusicGetCodecInfo(strFile, &info))
  {
    m_CodecName     = info.strCodecName;
    m_SampleRate    = info.iSampleRate;
    m_Channels      = info.iChannels;
    m_BitsPerSample = info.iBitsPerSample;
    m_Bitrate       = info.iBitrate;
    m_DataFormat    = (AEDataFormat)info.format;
    m_TotalTime     = 0;
    return true;
  }
  return false;
}

void ContentAddonCodec::DeInit(void)
{
  if (m_addon)
    m_addon->MusicCloseFile();
}

int64_t ContentAddonCodec::Seek(int64_t iSeekTime)
{
  return m_addon ? m_addon->MusicSeek(iSeekTime) : 0;
}

int ContentAddonCodec::ReadPCM(BYTE* pBuffer, int size, int* actualsize)
{
  return m_addon ? m_addon->MusicReadPCM(pBuffer, size, actualsize) : 0;
}

bool ContentAddonCodec::CanInit(void)
{
  return m_addon ? true : false;
}

CAEChannelInfo ContentAddonCodec::GetChannelInfo(void)
{
  //TODO more channel configs
  static enum AEChannel ae_channel_map[2][3] =
  {
      { AE_CH_FC, AE_CH_NULL },
      { AE_CH_FL, AE_CH_FR, AE_CH_NULL }
  };

  return CAEChannelInfo(ae_channel_map[m_Channels - 1]);
}
