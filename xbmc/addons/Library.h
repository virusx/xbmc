/*
 *      Copyright (C) 2005-2012 Team XBMC
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
#pragma once

#include "Addon.h"
#include "AddonManager.h"
//#include "XBDateTime.h"
//#include "URL.h"
//#include "utils/Job.h"
//#include "threads/CriticalSection.h"
//#include "threads/SingleLock.h"

namespace ADDON
{
  class CLibrary;
  typedef boost::shared_ptr<CLibrary> LibraryPtr;

  class CLibrary : public CAddon
  {
  public:
    AddonPtr Clone(const AddonPtr &self) const;
    CLibrary(const AddonProps& props);
    CLibrary(const cp_extension_t *props);
    virtual ~CLibrary() { /* empty */ }

    const CStdString& GetLabel() const { return m_label; }
    const CStdString& GetDatabaseName() const { return m_dbName; }
    int GetDatabaseVersion() const { return m_dbVersion; }

  private:
    CLibrary(const CLibrary &rhs, const AddonPtr &self);

    CStdString m_label;
    CStdString m_dbName;
    int m_dbVersion;

    CCriticalSection m_critSection;
  };
}
