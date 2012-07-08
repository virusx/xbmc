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

#include "PythonLibrary.h"
#include "pyutil.h"
#include "pythreadstate.h"
//#include "../XBPython.h"
//#include "threads/Atomics.h"
#include "addons/LibraryAddonDatabase.h"
#include "addons/AddonManager.h"
#include "addons/IAddon.h"
#include "utils/log.h"

namespace PYXBMC
{
  // Copied from getDefaultId() in PythonAddon.cpp
  static const char* getDefaultId()
  {
    const char* id = NULL;

    // Get a reference to the main module and global dictionary
    PyObject* main_module = PyImport_AddModule((char*)"__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);

    // Extract a reference to the function "func_name" from the global dictionary
    PyObject* pyid = PyDict_GetItemString(global_dict, "__xbmcaddonid__");
    if (pyid)
      id = PyString_AsString(pyid);
    return id;
  }

  // Copied from getAddonVersion() in PythonAddon.cpp
  static CStdString getAddonVersion()
  {
    // Get a reference to the main module and global dictionary
    PyObject* main_module = PyImport_AddModule((char*)"__main__");
    PyObject* global_dict = PyModule_GetDict(main_module);

    // Extract a reference to the function "func_name" from the global dictionary
    PyObject* pyversion = PyDict_GetItemString(global_dict, "__xbmcapiversion__");
    CStdString version(PyString_AsString(pyversion));
    return version;
  }
}

#ifdef __cplusplus
extern "C" {
#endif

using ADDON::AddonPtr;
using ADDON::CAddonMgr;

namespace PYXBMC
{
  
  PyObject* Library_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
  {
    LibraryAddon *self;

    self = (LibraryAddon*)type->tp_alloc(type, 0);
    if (!self)
      return NULL;

    static const char *keywords[] = { "id", NULL };
    const char *id = NULL;

    // Parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"|s", (char**)keywords, (char**)&id))
    {
      Py_DECREF(self);
      return NULL;
    }

    // If the id wasn't passed then get the id from the global dictionary
    if (!id)
      id = getDefaultId();

    // if we still don't have an id then bail
    if (!id)
    {
        PyErr_SetString(PyExc_Exception, "No valid addon id could be obtained. "
                        "None was passed and the script wasn't executed in a normal xbmc manner.");
        Py_DECREF(self);
        return NULL;
    }

    // If we still fail we MAY be able to recover.
    AddonPtr addon;
    if (!CAddonMgr::Get().GetAddon(id, addon))
    {
      // we need to check the version prior to trying a bw compatibility trick
      ADDON::AddonVersion version(getAddonVersion());
      ADDON::AddonVersion allowable("1.0");

      if (version <= allowable)
      {
        // try the default ...
        id = getDefaultId();

        if (!CAddonMgr::Get().GetAddon(id, addon))
        {
          PyErr_SetString(PyExc_Exception, "Could not get AddonPtr!");
          Py_DECREF(self);
          return NULL;
        }
        else
        {
          CLog::Log(LOGERROR,"Use of deprecated functionality. Please to not assume that \"os.getcwd\" will return the script directory.");
        }
      }
      else
      {
        CStdString errorMessage ("Could not get AddonPtr given a script id of ");
        errorMessage += id;
        errorMessage += ". If you are trying to use 'os.getcwd' to set the path, you cannot do that in a ";
        errorMessage += version.Print();
        errorMessage += " plugin.";
        PyErr_SetString(PyExc_Exception, errorMessage.c_str());
        Py_DECREF(self);
        return NULL;
      }
    }

    self->pAddon = boost::dynamic_pointer_cast<ADDON::CLibrary>(addon);
    return (PyObject*)self;
  }
  
  void Library_Dealloc(LibraryAddon* self)
  {
    self->ob_type->tp_free((PyObject*)self);
  }
  
  PyDoc_STRVAR(execute__doc__,
    "execute(query) -- Execute a query that does not return any result.\n"
    "\n"
    "query          : string - The query to execute.\n"
    "\n"
    "True if the query was executed successfully, false otherwise.\n"
    "\n"
    "example:\n"
    "  - success = self.Library.execute('DROP TABLE bass')\n");

  PyObject* Library_Execute(LibraryAddon *self, PyObject *args)
  {
    char *q_line;
    if (!PyArg_ParseTuple(args, (char*)"s", &q_line))
      return NULL;

    CStdString strQuery(q_line);
    bool bResult = true;

    CPyThreadState pyState;

    CLibraryAddonDatabase db(self->pAddon);
    db.Open();
    bResult = db.ExecuteQuery(strQuery);
    
    return Py_BuildValue((char*)"b", bResult);
  }

  PyMethodDef Library_methods[] = {
    {(char*)"execute", (PyCFunction)Library_Execute, METH_VARARGS, execute__doc__},
    {NULL, NULL, 0, NULL}
  };

  PyDoc_STRVAR(library__doc__,
    "Library class.\n"
    "\n"
    "Library(id) -- Creates a new Library class for the given library addon ID.\n"
    "\n"
    "id          : string - ID of the addon.\n"
    "\n"
    "* Note, You can use the above as a keyword.\n"
    "\n"
    "example:\n"
    " - self.Library = xbmclibrary.Library(id='library.xbmc.pictures')\n");

// Restore code and data sections to normal.

  PyTypeObject Library_Type;

  void initLibrary_Type()
  {
    PyXBMCInitializeTypeObject(&Library_Type);

    Library_Type.tp_name = (char*)"xbmclibrary.Library";
    Library_Type.tp_basicsize = sizeof(LibraryAddon);
    Library_Type.tp_dealloc = (destructor)Library_Dealloc;
    Library_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    Library_Type.tp_doc = library__doc__;
    Library_Type.tp_methods = Library_methods;
    Library_Type.tp_base = 0;
    Library_Type.tp_new = Library_New;
  }
}











/*
struct SPyEvent
{
  SPyEvent(CPythonMonitor* monitor
         , const std::string &function, const std::string &arg = "")
  {
    m_monitor   = monitor;
    m_monitor->Acquire();
    m_function = function;
    m_arg = arg;
  }

  ~SPyEvent()
  {
    m_monitor->Release();
  }

  std::string    m_function;
  std::string    m_arg;
  CPythonMonitor* m_monitor;
};

/*
 * called from python library!
 *
static int SPyEvent_Function(void* e)
{
  SPyEvent* object = (SPyEvent*)e;
  PyObject* ret    = NULL;

  if(object->m_monitor->m_callback)
  { 
    if (!object->m_arg.empty())
      ret = PyObject_CallMethod(object->m_monitor->m_callback, (char*)object->m_function.c_str(), "(s)", object->m_arg.c_str());
    else
      ret = PyObject_CallMethod(object->m_monitor->m_callback, (char*)object->m_function.c_str(), NULL);
  }

  if(ret)
  {
    Py_DECREF(ret);
  }

  CPyThreadState pyState;
  delete object;

  return 0;

}

CPythonMonitor::CPythonMonitor()
{
  m_callback = NULL;
  m_refs     = 1;
  g_pythonParser.RegisterPythonMonitorCallBack(this);
}

void CPythonMonitor::Release()
{
  if(AtomicDecrement(&m_refs) == 0)
    delete this;
}

void CPythonMonitor::Acquire()
{
  AtomicIncrement(&m_refs);
}

CPythonMonitor::~CPythonMonitor(void)
{
  g_pythonParser.UnregisterPythonMonitorCallBack(this);
}

void CPythonMonitor::OnSettingsChanged()
{
  PyXBMC_AddPendingCall(m_state, SPyEvent_Function, new SPyEvent(this, "onSettingsChanged"));
  g_pythonParser.PulseGlobalEvent();
}

void CPythonMonitor::OnScreensaverActivated()
{
  PyXBMC_AddPendingCall(m_state, SPyEvent_Function, new SPyEvent(this, "onScreensaverActivated"));
  g_pythonParser.PulseGlobalEvent();
}

void CPythonMonitor::OnScreensaverDeactivated()
{
  PyXBMC_AddPendingCall(m_state, SPyEvent_Function, new SPyEvent(this, "onScreensaverDeactivated"));
  g_pythonParser.PulseGlobalEvent();
}

void CPythonMonitor::OnDatabaseUpdated(const std::string &database)
{
 PyXBMC_AddPendingCall(m_state, SPyEvent_Function, new SPyEvent(this, "onDatabaseUpdated", database));
 g_pythonParser.PulseGlobalEvent();
}

void CPythonMonitor::SetCallback(PyThreadState *state, PyObject *object)
{
  m_callback = object;
  m_state    = state;
}
/**/

#ifdef __cplusplus
}
#endif
