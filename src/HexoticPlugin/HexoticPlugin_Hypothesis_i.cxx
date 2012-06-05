// Copyright (C) 2007-2012  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// ---
// File   : HexoticPlugin_Hypothesis_i.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hypothesis_i.hxx"
#include "SMESH_Mesh_i.hxx"
#include "SMESH_Gen.hxx"
#include "SMESH_PythonDump.hxx"

#include "Utils_CorbaException.hxx"
#include "utilities.h"

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::HexoticPlugin_Hypothesis_i
 *
 *  Constructor
 */
//=============================================================================
HexoticPlugin_Hypothesis_i::
HexoticPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                           int                     theStudyId,
                           ::SMESH_Gen*            theGenImpl)
  : SALOME::GenericObj_i( thePOA ), 
    SMESH_Hypothesis_i( thePOA )
{
  MESSAGE( "HexoticPlugin_Hypothesis_i::HexoticPlugin_Hypothesis_i" );
  myBaseImpl = new ::HexoticPlugin_Hypothesis (theGenImpl->GetANewId(),
                                              theStudyId,
                                              theGenImpl);
}

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::~HexoticPlugin_Hypothesis_i
 *
 *  Destructor
 */
//=============================================================================
HexoticPlugin_Hypothesis_i::~HexoticPlugin_Hypothesis_i()
{
  MESSAGE( "HexoticPlugin_Hypothesis_i::~HexoticPlugin_Hypothesis_i" );
}

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::SetHexesMinLevel
 *  HexoticPlugin_Hypothesis_i::SetHexesMaxLevel
 *  HexoticPlugin_Hypothesis_i::SetHexoticQuadrangles
 *  HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges
 *  HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements
 *  HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold
 *  HexoticPlugin_Hypothesis_i::SetHexoticNbProc 
 *  HexoticPlugin_Hypothesis_i::SetHexoticWorkingDirectory 
 */
//=============================================================================

void HexoticPlugin_Hypothesis_i::SetHexesMinLevel (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMinLevel");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexesMinLevel(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexesMinLevel( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexesMaxLevel (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMaxLevel");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexesMaxLevel(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexesMaxLevel( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticQuadrangles (CORBA::Boolean theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticQuadrangles");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticQuadrangles(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexoticQuadrangles( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges (CORBA::Boolean theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticIgnoreRidges(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexoticIgnoreRidges( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements (CORBA::Boolean theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticInvalidElements(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexoticInvalidElements( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticSharpAngleThreshold(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexoticSharpAngleThreshold( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticNbProc (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticNbProc");
  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticNbProc(theValue);
  SMESH::TPythonDump() << _this() << ".SetHexoticNbProc( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticWorkingDirectory(const char* path) throw ( SALOME::SALOME_Exception )
{
  if (!path )
    THROW_SALOME_CORBA_EXCEPTION( "Null working directory",SALOME::BAD_PARAM );

  string file(path);
  const char lastChar = *file.rbegin();
#ifdef WIN32
  if ( lastChar != '\\' ) file += '\\';
#else
  if ( lastChar != '/' ) file += '/';
#endif
  file += "Hexotic_In.mesh";
  SMESH_Mesh_i::PrepareForWriting (file.c_str());

  ASSERT(myBaseImpl);
  this->GetImpl()->SetHexoticWorkingDirectory(path);
  SMESH::TPythonDump() << _this() << ".SetHexoticWorkingDirectory( '" << path << "' )";
}

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::GetHexesMinLevel
 *  HexoticPlugin_Hypothesis_i::GetHexesMaxLevel
 *  HexoticPlugin_Hypothesis_i::GetHexoticQuadrangles
 *  HexoticPlugin_Hypothesis_i::GetHexoticIgnoreRidges
 *  HexoticPlugin_Hypothesis_i::GetHexoticInvalidElements
 *  HexoticPlugin_Hypothesis_i::GetHexoticSharpAngleThreshold 
 *  HexoticPlugin_Hypothesis_i::GetHexoticNbProc 
 *  HexoticPlugin_Hypothesis_i::GetHexoticWorkingDirectory 
 */
//=============================================================================

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexesMinLevel()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexesMinLevel");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexesMinLevel();
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexesMaxLevel()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexesMaxLevel");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexesMaxLevel();
}

CORBA::Boolean HexoticPlugin_Hypothesis_i::GetHexoticQuadrangles()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticQuadrangles");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticQuadrangles();
}

CORBA::Boolean HexoticPlugin_Hypothesis_i::GetHexoticIgnoreRidges()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticIgnoreRidges");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticIgnoreRidges();
}

CORBA::Boolean HexoticPlugin_Hypothesis_i::GetHexoticInvalidElements()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticInvalidElements");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticInvalidElements();
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexoticSharpAngleThreshold()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticSharpAngleThreshold");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticSharpAngleThreshold();
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexoticNbProc()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticNbProc");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticNbProc();
}

char* HexoticPlugin_Hypothesis_i::GetHexoticWorkingDirectory()
{
  ASSERT(myBaseImpl);
  return CORBA::string_dup( this->GetImpl()->GetHexoticWorkingDirectory().c_str() );
}

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::GetImpl
 *
 *  Get implementation
 */
//=============================================================================
::HexoticPlugin_Hypothesis* HexoticPlugin_Hypothesis_i::GetImpl()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetImpl");
  return (::HexoticPlugin_Hypothesis*)myBaseImpl;
}

//================================================================================
/*!
 * \brief Verify whether hypothesis supports given entity type 
  * \param type - dimension (see SMESH::Dimension enumeration)
  * \retval CORBA::Boolean - TRUE if dimension is supported, FALSE otherwise
 * 
 * Verify whether hypothesis supports given entity type (see SMESH::Dimension enumeration)
 */
//================================================================================  
CORBA::Boolean HexoticPlugin_Hypothesis_i::IsDimSupported( SMESH::Dimension type )
{
  return type == SMESH::DIM_3D;
}
