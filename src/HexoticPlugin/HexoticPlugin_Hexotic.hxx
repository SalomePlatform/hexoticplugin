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
// File      : HexoticPlugin_Hexotic.hxx
// Author    : Lioka RAZAFINDRAZAKA (CEA)
// Date      : 2006/06/30
// Project   : SALOME
//=============================================================================

#ifndef _HexoticPlugin_Hexotic_HXX_
#define _HexoticPlugin_Hexotic_HXX_

#include "SMESH_3D_Algo.hxx"
#include "SMESH_Mesh.hxx"
#include "Utils_SALOME_Exception.hxx"

class SMESH_Mesh;
class HexoticPlugin_Hypothesis;

class HexoticPlugin_Hexotic: public SMESH_3D_Algo
{
public:
  HexoticPlugin_Hexotic(int hypId, int studyId, SMESH_Gen* gen);
  virtual ~HexoticPlugin_Hexotic();

  virtual bool CheckHypothesis(SMESH_Mesh&                          aMesh,
                               const TopoDS_Shape&                  aShape,
                               SMESH_Hypothesis::Hypothesis_Status& aStatus);

  void SetParameters(const HexoticPlugin_Hypothesis* hyp);

  virtual bool Compute(SMESH_Mesh&         aMesh,
                       const TopoDS_Shape& aShape);

  ostream & SaveTo(ostream & save);
  istream & LoadFrom(istream & load);
  friend ostream & operator << (ostream & save, HexoticPlugin_Hexotic & hyp);
  friend istream & operator >> (istream & load, HexoticPlugin_Hexotic & hyp);

protected:
  const HexoticPlugin_Hypothesis* _hypothesis;
private:
  int  _iShape;
  int  _nbShape;
  int  _hexesMinLevel;
  int  _hexesMaxLevel;
  bool _hexoticQuadrangles;
  bool _hexoticIgnoreRidges;
  bool _hexoticInvalidElements;
  int  _hexoticSharpAngleThreshold;
  SMDS_MeshNode** _tabNode;
};

#endif
