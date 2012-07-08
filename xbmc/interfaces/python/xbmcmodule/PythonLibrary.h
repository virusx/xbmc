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

#include <Python.h>
#include "addons/Library.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  typedef struct {
    PyObject_HEAD
    ADDON::LibraryPtr pAddon;
  } LibraryAddon;

  extern PyTypeObject Library_Type;
  void initLibrary_Type();
}

/*
int Py_XBMC_Event_OnSettingsChanged(void* arg);
int Py_XBMC_Event_OnScreensaverActivated(void* arg);
int Py_XBMC_Event_OnScreensaverDeactivated(void* arg);
int Py_XBMC_Event_OnDatabaseUpdated(void* arg);

class CPythonLibrary
{
public:
  CPythonLibrary();
  void SetCallback(PyThreadState *state, PyObject *object);
  
  void Acquire();
  void Release();

  PyObject* m_callback;
  PyThreadState *m_state;
  std::string Id;
protected:
  ~CPythonLibrary(void);
  long   m_refs;
};
*/

#ifdef __cplusplus
}
#endif
