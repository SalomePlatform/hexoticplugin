dnl  Copyright (C) 2007-2008  CEA/DEN, EDF R&D
dnl
dnl  This library is free software; you can redistribute it and/or
dnl  modify it under the terms of the GNU Lesser General Public
dnl  License as published by the Free Software Foundation; either
dnl  version 2.1 of the License.
dnl
dnl  This library is distributed in the hope that it will be useful,
dnl  but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl  Lesser General Public License for more details.
dnl
dnl  You should have received a copy of the GNU Lesser General Public
dnl  License along with this library; if not, write to the Free Software
dnl  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl
AC_DEFUN([CHECK_HEXOTIC],[

AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_CHECKING(for Hexotic comercial product)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(,
	    [  --with-Hexotic=DIR root directory path of Hexotic installation],
	    Hexotic_HOME=$withval,Hexotic_HOME="")

Hexotic_ok=no

if test "x$Hexotic_HOME" == "x" ; then

# no --with-Hexotic option used
   if test "x$HexoticHOME" != "x" ; then

    # HexoticHOME environment variable defined
      Hexotic_HOME=$HexoticHOME

   fi
# 
fi

if test "x$Hexotic_HOME" != "x"; then

  echo
  echo -------------------------------------------------
  echo You are about to choose to use somehow the
  echo Hexotic commercial product to generate 3D hexahedral mesh.
  echo

  AC_MSG_CHECKING(for Hexotic executable)

  AC_CHECK_PROG(HEXOTIC, hexotic,found)

  if test "x$HEXOTIC" == x ; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN(Hexotic program not found in PATH variable)
  else
    Hexotic_ok=yes
  fi

fi

AC_MSG_RESULT(for Hexotic: $Hexotic_ok)
AC_LANG_RESTORE

])dnl
