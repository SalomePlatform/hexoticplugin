//  HexoticPlugin : C++ implementation
//
//  Copyright (C) 2006  OPEN CASCADE, CEA/DEN, EDF R&D
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
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
// File      : HexoticPlugin_Hypothesis.cxx
// Author    : Lioka RAZAFINDRAZAKA (CEA)
// Date      : 2006/06/30
// Project   : SALOME
//=============================================================================

using namespace std;
#include <HexoticPlugin_Hypothesis.hxx>
#include <utilities.h>

#include <qstring.h>

//=============================================================================
/*!
 *  
 */
//=============================================================================
HexoticPlugin_Hypothesis::HexoticPlugin_Hypothesis (int hypId, int studyId,
                                                  SMESH_Gen * gen)
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
ostream & HexoticPlugin_Hypothesis::SaveTo(ostream & save)
{
  /*save << _hexesMinLevel << " " << _hexesMaxLevel;
  save << " " << (int)_hexoticQuadrangles;
  save << " " << (int)_hexoticIgnoreRidges;
  save << " " << (int)_hexoticInvalidElements;
  save << " " << _hexoticSharpAngleThreshold;
  cout <<endl;
  cout << "save : " << save << endl;
  cout << endl;*/

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
istream & HexoticPlugin_Hypothesis::LoadFrom(istream & load)
{
   //explicit inputs for future code compatibility of saved .hdf
   bool isOK = true;
   int imax;
   string str1;
   QString str2,str3,str4,str5;
   
   //save without any whitespaces!
   isOK = (load >> str1);
   if (!(isOK)) {
     //defaults values assumed
     load.clear(ios::badbit | load.rdstate());
     return load;
   }
   str2 = (QString) str1;
   imax = str2.contains(";");
   for (int i=0; i<=imax; i++) {
      str3 = str2.section(";",i,i);
      str4 = str3.section("=",0,0);
      str5 = str3.section("=",1,1);

      if (str4=="hexesMinLevel") _hexesMinLevel = str5.toInt();
      if (str4=="hexesMaxLevel") _hexesMaxLevel = str5.toInt();
      if (str4=="hexoticQuadrangles") _hexoticQuadrangles = (bool) str5.toInt();
      if (str4=="hexoticIgnoreRidges") _hexoticIgnoreRidges = (bool) str5.toInt();
      if (str4=="hexoticInvalidElements") _hexoticInvalidElements = (bool) str5.toInt();
      if (str4=="hexoticSharpAngleThreshold") _hexoticSharpAngleThreshold = str5.toInt();
   }
   return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
ostream & operator <<(ostream & save, HexoticPlugin_Hypothesis & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
istream & operator >>(istream & load, HexoticPlugin_Hypothesis & hyp)
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
