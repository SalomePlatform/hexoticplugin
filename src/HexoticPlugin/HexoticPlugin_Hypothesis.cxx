// Copyright (C) 2007-2013  CEA/DEN, EDF R&D
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
// File   : HexoticPlugin_Hypothesis.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hypothesis.hxx"
#include <utilities.h>

#include <TCollection_AsciiString.hxx>

//=============================================================================
/*!
 *  
 */
//=============================================================================
HexoticPlugin_Hypothesis::HexoticPlugin_Hypothesis (int hypId, int studyId,
                                                    SMESH_Gen* gen)
  : SMESH_Hypothesis(hypId, studyId, gen),
    _hexesMinLevel( GetDefaultHexesMinLevel() ),
    _hexesMaxLevel( GetDefaultHexesMaxLevel() ),
    _minSize( GetDefaultMinSize() ),
    _maxSize( GetDefaultMaxSize() ),
    _hexoticIgnoreRidges( GetDefaultHexoticIgnoreRidges() ),
    _hexoticInvalidElements( GetDefaultHexoticInvalidElements() ), 
    _hexoticSharpAngleThreshold( GetDefaultHexoticSharpAngleThreshold() ),
    _hexoticNbProc( GetDefaultHexoticNbProc() ),
    _hexoticWorkingDirectory( GetDefaultHexoticWorkingDirectory() ),
    _hexoticSdMode(GetDefaultHexoticSdMode()),
    _hexoticVerbosity(GetDefaultHexoticVerbosity()),
    _hexoticMaxMemory(GetDefaultHexoticMaxMemory()),
    _sizeMaps(GetDefaultHexoticSizeMaps())
{
  MESSAGE("HexoticPlugin_Hypothesis::HexoticPlugin_Hypothesis");
  _name = "Hexotic_Parameters";
  _param_algo_dim = 3;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

void HexoticPlugin_Hypothesis::SetHexesMinLevel(int theVal) {
  if (theVal != _hexesMinLevel) {
    _hexesMinLevel = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexesMaxLevel(int theVal) {
  if (theVal != _hexesMaxLevel) {
    _hexesMaxLevel = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetMinSize(double theVal) {
  if (theVal != _minSize) {
	  _minSize = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetMaxSize(double theVal) {
  if (theVal != _maxSize) {
	  _maxSize = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticIgnoreRidges(bool theVal) {
  if (theVal != _hexoticIgnoreRidges) {
    _hexoticIgnoreRidges = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticInvalidElements(bool theVal) {
  if (theVal != _hexoticInvalidElements) {
    _hexoticInvalidElements = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticSharpAngleThreshold(double theVal) {
  if (theVal != _hexoticSharpAngleThreshold) {
    _hexoticSharpAngleThreshold = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticNbProc(int theVal) {
  if (theVal != _hexoticNbProc) {
    _hexoticNbProc = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticWorkingDirectory(const std::string& path)
{
  if ( _hexoticWorkingDirectory != path ) {
    _hexoticWorkingDirectory = path;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticSdMode(int theVal) {
  if (theVal != _hexoticSdMode) {
    _hexoticSdMode = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticVerbosity(int theVal) {
  if (theVal != _hexoticVerbosity) {
    _hexoticVerbosity = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

void HexoticPlugin_Hypothesis::SetHexoticMaxMemory(int theVal) {
  if (theVal != _hexoticMaxMemory) {
    _hexoticMaxMemory = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

bool HexoticPlugin_Hypothesis::AddSizeMap(std::string theEntry, double theSize) {
  THexoticSizeMaps::iterator it;
  it=_sizeMaps.find(theEntry);
  
  if(theSize <= 0)
    return false;
  
  if( it == _sizeMaps.end() ) // If no size map is defined on the given object
  {
    _sizeMaps[theEntry] = theSize;
    MESSAGE("NEW size map, entry :"<<theEntry<<", size : "<<theSize);
    NotifySubMeshesHypothesisModification();
    return true;
  }
  else if( it->second != theSize ) // If a size map exists with a different size value
  {
    it->second = theSize;
    MESSAGE("MODIFIED size map, entry :"<<theEntry<<"with size : "<<theSize);
    NotifySubMeshesHypothesisModification();
    return true;
  }
  else
  {
    MESSAGE("NO size map added")
    return false; // No size map added
  }
}

bool HexoticPlugin_Hypothesis::UnsetSizeMap(std::string theEntry) {
  THexoticSizeMaps::iterator it;
  it=_sizeMaps.find(theEntry);
  if( it != _sizeMaps.end() )
  {
    _sizeMaps.erase(it);  
    return true;
  }
  else
    return false;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
std::ostream& HexoticPlugin_Hypothesis::SaveTo(std::ostream& save)
{
  //explicit outputs for future code compatibility of saved .hdf
  //save without any whitespaces!
  int dummy = -1;
  save<<"hexesMinLevel="<<_hexesMinLevel<<";"; 
  save<<"hexesMaxLevel="<<_hexesMaxLevel<<";";
  save<<"hexoticIgnoreRidges="<<(int)_hexoticIgnoreRidges<<";";
  save<<"hexoticInvalidElements="<<(int)_hexoticInvalidElements<<";";
  save<<"hexoticSharpAngleThreshold="<<_hexoticSharpAngleThreshold<<";";
  save<<"hexoticNbProc="<<_hexoticNbProc<<";";
  save<<"hexoticWorkingDirectory="<<_hexoticWorkingDirectory<<";";
  save<<"minSize="<<_minSize<<";";
  save<<"maxSize="<<_maxSize<<";";
  save<<"hexoticSdMode="<<_hexoticSdMode<<";";
  save<<"hexoticVerbosity="<<_hexoticVerbosity<<";";
  save<<"hexoticMaxMemory="<<_hexoticMaxMemory<<";";
  THexoticSizeMaps::iterator it = _sizeMaps.begin();
  if ( it != _sizeMaps.end() )
  {
    save<<"sizeMaps=";
    for ( ; it!=_sizeMaps.end() ; it++ )
    {
      save<< it->first << "/" << it->second << "#" ;
    }
    save<<";";
  }
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
std::istream& HexoticPlugin_Hypothesis::LoadFrom(std::istream& load)
{
   //explicit inputs for future code compatibility of saved .hdf
   bool isOK = true;
   std::string str1,str2,str3,str4;

   //save without any whitespaces!
   isOK = (load >> str1);
   if (!(isOK)) {
     //defaults values assumed
     load.clear(std::ios::badbit | load.rdstate());
     return load;
   }
   int pos = 0;
   int len = str1.length();
   while (pos < len) {
      int found = str1.find(';',pos);
      str2 = str1.substr(pos,found-pos);
      int eqpos = str2.find('=',0);
      str3 = str2.substr(0,eqpos);
      str4 = str2.substr(eqpos+1);
      pos = found + 1;

      if (str3=="hexesMinLevel") _hexesMinLevel = atoi(str4.c_str());
      if (str3=="hexesMaxLevel") _hexesMaxLevel = atoi(str4.c_str());
      if (str3=="hexoticQuadrangles") {}
      if (str3=="hexoticIgnoreRidges") _hexoticIgnoreRidges = (bool) atoi(str4.c_str());
      if (str3=="hexoticInvalidElements") _hexoticInvalidElements = (bool) atoi(str4.c_str());
      if (str3=="hexoticSharpAngleThreshold") _hexoticSharpAngleThreshold = atof(str4.c_str());
      if (str3=="hexoticNbProc") _hexoticNbProc = atoi(str4.c_str());
      if (str3=="hexoticWorkingDirectory") _hexoticWorkingDirectory = str4;
      if (str3=="minSize") _minSize = atof(str4.c_str());
      if (str3=="maxSize") _maxSize = atof(str4.c_str());
      if (str3=="hexoticSdMode") _hexoticSdMode = atoi(str4.c_str());
      if (str3=="hexoticVerbosity") _hexoticVerbosity = atoi(str4.c_str());
      if (str3=="hexoticMaxMemory") _hexoticMaxMemory = atoi(str4.c_str());
      if (str3=="sizeMaps")
      {
        std::string sm_substr, sm_substr1, sm_substr2;
        int sm_pos = 0;
        int sm_len = str4.length();
        while ( sm_pos < sm_len )
        {
          int sm_found = str4.find('#',sm_pos);
          sm_substr = str4.substr(sm_pos,sm_found-sm_pos);
          int sm_slashpos = sm_substr.find('/',0);
          sm_substr1 = sm_substr.substr(0,sm_slashpos);
          sm_substr2 = sm_substr.substr(sm_slashpos+1);
          _sizeMaps[sm_substr1] = atof(sm_substr2.c_str());
          sm_pos = sm_found + 1;
        }
      }
   }
   return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
std::ostream& operator <<(std::ostream& save, HexoticPlugin_Hypothesis& hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
std::istream& operator >>(std::istream& load, HexoticPlugin_Hypothesis& hyp)
{
  return hyp.LoadFrom( load );
}


//================================================================================
/*!
 * \brief Does nothing
 * \param theMesh - the built mesh
 * \param theShape - the geometry of interest
 * \retval bool - always false
 */
//================================================================================
bool HexoticPlugin_Hypothesis::SetParametersByMesh(const SMESH_Mesh*   theMesh,
                                                   const TopoDS_Shape& theShape)
{
  return false;
}
//================================================================================
/*!
 * \brief Initialize my parameter values by default parameters.
 *  \retval bool - true if parameter values have been successfully defined
 */
//================================================================================

bool HexoticPlugin_Hypothesis::SetParametersByDefaults(const TDefaults&  /*dflts*/,
                                                       const SMESH_Mesh* /*theMesh*/)
{
  return false;
}

//=============================================================================
int HexoticPlugin_Hypothesis::GetDefaultHexesMinLevel()
{
  return 6;
}

int HexoticPlugin_Hypothesis::GetDefaultHexesMaxLevel()
{
  return 10;
}

double HexoticPlugin_Hypothesis::GetDefaultMinSize()
{
  return 0.0;
}

double HexoticPlugin_Hypothesis::GetDefaultMaxSize()
{
  return 0.0;
}

bool HexoticPlugin_Hypothesis::GetDefaultHexoticIgnoreRidges()
{
  return false;
}

bool HexoticPlugin_Hypothesis::GetDefaultHexoticInvalidElements()
{
  return false;
}

double HexoticPlugin_Hypothesis::GetDefaultHexoticSharpAngleThreshold()
{
  return 60.0;
}

int HexoticPlugin_Hypothesis::GetDefaultHexoticNbProc()
{
  return 4;
}

std::string HexoticPlugin_Hypothesis::GetDefaultHexoticWorkingDirectory()
{
  std::string aTmpDir;

  char *Tmp_dir = getenv("SALOME_TMP_DIR");
#ifdef WIN32
  if(Tmp_dir == NULL) {
    Tmp_dir = getenv("TEMP");
    if( Tmp_dir== NULL )
      Tmp_dir = getenv("TMP");
  }
#endif
  if( Tmp_dir != NULL ) {
    aTmpDir = std::string(Tmp_dir);
#ifdef WIN32
    if(aTmpDir[aTmpDir.size()-1] != '\\') aTmpDir+='\\';
#else
    if(aTmpDir[aTmpDir.size()-1] != '/') aTmpDir+='/';
#endif
  }
  else {
#ifdef WIN32
    aTmpDir = "C:\\";
#else
    aTmpDir = "/tmp/";
#endif
  }
  return aTmpDir;
}

int HexoticPlugin_Hypothesis::GetDefaultHexoticSdMode()
{
  return 4;
}

int HexoticPlugin_Hypothesis::GetDefaultHexoticVerbosity()
{
  return 1;
}

int HexoticPlugin_Hypothesis::GetDefaultHexoticMaxMemory()
{
  return 2048;
}

HexoticPlugin_Hypothesis::THexoticSizeMaps HexoticPlugin_Hypothesis::GetDefaultHexoticSizeMaps()
{
  return THexoticSizeMaps();
}


