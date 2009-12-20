#/**********************************************************\ 
#Original Author: Richard Bateman (taxilian)
#
#Created:    Nov 20, 2009
#License:    Dual license model; choose one of two:
#            Eclipse Public License - Version 1.0
#            http://www.eclipse.org/legal/epl-v10.html
#            - or -
#            GNU Lesser General Public License, version 2.1
#            http://www.gnu.org/licenses/lgpl-2.1.html
#            
#Copyright 2009 PacketPass, Inc and the Firebreath development team
#\**********************************************************/

add_subdirectory(${ACTIVEXPLUGIN_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ActiveXPlugin)
add_subdirectory(${ACTIVEXPLUGINTEST_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/ActiveXPluginTest)

add_subdirectory(${NPAPIPLUGIN_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/NpapiPlugin)
add_subdirectory(${NPAPIPLUGINTEST_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/NpapiPluginTest)
