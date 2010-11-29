#  Copyright (C) 2007-2010  CEA/DEN, EDF R&D
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
lib_LTLIBRARIES = libHexoticEngine.la

dist_libHexoticEngine_la_SOURCES =	\
	HexoticPlugin_Hexotic.hxx		\
	HexoticPlugin_Hexotic.cxx		\
	HexoticPlugin_Hexotic_i.hxx	\
	HexoticPlugin_Hexotic_i.cxx	\
	HexoticPlugin_Hypothesis.hxx	\
	HexoticPlugin_Hypothesis.cxx	\
	HexoticPlugin_Hypothesis_i.hxx	\
	HexoticPlugin_Hypothesis_i.cxx	\
	HexoticPlugin_i.cxx

libHexoticEngine_la_CPPFLAGS =			\
	-I$(top_builddir)/idl			\
	$(SMESH_CXXFLAGS)			\
	$(CORBA_CXXFLAGS)			\
	$(CORBA_INCLUDES)			\
	$(BOOST_CPPFLAGS)			\
	$(CAS_CPPFLAGS)				\
	$(VTK_INCLUDES)	                        \
	$(KERNEL_CXXFLAGS)			\
	$(GUI_CXXFLAGS)				\
	$(MED_CXXFLAGS)				\
	$(GEOM_CXXFLAGS)

libHexoticEngine_la_LDFLAGS  =			\
	../../idl/libSalomeIDLHexoticPLUGIN.la	\
	$(SMESH_LDFLAGS) -lSMESHimpl -lSMESHEngine -lStdMeshersEngine	\
	$(KERNEL_LDFLAGS) -lSalomeGenericObj
