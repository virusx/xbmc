/*
 *      Copyright (C) 2005-2008 Team XBMC
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

//#include "threads/SystemClock.h"
//#include "system.h"
//#include "signal.h"
//#include "limits.h"
//#include "threads/SingleLock.h"
//#include "guilib/AudioContext.h"
#include "SquarePlayer.h"
//#include "windowing/WindowingFactory.h"
//#include "dialogs/GUIDialogOK.h"
//#include "guilib/GUIWindowManager.h"
//#include "Application.h"
//#include "filesystem/FileMusicDatabase.h"
#include "FileItem.h"
//#include "utils/RegExp.h"
//#include "utils/StringUtils.h"
//#include "URL.h"
#include "utils/XMLUtils.h"
//#include "utils/TimeUtils.h"
#include "utils/log.h"
#if defined(_WIN32)
  #include "Windows.h"
/*
  #ifdef HAS_IRSERVERSUITE
    #include "input/windows/IRServerSuite.h"
  #endif
*/
#endif
/*
#if defined(HAS_LIRC)
  #include "input/linux/LIRC.h"
#endif
*/

// If the process ends in less than this time (ms), we assume it's a launcher
// and wait for manual intervention before continuing
#define LAUNCHER_PROCESS_TIME 2000
// Time (ms) we give a process we sent a WM_QUIT to close before terminating
#define PROCESS_GRACE_TIME 3000
// Default time after which the item's playcount is incremented
#define DEFAULT_PLAYCOUNT_MIN_TIME 10

//using namespace XFILE;

CSquarePlayer::CSquarePlayer(IPlayerCallback& callback)
    : IPlayer(callback),
      CThread("CSquarePlayer")
{
  m_bAbortRequest = false; // Also found in CDVDPlayer
  m_bIsPlaying = false;
  m_bPaused = false;
  m_playbackStartTime = 0;
  m_speed = 1;
  m_totalTime = 1;
  m_time = 0;

  m_playCountMinTime = DEFAULT_PLAYCOUNT_MIN_TIME;
}

CSquarePlayer::~CSquarePlayer()
{
  CloseFile();
}

bool CSquarePlayer::OpenFile(const CFileItem& file, const CPlayerOptions &options)
{
  try
  {
    /*
    // if playing a file close it first
    // this has to be changed so we won't have to close it.
    if (ThreadHandle())
      CloseFile();
    m_bAbortRequest = false;
    */
    
    m_bIsPlaying = true;
    m_filename = file.m_strPath;
    CLog::Log(LOGNOTICE, "%s: %s", __FUNCTION__, m_filename.c_str());
    
    if (ThreadHandle() == NULL)
      Create();

    m_startEvent.Set();

    return true;
  }
  catch(...)
  {
    m_bIsPlaying = false;
    CLog::Log(LOGERROR,"%s - Exception thrown", __FUNCTION__);
    return false;
  }
}

bool CSquarePlayer::CloseFile()
{
  // set the abort request so that other threads can finish up
  m_bAbortRequest = true;
  return true;
}

bool CSquarePlayer::IsPlaying() const
{
  return m_bIsPlaying; // In CDVDPlayer, this is !m_bStop
}

void CSquarePlayer::Process()
{
  // CExternalPlayer processes m_args and m_filenameReplacers here
  
  CLog::Log(LOGNOTICE, "%s: File: %s", __FUNCTION__, m_filename.c_str());
  CLog::Log(LOGNOTICE, "%s: Start", __FUNCTION__);

  m_playbackStartTime = XbmcThreads::SystemClockMillis();

  if (m_startEvent.WaitMSec(100))
  {
    m_startEvent.Reset();
    do
    {
      if (!m_bPaused)
      {
        /*
        if (!ProcessPAP())
          break;
        */
      }
      else
      {
        Sleep(100);
      }
    }
    while (!m_bAbortRequest);
  }
  
  m_bIsPlaying = false;
  CLog::Log(LOGNOTICE, "%s: Stop", __FUNCTION__);

  //if (!m_bStopPlaying && !m_bStop) // from CPAPlayer
  if (m_bAbortRequest)
    m_callback.OnPlayBackStopped();
  else
    m_callback.OnPlayBackEnded();
}

void CSquarePlayer::Pause()
{
  m_bPaused = !m_bPaused;
}

bool CSquarePlayer::IsPaused() const
{
  return m_bPaused;
}

bool CSquarePlayer::HasVideo() const
{
  return true;
}

bool CSquarePlayer::HasAudio() const
{
  return false;
}

void CSquarePlayer::SwitchToNextLanguage()
{
}

void CSquarePlayer::ToggleSubtitles()
{
}

bool CSquarePlayer::CanSeek()
{
  return false;
}

void CSquarePlayer::Seek(bool bPlus, bool bLargeStep)
{
}

void CSquarePlayer::GetAudioInfo(CStdString& strAudioInfo)
{
  strAudioInfo = "CSquarePlayer:GetAudioInfo";
}

void CSquarePlayer::GetVideoInfo(CStdString& strVideoInfo)
{
  strVideoInfo = "CSquarePlayer:GetVideoInfo";
}

void CSquarePlayer::GetGeneralInfo(CStdString& strGeneralInfo)
{
  strGeneralInfo = "CSquarePlayer:GetGeneralInfo";
}

void CSquarePlayer::SwitchToNextAudioLanguage()
{
}

void CSquarePlayer::SeekPercentage(float iPercent)
{
}

float CSquarePlayer::GetPercentage()
{
  __int64 iTime = GetTime();
  __int64 iTotalTime = GetTotalTime() * 1000;

  if (iTotalTime != 0)
  {
    CLog::Log(LOGDEBUG, "Percentage is %f", (iTime * 100 / (float)iTotalTime));
    return iTime * 100 / (float)iTotalTime;
  }

  return 0.0f;
}

void CSquarePlayer::SetAVDelay(float fValue)
{
}

float CSquarePlayer::GetAVDelay()
{
  return 0.0f;
}

void CSquarePlayer::SetSubTitleDelay(float fValue)
{
}

float CSquarePlayer::GetSubTitleDelay()
{
  return 0.0;
}

void CSquarePlayer::SeekTime(__int64 iTime)
{
}

__int64 CSquarePlayer::GetTime() // in millis
{
  if ((XbmcThreads::SystemClockMillis() - m_playbackStartTime) / 1000 > m_playCountMinTime)
  {
    m_time = m_totalTime * 1000;
  }

  return m_time;
}

int CSquarePlayer::GetTotalTime() // in seconds
{
  return m_totalTime;
}

void CSquarePlayer::ToFFRW(int iSpeed)
{
  m_speed = iSpeed;
}

void CSquarePlayer::ShowOSD(bool bOnoff)
{
}

CStdString CSquarePlayer::GetPlayerState()
{
  return "";
}

bool CSquarePlayer::SetPlayerState(CStdString state)
{
  return true;
}

bool CSquarePlayer::Initialize(TiXmlElement* pConfig)
{
  XMLUtils::GetString(pConfig, "filename", m_filename); 
  if (m_filename.length() > 0)
  {
    CLog::Log(LOGNOTICE, "SquarePlayer Filename: %s", m_filename.c_str());
  }
  else
  {
    CStdString xml;
    xml<<*pConfig;
    CLog::Log(LOGERROR, "SquarePlayer Error: filename element missing from: %s", xml.c_str());
    return false;
  }

  XMLUtils::GetString(pConfig, "color", m_color);

  XMLUtils::GetInt(pConfig, "playcountminimumtime", m_playCountMinTime, 1, INT_MAX);

  CLog::Log(LOGNOTICE, "SquarePlayer Tweaks: color (%s), playcountminimumtime (%d)",
          m_color.c_str(),
          m_playCountMinTime);

  return true;
}
