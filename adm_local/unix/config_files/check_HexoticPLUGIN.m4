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
