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

#include "JoystickTypes.h"

/*!
 * \ingroup joysticks
 * \brief Interface for handling events for physical joystick features like
 *        buttons, triggers and analog sticks
 */
class IJoystickInputHandler
{
public:
  virtual ~IJoystickInputHandler(void) { }

  /*!
   * \brief A digital button has been pressed or released
   *
   * \param id        The ID of the button or trigger
   * \param bPressed  True if pressed, false if released
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnButtonPress(JoystickFeatureID id, bool bPressed) { return false; }

  /*!
   * \brief An analog button has changed state
   *
   * This could be a trigger or a pressure-sensitive button.
   *
   * \param id        The ID of the button or trigger
   * \param magnitude The button pressure or trigger travel distance in the
   *                  closed interval [0, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnButtonMotion(JoystickFeatureID id, float magnitude) { return false; }

  /*!
   * \brief An analog stick has moved
   *
   * \param id        The ID of the analog stick
   * \param x         The x coordinate in the closed interval [-1, 1]
   * \param y         The y coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAnalogStickMotion(JoystickFeatureID id, float x, float y) { return false; }

  /*!
   * \brief An accelerometer's acceleration has changed
   *
   * \param x         The x coordinate in the closed interval [-1, 1]
   * \param y         The y coordinate in the closed interval [-1, 1]
   * \param z         The z coordinate in the closed interval [-1, 1]
   *
   * \return True if the event was handled otherwise false
   */
  virtual bool OnAccelerometerMotion(JoystickFeatureID id, float x, float y, float z) { return false; }
};
