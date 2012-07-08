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

#include <Python.h>

#include "PythonLibrary.h"
#include "pyutil.h"
#include "pythreadstate.h"

//using namespace std;
//using namespace XFILE;
using namespace PYXBMC;


#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#ifdef __cplusplus
extern "C" {
#endif
  
namespace xbmcvfs
{
  /*****************************************************************
    * start of xbmclibrary methods
    *****************************************************************/
  // put module methods here

  // define c functions to be used in python here
  PyMethodDef xbmcLibraryMethods[] = {
    {NULL, NULL, 0, NULL}
  };

  /*****************************************************************
    * end of methods and python objects
    *****************************************************************/

  PyMODINIT_FUNC
  InitLibraryModule()
  {
    Py_INCREF(&Library_Type);

    initLibrary_Type();
    
    if (PyType_Ready(&Library_Type) < 0)
      return;

    // init general xbmc modules
    PyObject* pXbmcLibraryModule;
    pXbmcLibraryModule = Py_InitModule((char*)"xbmclibrary", xbmcLibraryMethods);
    if (pXbmcLibraryModule == NULL)
      return;

    PyModule_AddObject(pXbmcLibraryModule, (char*)"Library", (PyObject*)&Library_Type);

    // constants
    PyModule_AddStringConstant(pXbmcLibraryModule, (char*)"__author__", (char*)PY_XBMC_AUTHOR);
    PyModule_AddStringConstant(pXbmcLibraryModule, (char*)"__date__", (char*)"8 April 2011");
    PyModule_AddStringConstant(pXbmcLibraryModule, (char*)"__version__", (char*)"1.0");
    PyModule_AddStringConstant(pXbmcLibraryModule, (char*)"__credits__", (char*)PY_XBMC_CREDITS);
    PyModule_AddStringConstant(pXbmcLibraryModule, (char*)"__platform__", (char*)PY_XBMC_PLATFORM);
  }

  PyMODINIT_FUNC
  DeinitLibraryModule()
  {
    // no need to Py_DECREF our objects (see InitLibraryModule()) as they were created only
    // so that they could be added to the module, which steals a reference.
  }

}
  
#ifdef __cplusplus
}
#endif
