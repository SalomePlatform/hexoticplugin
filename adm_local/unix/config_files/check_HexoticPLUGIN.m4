# Copyright (C) 2006-2008 OPEN CASCADE, CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# ---
#
# File   : check_HexoticPLUGIN.m4
# Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
#
# ---

AC_DEFUN([CHECK_HEXOTICPLUGIN],[

AC_CHECKING(for Hexotic mesh plugin)

Hexoticplugin_ok=no

HexoticPLUGIN_LDFLAGS=""
HexoticPLUGIN_CXXFLAGS=""

AC_ARG_WITH(Hexoticplugin,
	    [  --with-Hexoticplugin=DIR root directory path of Hexotic mesh plugin installation ])

if test "$with_Hexoticplugin" != "no" ; then
    if test "$with_Hexoticplugin" == "yes" || test "$with_Hexoticplugin" == "auto"; then
	if test "x$HexoticPLUGIN_ROOT_DIR" != "x" ; then
            HexoticPLUGIN_DIR=$HexoticPLUGIN_ROOT_DIR
        fi
    else
        HexoticPLUGIN_DIR="$with_Hexoticplugin"
    fi

    if test "x$HexoticPLUGIN_DIR" != "x" ; then
	if test -f ${HexoticPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome/libHexoticEngine.so ; then
	    Hexoticplugin_ok=yes
	    AC_MSG_RESULT(Using Hexotic mesh plugin distribution in ${HexoticPLUGIN_DIR})
	    HexoticPLUGIN_ROOT_DIR=${HexoticPLUGIN_DIR}
	    HexoticPLUGIN_LDFLAGS=-L${HexoticPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
	    HexoticPLUGIN_CXXFLAGS=-I${HexoticPLUGIN_DIR}/include/salome
	else
	    AC_MSG_WARN("Cannot find compiled Hexotic mesh plugin distribution")
	fi
    else
	AC_MSG_WARN("Cannot find compiled Hexotic mesh plugin distribution")
    fi
fi

AC_MSG_RESULT(for Hexotic mesh plugin: $Hexoticplugin_ok)

AC_SUBST(HexoticPLUGIN_ROOT_DIR)
AC_SUBST(HexoticPLUGIN_LDFLAGS)
AC_SUBST(HexoticPLUGIN_CXXFLAGS)
 
])dnl
