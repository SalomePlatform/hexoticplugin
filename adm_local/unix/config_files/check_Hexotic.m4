dnl Copyright (C) 2007-2012  CEA/DEN, EDF R&D
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl

dnl  File   : check_Hexotic.m4
dnl  Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
dnl
AC_DEFUN([CHECK_HEXOTIC],[

AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_CHECKING(for Hexotic commercial product)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(,
	    [  --with-hexotic=DIR      root directory path of Hexotic installation])

Hexotic_ok=no

if test "$with_hexotic" == "no" ; then
    AC_MSG_WARN(You have choosen building plugin without Hexotic)
else
    if test "$with_hexotic" == "yes" || test "$with_hexotic" == "auto"; then
        Hexotic_HOME=""
    else
        Hexotic_HOME="$with_hexotic"
    fi
    
    if test "$Hexotic_HOME" == "" ; then
        if test "x$HexoticHOME" != "x" ; then
            Hexotic_HOME=$HexoticHOME
        else
            AC_MSG_WARN(Build plugin without Hexotic)
        fi
    fi
    
    if test "x$Hexotic_HOME" != "x"; then
    
      echo
      echo -------------------------------------------------
      echo You are about to choose to use somehow the
      echo Hexotic commercial product to generate 3D hexahedral mesh.
      echo
    
      AC_CHECKING(for Hexotic executable)
    
      AC_CHECK_PROG(Hexotic,hexotic,found)
    
      if test "x$Hexotic" == x ; then
        AC_MSG_WARN(hexotic program not found in PATH variable)
        AC_MSG_WARN(Build plugin without Hexotic)
      else
        Hexotic_ok=yes
      fi
    
    fi
fi

AC_MSG_RESULT(for Hexotic: $Hexotic_ok)
AC_LANG_RESTORE

])dnl
