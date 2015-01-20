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

#include "GenericJoystickMultiInputHandler.h"

CGenericJoystickMultiInputHandler::CGenericJoystickMultiInputHandler(IJoystickActionHandler *handler)
 : m_handler(handler)
{
}

CGenericJoystickMultiInputHandler::~CGenericJoystickMultiInputHandler(void)
{
  for (std::map<JoystickActionID, AnalogStick>::const_iterator it = m_analogSticks.begin(); it != m_analogSticks.end(); ++it)
    m_handler->OnAnalogStickMotion(it->first, it->second.x, it->second.y);

  for (std::map<JoystickActionID, Accelerometer>::const_iterator it = m_accelerometers.begin(); it != m_accelerometers.end(); ++it)
    m_handler->OnAccelerometerMotion(it->first, it->second.x, it->second.y, it->second.z);
}

bool CGenericJoystickMultiInputHandler::OnAnalogStickMotion(JoystickActionID id, float x, float y)
{
  m_analogSticks[id].x = x;
  m_analogSticks[id].y = y;
  return true;
}

bool CGenericJoystickMultiInputHandler::OnAccelerometerMotion(JoystickActionID id, float x, float y, float z)
{
  m_accelerometers[id].x = x;
  m_accelerometers[id].y = y;
  m_accelerometers[id].z = z;
  return true;
}
