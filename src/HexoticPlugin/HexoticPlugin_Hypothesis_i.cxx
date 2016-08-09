// Copyright (C) 2007-2016  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
 *  HexoticPlugin_Hypothesis_i::SetMinSize
 *  HexoticPlugin_Hypothesis_i::SetMaxSize
 *  HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges
 *  HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements
 *  HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold
 *  HexoticPlugin_Hypothesis_i::SetHexoticNbProc 
 *  HexoticPlugin_Hypothesis_i::SetHexoticWorkingDirectory 
 *  HexoticPlugin_Hypothesis_i::SetHexoticSdMode
 *  HexoticPlugin_Hypothesis_i::SetVerbosity
 *  HexoticPlugin_Hypothesis_i::SetHexoticMaxMemory
 *  HexoticPlugin_Hypothesis_i::SetTextOptions
 *  HexoticPlugin_Hypothesis_i::SetNbLayers
 *  HexoticPlugin_Hypothesis_i::SetFirstLayerSize
 *  HexoticPlugin_Hypothesis_i::SetDirection
 *  HexoticPlugin_Hypothesis_i::SetGrowth
 *  HexoticPlugin_Hypothesis_i::SetFacesWithLayers
 *  HexoticPlugin_Hypothesis_i::SetImprintedFaces
 */
//=============================================================================

void HexoticPlugin_Hypothesis_i::SetHexesMinLevel (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMinLevel");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexesMinLevel();
  this->GetImpl()->SetHexesMinLevel(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexesMinLevel( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexesMaxLevel (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMaxLevel");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexesMaxLevel();
  this->GetImpl()->SetHexesMaxLevel(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexesMaxLevel( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetMinSize (CORBA::Double theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMaxLevel");
  ASSERT(myBaseImpl);
  CORBA::Double oldValue = GetMinSize();
  this->GetImpl()->SetMinSize(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetMinSize( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetMaxSize (CORBA::Double theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexesMaxLevel");
  ASSERT(myBaseImpl);
  CORBA::Double oldValue = GetMaxSize();
  this->GetImpl()->SetMaxSize(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetMaxSize( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges (CORBA::Boolean theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticIgnoreRidges");
  ASSERT(myBaseImpl);
  CORBA::Boolean oldValue = GetHexoticIgnoreRidges();
  this->GetImpl()->SetHexoticIgnoreRidges(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticIgnoreRidges( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements (CORBA::Boolean theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticInvalidElements");
  ASSERT(myBaseImpl);
  CORBA::Boolean oldValue = GetHexoticInvalidElements();
  this->GetImpl()->SetHexoticInvalidElements(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticInvalidElements( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold (CORBA::Double theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticSharpAngleThreshold");
  ASSERT(myBaseImpl);
  CORBA::Double oldValue = GetHexoticSharpAngleThreshold();
  this->GetImpl()->SetHexoticSharpAngleThreshold(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticSharpAngleThreshold( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticNbProc (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticNbProc");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexoticNbProc();
  this->GetImpl()->SetHexoticNbProc(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticNbProc( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticWorkingDirectory(const char* path) throw ( SALOME::SALOME_Exception )
{
  if (!path )
    THROW_SALOME_CORBA_EXCEPTION( "Null working directory",SALOME::BAD_PARAM );

  ASSERT(myBaseImpl);
  std::string file(path);
  std::string oldValue(GetHexoticWorkingDirectory());
  bool doDump = false;
  if (oldValue != file)
    doDump = true;

  const char lastChar = *file.rbegin();
#ifdef WIN32
  if ( lastChar != '\\' ) file += '\\';
#else
  if ( lastChar != '/' ) file += '/';
#endif
  file += "Hexotic_In.mesh";
  SMESH_Mesh_i::PrepareForWriting (file.c_str());

  this->GetImpl()->SetHexoticWorkingDirectory(path);
  if (doDump)
    SMESH::TPythonDump() << _this() << ".SetHexoticWorkingDirectory( '" << path << "' )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticSdMode (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticSdMode");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexoticSdMode();
  this->GetImpl()->SetHexoticSdMode(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticSdMode( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticVerbosity (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetVerbosity");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexoticVerbosity();
  this->GetImpl()->SetHexoticVerbosity(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticVerbosity( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetHexoticMaxMemory (CORBA::Long theValue)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetHexoticMaxMemory");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetHexoticMaxMemory();
  this->GetImpl()->SetHexoticMaxMemory(theValue);
  if (theValue != oldValue)
    SMESH::TPythonDump() << _this() << ".SetHexoticMaxMemory( " << theValue << " )";
}

void HexoticPlugin_Hypothesis_i::SetTextOptions(const char* theOptions)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetTextOptions");
  ASSERT(myBaseImpl);
  std::string oldValue(GetAdvancedOption());
  this->GetImpl()->SetAdvancedOption(theOptions);
  if (theOptions != oldValue)
    SMESH::TPythonDump() << _this() << ".SetAdvancedOption( '" << theOptions << "' )";
}

void HexoticPlugin_Hypothesis_i::SetAdvancedOption(const char* theOptions)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetAdvancedOption");
  ASSERT(myBaseImpl);
  std::string oldValue(GetAdvancedOption());
  this->GetImpl()->SetAdvancedOption(theOptions);
  if (theOptions != oldValue)
    SMESH::TPythonDump() << _this() << ".SetAdvancedOption( '" << theOptions << "' )";
}

HexoticPlugin::HexoticPluginSizeMapsList* HexoticPlugin_Hypothesis_i::GetSizeMaps ()
{
  // Get the std::map < std::string entry, double size >
  HexoticPlugin::HexoticPluginSizeMapsList_var result = new HexoticPlugin::HexoticPluginSizeMapsList();
  const ::HexoticPlugin_Hypothesis::THexoticSizeMaps sizeMaps = this->GetImpl()->GetSizeMaps();
  result->length( sizeMaps.size() );
  
  // Write the content into a CORBA sequence of struct{ entry=anEntry; size=aSize; }
  ::HexoticPlugin_Hypothesis::THexoticSizeMaps::const_iterator it = sizeMaps.begin(); 
  for ( int i = 0; it != sizeMaps.end(); i++, it++ )
  {
    HexoticPlugin::HexoticPluginSizeMap_var aSizeMap = new HexoticPlugin::HexoticPluginSizeMap();
    aSizeMap->entry = CORBA::string_dup( it->first.c_str() );
    aSizeMap->size = it->second;
    result[i] = aSizeMap;
  }
  return result._retn();
}

void HexoticPlugin_Hypothesis_i::SetSizeMapEntry ( const char* theEntry, CORBA::Double theSize )
{
//   MESSAGE("HexoticPlugin_Hypothesis_i::SetSizeMapEntry");
  bool valueChanged = this->GetImpl()->AddSizeMap(theEntry, theSize);
  if (valueChanged)
    SMESH::TPythonDump() << _this() << ".SetSizeMap( "<< theEntry << ", " << theSize << " )";
}

void HexoticPlugin_Hypothesis_i::UnsetSizeMapEntry ( const char* theEntry )
{
//   MESSAGE("HexoticPlugin_Hypothesis_i::UnsetSizeMapEntry");
  bool entryRemoved = this->GetImpl()->UnsetSizeMap(theEntry);
  if (entryRemoved)
    SMESH::TPythonDump() << _this() << ".UnsetSizeMap( "<< theEntry << " )";
}

void HexoticPlugin_Hypothesis_i::SetSizeMap (const GEOM::GEOM_Object_ptr theGeomObj, const double theSize)
{
//   MESSAGE("HexoticPlugin_Hypothesis_i::SetSizeMap");
  ASSERT(myBaseImpl);
  std::string entry = theGeomObj->GetStudyEntry();
  SetSizeMapEntry( entry.c_str(), theSize);
}

void HexoticPlugin_Hypothesis_i::UnsetSizeMap (const GEOM::GEOM_Object_ptr theGeomObj)
{
//   MESSAGE("HexoticPlugin_Hypothesis_i::UnsetSizeMap");
  ASSERT(myBaseImpl);
  std::string entry = theGeomObj->GetStudyEntry();
  UnsetSizeMapEntry( entry.c_str());
}

void HexoticPlugin_Hypothesis_i::SetNbLayers(CORBA::Long theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetNbLayers");
  ASSERT(myBaseImpl);
  CORBA::Long oldValue = GetNbLayers();
  this->GetImpl()->SetNbLayers(theVal);
  if (theVal != oldValue)
    SMESH::TPythonDump() << _this() << ".SetNbLayers( " << theVal << " )";
}

void HexoticPlugin_Hypothesis_i::SetFirstLayerSize(CORBA::Double theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetFirstLayerSize");
  ASSERT(myBaseImpl);
  CORBA::Double oldValue = GetFirstLayerSize();
  this->GetImpl()->SetFirstLayerSize(theVal);
  if (theVal != oldValue)
    SMESH::TPythonDump() << _this() << ".SetFirstLayerSize( " << theVal << " )";
}

void HexoticPlugin_Hypothesis_i::SetDirection(CORBA::Boolean theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetDirection");
  ASSERT(myBaseImpl);
  CORBA::Boolean oldValue = GetDirection();
  this->GetImpl()->SetDirection(theVal);
  if (theVal != oldValue)
    SMESH::TPythonDump() << _this() << ".SetDirection( " << theVal << " )";
}

void HexoticPlugin_Hypothesis_i::SetGrowth(CORBA::Double theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetGrowth");
  ASSERT(myBaseImpl);
  CORBA::Double oldValue = GetGrowth();
  this->GetImpl()->SetGrowth(theVal);
  if (theVal != oldValue)
    SMESH::TPythonDump() << _this() << ".SetGrowth( " << theVal << " )";
}

void HexoticPlugin_Hypothesis_i::SetFacesWithLayers(const ::SMESH::long_array& theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetFacesWithLayers");
  std::vector<int> ids( theVal.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
   ids[i] = theVal[i];

  bool valueChanged = this->GetImpl()->SetFacesWithLayers(ids);
  if (valueChanged)
    SMESH::TPythonDump() << _this() << ".SetFacesWithLayers( "<< theVal << " )";
}

void HexoticPlugin_Hypothesis_i::SetImprintedFaces(const ::SMESH::long_array& theVal)
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::SetImprintedFaces");
  std::vector<int> ids( theVal.length() );
  for ( unsigned i = 0; i < ids.size(); ++i )
   ids[i] = theVal[i];

  bool valueChanged = this->GetImpl()->SetImprintedFaces(ids);
  if (valueChanged)
    SMESH::TPythonDump() << _this() << ".SetImprintedFaces( "<< theVal << " )";
}

//=============================================================================
/*!
 *  HexoticPlugin_Hypothesis_i::GetHexesMinLevel
 *  HexoticPlugin_Hypothesis_i::GetHexesMaxLevel
 *  HexoticPlugin_Hypothesis_i::GetMinSize
 *  HexoticPlugin_Hypothesis_i::GetMaxSize
 *  HexoticPlugin_Hypothesis_i::GetHexoticIgnoreRidges
 *  HexoticPlugin_Hypothesis_i::GetHexoticInvalidElements
 *  HexoticPlugin_Hypothesis_i::GetHexoticSharpAngleThreshold 
 *  HexoticPlugin_Hypothesis_i::GetHexoticNbProc 
 *  HexoticPlugin_Hypothesis_i::GetHexoticWorkingDirectory 
 *  HexoticPlugin_Hypothesis_i::GetHexoticSdMode
 *  HexoticPlugin_Hypothesis_i::GetVerbosity
 *  HexoticPlugin_Hypothesis_i::GetHexoticMaxMemory
 *  HexoticPlugin_Hypothesis_i::GetTextOptions
 *  HexoticPlugin_Hypothesis_i::GetNbLayers
 *  HexoticPlugin_Hypothesis_i::GetFirstLayerSize
 *  HexoticPlugin_Hypothesis_i::GetDirection
 *  HexoticPlugin_Hypothesis_i::GetGrowth
 *  HexoticPlugin_Hypothesis_i::GetFacesWithLayers
 *  HexoticPlugin_Hypothesis_i::GetImprintedFaces
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

CORBA::Double HexoticPlugin_Hypothesis_i::GetMinSize()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetMinSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMinSize();
}

CORBA::Double HexoticPlugin_Hypothesis_i::GetMaxSize()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetMaxSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetMaxSize();
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

CORBA::Double HexoticPlugin_Hypothesis_i::GetHexoticSharpAngleThreshold()
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

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexoticSdMode ()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticSdMode");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticSdMode();
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexoticVerbosity()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetVerbosity");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticVerbosity();
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetHexoticMaxMemory()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetHexoticMaxMemory");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetHexoticMaxMemory();
}

char* HexoticPlugin_Hypothesis_i::GetAdvancedOption()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetAdvancedOption");
  ASSERT(myBaseImpl);
  return CORBA::string_dup( this->GetImpl()->GetAdvancedOption().c_str() );
}
char* HexoticPlugin_Hypothesis_i::GetTextOptions()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetTextOptions");
  ASSERT(myBaseImpl);
  return CORBA::string_dup( this->GetImpl()->GetAdvancedOption().c_str() );
}

CORBA::Long HexoticPlugin_Hypothesis_i::GetNbLayers()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetNbLayers");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetNbLayers();
}

CORBA::Double HexoticPlugin_Hypothesis_i::GetFirstLayerSize()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetFirstLayerSize");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetFirstLayerSize();
}

CORBA::Boolean HexoticPlugin_Hypothesis_i::GetDirection()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetDirection");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetDirection();
}

CORBA::Double HexoticPlugin_Hypothesis_i::GetGrowth()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetGrowth");
  ASSERT(myBaseImpl);
  return this->GetImpl()->GetGrowth();
}

SMESH::long_array* HexoticPlugin_Hypothesis_i::GetFacesWithLayers()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetFacesWithLayers");
  ASSERT(myBaseImpl);
  std::vector<int> idsVec = this->GetImpl()->GetFacesWithLayers();
  SMESH::long_array_var ids = new SMESH::long_array;
  ids->length( idsVec.size() );
  for ( unsigned i = 0; i < idsVec.size(); ++i )
    ids[i] = idsVec[i];
  return ids._retn();
}

SMESH::long_array* HexoticPlugin_Hypothesis_i::GetImprintedFaces()
{
  // MESSAGE("HexoticPlugin_Hypothesis_i::GetImprintedFaces");
  ASSERT(myBaseImpl);
  std::vector<int> idsVec = this->GetImpl()->GetImprintedFaces();
  SMESH::long_array_var ids = new SMESH::long_array;
  ids->length( idsVec.size() );
  for ( unsigned i = 0; i < idsVec.size(); ++i )
    ids[i] = idsVec[i];
  return ids._retn();
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
