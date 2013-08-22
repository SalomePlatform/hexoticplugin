dnl Copyright (C) 2007-2013  CEA/DEN, EDF R&D
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

dnl  File   : check_GHS3DPLUGIN.m4
dnl  Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
dnl
AC_DEFUN([CHECK_GHS3DPLUGIN],[

AC_CHECKING(for GHS3D mesh plugin)

GHS3Dplugin_ok=no

GHS3DPLUGIN_LDFLAGS=""
GHS3DPLUGIN_CXXFLAGS=""

AC_ARG_WITH(GHS3Dplugin,
      --with-GHS3Dplugin=DIR  root directory path of GHS3D mesh plugin installation,
      GHS3DPLUGIN_DIR="$withval",GHS3DPLUGIN_DIR="")

if test "x$GHS3DPLUGIN_DIR" = "x" ; then

# no --with-GHS3Dplugin option used

  if test "x$GHS3DPLUGIN_ROOT_DIR" != "x" ; then

    # SALOME_ROOT_DIR environment variable defined
    GHS3DPLUGIN_DIR=$GHS3DPLUGIN_ROOT_DIR

  else

    # search Salome binaries in PATH variable
    AC_PATH_PROG(TEMP, libGHS3DEngine.so)
    if test "x$TEMP" != "x" ; then
      GHS3DPLUGIN_DIR=`dirname $TEMP`
    fi

  fi

fi

if test -f ${GHS3DPLUGIN_DIR}/lib${LIB_LOCATION_SUFFIX}/salome/libGHS3DEngine.so  ; then
  GHS3Dplugin_ok=yes
  AC_MSG_RESULT(Using GHS3D mesh plugin distribution in ${GHS3DPLUGIN_DIR})

  if test "x$GHS3DPLUGIN_ROOT_DIR" == "x" ; then
    GHS3DPLUGIN_ROOT_DIR=${GHS3DPLUGIN_DIR}
  fi
  GHS3DPLUGIN_CXXFLAGS=-I${GHS3DPLUGIN_ROOT_DIR}/include/salome
  GHS3DPLUGIN_LDFLAGS="-L${GHS3DPLUGIN_ROOT_DIR}/lib${LIB_LOCATION_SUFFIX}/salome -lGHS3DEngine"
else
  AC_MSG_WARN("Cannot find compiled GHS3D mesh plugin distribution")
fi
  
AC_MSG_RESULT(for GHS3D mesh plugin: $GHS3Dplugin_ok)

AC_SUBST(GHS3DPLUGIN_ROOT_DIR)
AC_SUBST(GHS3DPLUGIN_LDFLAGS)
AC_SUBST(GHS3DPLUGIN_CXXFLAGS)
 
])dnl
