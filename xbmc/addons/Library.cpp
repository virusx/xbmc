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

#include "Library.h"
//#include "tinyXML/tinyxml.h"
//#include "filesystem/File.h"
//#include "AddonDatabase.h"
//#include "settings/Settings.h"
//#include "FileItem.h"
//#include "utils/JobManager.h"
//#include "addons/AddonInstaller.h"
//#include "utils/log.h"
//#include "utils/URIUtils.h"
//#include "dialogs/GUIDialogYesNo.h"
//#include "dialogs/GUIDialogKaiToast.h"

//using namespace XFILE;
using namespace ADDON;

AddonPtr CLibrary::Clone(const AddonPtr &self) const
{
  CLibrary *result = new CLibrary(*this, self);
  result->m_label = m_label;
  result->m_dbName = m_dbName;
  result->m_dbVersion = m_dbVersion;
  return AddonPtr(result);
}

CLibrary::CLibrary(const AddonProps &props) : CAddon(props), m_dbVersion(0)
{
}

CLibrary::CLibrary(const cp_extension_t *ext) : CAddon(ext), m_dbVersion(0)
{
  // Read in the properties we need
  if (ext)
  {
    m_label = CAddonMgr::Get().GetExtValue(ext->configuration, "library@label");
    m_dbName = CAddonMgr::Get().GetExtValue(ext->configuration, "database@name");
    // CptSpiff: strtol(foo.c_str()) is the best way to convert a CStdString to an int
    m_dbVersion = strtol(CAddonMgr::Get().GetExtValue(ext->configuration, "database@version").c_str(), NULL, 10);
    if (m_dbVersion < 1)
      m_dbVersion = 1;
  }
}

/*!
 \access private
 */
CLibrary::CLibrary(const CLibrary &rhs, const AddonPtr &self) : CAddon(rhs, self)
{
}
