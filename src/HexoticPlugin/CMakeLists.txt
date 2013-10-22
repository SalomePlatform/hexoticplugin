# Copyright (C) 2012-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${SMESH_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${BLSURFPLUGIN_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(  
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKernel}
  ${CAS_TKMath}
  ${CAS_TKG3d}
  ${CAS_TKTopAlgo}
  ${CAS_TKBRep}
  ${CAS_SALOMELocalTrace}
  ${SMESH_SMESHimpl}
  ${SMESH_SMESHEngine}
  ${SMESH_StdMeshersEngine}
  ${SMESH_SMDS}
  ${SMESH_SMESHDS}
  ${SMESH_SalomeIDLSMESH}
  ${SMESH_SMESHUtils}
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SALOMELocalTrace}
  SalomeIDLHexoticPLUGIN
)

# --- headers ---

# header files
SET(NETGENEngine_HEADERS
  NETGENPlugin_NETGEN_3D.hxx 
  NETGENPlugin_NETGEN_3D_i.hxx 
  NETGENPlugin_NETGEN_2D.hxx 
  NETGENPlugin_NETGEN_2D_i.hxx 
  NETGENPlugin_NETGEN_2D3D.hxx 
  NETGENPlugin_NETGEN_2D3D_i.hxx 
  NETGENPlugin_NETGEN_2D_ONLY.hxx 
  NETGENPlugin_NETGEN_2D_ONLY_i.hxx 
  NETGENPlugin_Hypothesis.hxx 
  NETGENPlugin_Hypothesis_i.hxx 
  NETGENPlugin_Hypothesis_2D.hxx 
  NETGENPlugin_Hypothesis_2D_i.hxx 
  NETGENPlugin_Hypothesis_3D_i.hxx 
  NETGENPlugin_Hypothesis_2D_ONLY_i.hxx 
  NETGENPlugin_SimpleHypothesis_2D.hxx 
  NETGENPlugin_SimpleHypothesis_3D.hxx 
  NETGENPlugin_SimpleHypothesis_2D_i.hxx 
  NETGENPlugin_SimpleHypothesis_3D_i.hxx 
  NETGENPlugin_Mesher.hxx 
  NETGENPlugin_Defs.hxx
)

# --- sources ---

# sources / static
SET(HexoticEngine_SOURCES
  HexoticPlugin_Hexotic.hxx
  HexoticPlugin_Hexotic.cxx
  HexoticPlugin_Hexotic_i.hxx
  HexoticPlugin_Hexotic_i.cxx
  HexoticPlugin_Hypothesis.hxx
  HexoticPlugin_Hypothesis.cxx
  HexoticPlugin_Hypothesis_i.hxx
  HexoticPlugin_Hypothesis_i.cxx
  HexoticPlugin_i.cxx
  HexoticPlugin_Defs.hxx
)

# --- scripts ---

# scripts / static
SET(_python_SCRIPTS
  __init__.py
  HexoticPLUGINBuilder.py
)

# --- rules ---

ADD_LIBRARY(HexoticEngine ${HexoticEngine_SOURCES})
TARGET_LINK_LIBRARIES(HexoticEngine ${_link_LIBRARIES} )
INSTALL(TARGETS HexoticEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

SALOME_INSTALL_SCRIPTS("${_python_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/HexoticPLUGIN)
