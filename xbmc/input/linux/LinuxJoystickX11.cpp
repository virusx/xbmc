/*
*      Copyright (C) 2007-2013 Team XBMC
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
*  Parts of this file are Copyright (C) 2009 Peter Hutterer
*/

#include "system.h"
#if defined(HAVE_X11)

#include "LinuxJoystickX11.h"
#include "utils/log.h"

#include <iostream>
#include <X11/extensions/XInput2.h>
#include <X11/Xlib.h>

// Work-around to forward-declare anonymous struct
struct _Forward_XIAnyClassInfo : public XIAnyClassInfo { };


CLinuxJoystickX11::CLinuxJoystickX11(Display *display) : m_state(), m_display(display)
{
}

CLinuxJoystickX11::~CLinuxJoystickX11()
{
}

/* static */
void CLinuxJoystickX11::Initialize(JoystickArray &joysticks)
{
  // Connect to the X server
  Display *dpy = XOpenDisplay(NULL);

  if (!dpy)
  {
    CLog::Log(LOGERROR, /* __FUNCTION__ */ "CLinuxJoystickX11::Initialize: Failed to open display");
    return;
  }

  // XInput Extension available?
  int opcode, event, error;
  if (!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error))
  {
    CLog::Log(LOGERROR, /* __FUNCTION__ */ "CLinuxJoystickX11::Initialize: X Input extension not available");
    XCloseDisplay(dpy);
    return;
  }

  // Which version of XI2? We support 2.0
  if (!HasXI2(dpy))
  {
    XCloseDisplay(dpy);
    return;
  }

  ListDevices(dpy, XIAllDevices);

  m_display = dpy;
}

/* static */
bool CLinuxJoystickX11::HasXI2(Display *dpy)
{
  int major = 2;
  int minor = 0;
  int rc = XIQueryVersion(dpy, &major, &minor);

  if (rc == BadRequest)
  {
    CLog::Log(LOGERROR, /* __FUNCTION__ */ "CLinuxJoystickX11::Initialize: XI2 not available. Server supports %d.%d", major, minor);
    return false;
  }
  else if (rc != Success)
  {
    CLog::Log(LOGERROR, /* __FUNCTION__ */ "CLinuxJoystickX11::Initialize: Internal Error! This is a bug in Xlib");
    return false;
  }
  return true;
}

/* static */
void CLinuxJoystickX11::ListDevices(Display *dpy, int deviceID)
{
  int numDevices;

  XIDeviceInfo *info = XIQueryDevice(dpy, deviceID, &numDevices);

  for(int i = 0; i < numDevices; i++)
  {
    XIDeviceInfo *dev = &info[i];
    CLog::Log(LOGINFO, /* __FUNCTION__ */ "CLinuxJoystickX11::Initialize: \"%s\" (%d)", dev->name, dev->deviceid);
    CLog::Log(LOGDEBUG, "    Reporting %d classes:", dev->num_classes);
    PrintClasses(dpy, reinterpret_cast<_Forward_XIAnyClassInfo**>(dev->classes), dev->num_classes);
  }
}

/* static */
void CLinuxJoystickX11::PrintClasses(Display *dpy, _Forward_XIAnyClassInfo **_forward_classes, int numClasses)
{
  // Reverse our workaround by upcasting
  XIAnyClassInfo **classes = reinterpret_cast<XIAnyClassInfo**>(_forward_classes);

  for (int i = 0; i < numClasses; i++)
  {
    CLog::Log(LOGDEBUG, "        Class originated from: %d", classes[i]->sourceid);
    switch(classes[i]->type)
    {
    case XIButtonClass:
      {
        XIButtonClassInfo *b = reinterpret_cast<XIButtonClassInfo*>(classes[i]);
        CLog::Log(LOGDEBUG, "        Buttons supported: %d", b->num_buttons);

        std::ostringstream strButtons;
        for (int j = 0; j < b->num_buttons; j++)
          strButtons << " '" << (b->labels[j] ?  XGetAtomName(dpy, b->labels[j]) : "None") << "'";
        CLog::Log(LOGDEBUG, "        Button labels:%s", strButtons.str().c_str());

        std::ostringstream strButtonState;
        for (int j = 0; b->state.mask_len * 8; j++)
          if (XIMaskIsSet(b->state.mask, j))
            strButtonState << " " << j;
        CLog::Log(LOGDEBUG, "        Button state:%s", strButtonState.str().c_str());
      }
      break;

    case XIKeyClass:
      {
        XIKeyClassInfo *k = reinterpret_cast<XIKeyClassInfo*>(classes[i]);
        CLog::Log(LOGDEBUG, "        Keycodes supported: %d", k->num_keycodes);
      }
      break;

    case XIValuatorClass:
      {
        XIValuatorClassInfo *v = reinterpret_cast<XIValuatorClassInfo*>(classes[i]);
        CLog::Log(LOGDEBUG, "        Detail for Valuator %d:", v->number);
        CLog::Log(LOGDEBUG, "            Label:         '%s'", (v->label ? XGetAtomName(dpy, v->label) : "None"));
        CLog::Log(LOGDEBUG, "            Range:         %f - %f", v->min, v->max);
        CLog::Log(LOGDEBUG, "            Resolution:    %d units/m", v->resolution);
        CLog::Log(LOGDEBUG, "            Mode:          %s", (v->mode == XIModeAbsolute ? "absolute" : "relative"));
        if (v->mode == XIModeAbsolute)
          CLog::Log(LOGDEBUG, "            Current value: %f", v->value);
      }
      break;
    default:
      break;
    }
  }
}

/* static */
void CLinuxJoystickX11::DeInitialize(JoystickArray &joysticks)
{
  for (int i = 0; i < (int)joysticks.size(); i++)
  {
    if (boost::dynamic_pointer_cast<CLinuxJoystickX11>(joysticks[i]))
      joysticks.erase(joysticks.begin() + i--);
  }

  XCloseDisplay(m_display);
}

void CLinuxJoystickX11::Update()
{
}

#endif // HAVE_X11
