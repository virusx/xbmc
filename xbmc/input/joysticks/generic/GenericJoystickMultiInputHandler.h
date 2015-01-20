/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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

#include "input/joysticks/IJoystickActionHandler.h"

#include <map>

/*!
 * \brief Allows multiple actions to be emitted for IDs with multiple axes
 *
 * Say an accelerometer uses three axes: x, y and z. Instead of measuring all
 * three axes and calling OnAccelerometerMotion() with the results, the
 * accelerometer can call OnAccelerometerMotion() once per axis as each axis is
 * updated.
 *
 * This class stores the values as they arrive, and upon deconstruction forwards
 * the final set of axes to the callback exactly once.
 */
class CGenericJoystickMultiInputHandler : public IJoystickActionHandler
{
public:
  CGenericJoystickMultiInputHandler(IJoystickActionHandler *handler);

  virtual ~CGenericJoystickMultiInputHandler(void);

  // implementation of IJoystickActionHandler
  virtual bool OnAnalogStickMotion(JoystickActionID id, float x, float y);
  virtual bool OnAccelerometerMotion(JoystickActionID id, float x, float y, float z);

private:
  struct AnalogStick
  {
    float x;
    float y;
  };

  struct Accelerometer
  {
    float x;
    float y;
    float z;
  };

  IJoystickActionHandler*                   m_handler;
  std::map<JoystickActionID, AnalogStick>   m_analogSticks;
  std::map<JoystickActionID, Accelerometer> m_accelerometers;
};
