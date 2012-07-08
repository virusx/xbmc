# *  This Program is free software; you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 2, or (at your option)
# *  any later version.
# *
# *  This Program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with XBMC; see the file COPYING. If not, write to
# *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
# *  http://www.gnu.org/copyleft/gpl.html

import os
#import sys, urllib2, base64, socket, simplejson
import xbmc, xbmcaddon, xbmclibrary

__addon__      = xbmcaddon.Addon()
__provider__   = __addon__.getAddonInfo('name')
__cwd__        = __addon__.getAddonInfo('path')
__resource__   = xbmc.translatePath(os.path.join(__cwd__, 'resources', 'lib')).decode("utf-8")

library = xbmclibrary.Library()

def createTables():
	# Create tables
	library.execute('CREATE TABLE bass (idHardness integer primary key)')
	# Create views

def updateTables(version):
	if version < 2:
		library.execute('DROP TABLE bass')

if len(sys.argv) >= 2 and sys.argv[1] == 'updateoldversion':
	version = int(sys.argv[2])
	if version == 0:
		createTables()
	else:
		updateTables(version)
