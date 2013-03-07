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
 */

#pragma once

#include "input/IJoystick.h"

struct _XDisplay;
typedef _XDisplay Display;

// Work-around to forward-declare anonymous struct
struct _Forward_XIAnyClassInfo;

class CLinuxJoystickX11 : public IJoystick
{
public:
  static void Initialize(JoystickArray &joysticks);
  static void DeInitialize(JoystickArray &joysticks);

  virtual ~CLinuxJoystickX11();
  virtual void Update();
  virtual const SJoystick &GetState() const { return m_state; }

private:
  CLinuxJoystickX11(Display *display);
  
  static bool HasXI2(Display *dpy);
  static void ListDevices(Display *dpy, int deviceID);
  static void PrintClasses(Display *dpy, _Forward_XIAnyClassInfo **_forward_classes, int numClasses);

  static Display *m_display;

  SJoystick m_state;
};
