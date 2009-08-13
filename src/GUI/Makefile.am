#  Copyright (C) 2007-2008  CEA/DEN, EDF R&D
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# ---
# File   : Makefile.am
# Author : Vadim SANDLER, Open CASCADE S.A.S (vadim.sandler@opencascade.com)
# ---
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS =

# Libraries targets
lib_LTLIBRARIES = libHexoticPluginGUI.la

dist_libHexoticPluginGUI_la_SOURCES =		\
	HexoticPluginGUI.cxx			\
	HexoticPluginGUI_HypothesisCreator.h	\
	HexoticPluginGUI_HypothesisCreator.cxx

MOC_FILES = \
	HexoticPluginGUI_HypothesisCreator_moc.cxx

nodist_libHexoticPluginGUI_la_SOURCES = \
	$(MOC_FILES)

# additionnal information to compil and link file

libHexoticPluginGUI_la_CPPFLAGS = 	\
	$(QT_INCLUDES)			\
	$(CAS_CPPFLAGS)			\
	$(PYTHON_INCLUDES)		\
	$(KERNEL_CXXFLAGS)		\
	$(GUI_CXXFLAGS)			\
	$(GEOM_CXXFLAGS)		\
	$(MED_CXXFLAGS)			\
	$(SMESH_CXXFLAGS)		\
	$(BOOST_CPPFLAGS)		\
	$(CORBA_CXXFLAGS)		\
	$(CORBA_INCLUDES)		\
	-I$(srcdir)/../HexoticPlugin	\
	-I$(top_builddir)/idl

libHexoticPluginGUI_la_LDFLAGS =		\
	../HexoticPlugin/libHexoticEngine.la	\
	${SMESH_LDFLAGS} -lSMESH		\
	$(CAS_KERNEL)

# resources files
nodist_salomeres_DATA =			\
	HexoticPLUGIN_images.qm	\
	HexoticPLUGIN_msg_en.qm
