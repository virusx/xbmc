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

#include "ContentAddonFile.h"
#include "addons/ContentAddon.h"
#include "Util.h"
#include "cores/dvdplayer/DVDInputStreams/DVDInputStream.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "URL.h"

using namespace std;
using namespace XFILE;
using namespace ADDON;

CContentAddonFile::CContentAddonFile()
{
}

CContentAddonFile::~CContentAddonFile()
{
}

bool CContentAddonFile::Open(const CURL& url)
{
  CLog::Log(LOGDEBUG, "TODO: %s(%s)", __FUNCTION__, url.Get().c_str());
//  Close();

  //TODO
  return false;
}

void CContentAddonFile::Close()
{
  CLog::Log(LOGDEBUG, "TODO: %s()", __FUNCTION__);
  //TODO
}

unsigned int CContentAddonFile::Read(void* buffer, int64_t size)
{
  CLog::Log(LOGDEBUG, "%s - TODO", __FUNCTION__);
  //TODO
  return 0;
}

int64_t CContentAddonFile::GetLength()
{
  CLog::Log(LOGDEBUG, "%s - TODO", __FUNCTION__);
  //TODO
  return 0;
}

int64_t CContentAddonFile::Seek(int64_t pos, int whence)
{
  CLog::Log(LOGDEBUG, "%s - TODO", __FUNCTION__);
  //TODO
  return 0;
}

int64_t CContentAddonFile::GetPosition()
{
  CLog::Log(LOGDEBUG, "%s - TODO", __FUNCTION__);
  //TODO
  return 0;
}
