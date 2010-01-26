//  Copyright (C) 2007-2008  CEA/DEN, EDF R&D
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
//  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// ---
// File   : HexoticPlugin_Hypothesis.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hypothesis.hxx"
#include <utilities.h>

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
    _hexoticQuadrangles( GetDefaultHexoticQuadrangles() ),
    _hexoticIgnoreRidges( GetDefaultHexoticIgnoreRidges() ),
    _hexoticInvalidElements( GetDefaultHexoticInvalidElements() ), 
    _hexoticSharpAngleThreshold( GetDefaultHexoticSharpAngleThreshold() )
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

void HexoticPlugin_Hypothesis::SetHexoticQuadrangles(bool theVal) {
  if (theVal != _hexoticQuadrangles) {
    _hexoticQuadrangles = theVal;
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

void HexoticPlugin_Hypothesis::SetHexoticSharpAngleThreshold(int theVal) {
  if (theVal != _hexoticSharpAngleThreshold) {
    _hexoticSharpAngleThreshold = theVal;
    NotifySubMeshesHypothesisModification();
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
std::ostream& HexoticPlugin_Hypothesis::SaveTo(std::ostream& save)
{
  /*save << _hexesMinLevel << " " << _hexesMaxLevel;
  save << " " << (int)_hexoticQuadrangles;
  save << " " << (int)_hexoticIgnoreRidges;
  save << " " << (int)_hexoticInvalidElements;
  save << " " << _hexoticSharpAngleThreshold;
  std::cout <<std::endl;
  std::cout << "save : " << save << std::endl;
  std::cout << std::endl;*/

  //explicit outputs for future code compatibility of saved .hdf
  //save without any whitespaces!
  save<<"hexesMinLevel="<<_hexesMinLevel<<";"; 
  save<<"hexesMaxLevel="<<_hexesMaxLevel<<";";
  save<<"hexoticQuadrangles="<<(int)_hexoticQuadrangles<<";";
  save<<"hexoticIgnoreRidges="<<(int)_hexoticIgnoreRidges<<";";
  save<<"hexoticInvalidElements="<<(int)_hexoticInvalidElements<<";";
  save<<"hexoticSharpAngleThreshold="<<_hexoticSharpAngleThreshold<<";";
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
      if (str3=="hexoticQuadrangles") _hexoticQuadrangles = (bool) atoi(str4.c_str());
      if (str3=="hexoticIgnoreRidges") _hexoticIgnoreRidges = (bool) atoi(str4.c_str());
      if (str3=="hexoticInvalidElements") _hexoticInvalidElements = (bool) atoi(str4.c_str());
      if (str3=="hexoticSharpAngleThreshold") _hexoticSharpAngleThreshold = atoi(str4.c_str());
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
  return 3;
}

int HexoticPlugin_Hypothesis::GetDefaultHexesMaxLevel()
{
  return 8;
}

bool HexoticPlugin_Hypothesis::GetDefaultHexoticQuadrangles()
{
  return true;
}

bool HexoticPlugin_Hypothesis::GetDefaultHexoticIgnoreRidges()
{
  return false;
}

bool HexoticPlugin_Hypothesis::GetDefaultHexoticInvalidElements()
{
  return false;
}

int HexoticPlugin_Hypothesis::GetDefaultHexoticSharpAngleThreshold()
{
  return 60;
}
