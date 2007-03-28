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
    _hexoticQuadrangles( GetDefaultHexoticQuadrangles() )
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

//=============================================================================
/*!
 *  
 */
//=============================================================================
ostream & HexoticPlugin_Hypothesis::SaveTo(ostream & save)
{
  save << _hexesMinLevel << " " << _hexesMaxLevel;
  save << " " << (int)_hexoticQuadrangles;
  cout <<endl;
  cout << "save : " << save << endl;
  cout << endl;

  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================
istream & HexoticPlugin_Hypothesis::LoadFrom(istream & load)
{
//   bool isOK = true;
//   int is;
//   double val;

//   isOK = (load >> val);
//   if (isOK)
//     _maxSize = val;
//   else
//     load.clear(ios::badbit | load.rdstate());

//   isOK = (load >> is);
//   if (isOK)
//     SetFineness((Fineness) is);
//   else
//     load.clear(ios::badbit | load.rdstate());

//   if (_fineness == UserDefined)
//   {
//     isOK = (load >> val);
//     if (isOK)
//       _growthRate = val;
//     else
//       load.clear(ios::badbit | load.rdstate());

//     isOK = (load >> val);
//     if (isOK)
//       _nbSegPerEdge = val;
//     else
//       load.clear(ios::badbit | load.rdstate());

//     isOK = (load >> val);
//     if (isOK)
//       _nbSegPerRadius = val;
//     else
//       load.clear(ios::badbit | load.rdstate());
//   }

//   isOK = (load >> is);
//   if (isOK)
//     _secondOrder = (bool) is;
//   else
//     load.clear(ios::badbit | load.rdstate());

//   isOK = (load >> is);
//   if (isOK)
//     _optimize = (bool) is;
//   else
//     load.clear(ios::badbit | load.rdstate());
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
