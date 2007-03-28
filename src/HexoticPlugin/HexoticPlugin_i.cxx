//  SMESH HexoticPlugin : implementaion of SMESH idl descriptions
//
//  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
//  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
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
//
//  File    : HexoticPlugin_i.cxx
//  Author  : Lioka RAZAFINDRAZAKA (CEA)
//  Module  : SMESH
//  $Header: /home/server/cvs-PAL/HexoticPLUGIN/HexoticPLUGIN_SRC/src/HexoticPlugin_i.cxx,v 1.3 2006/06/01 11:43:58 jfa Exp $

using namespace std;
#include "SMESH_Hypothesis_i.hxx"

#include "utilities.h"

#include "HexoticPlugin_Hexotic_i.hxx"
#include "HexoticPlugin_Hypothesis_i.hxx"

template <class T> class HexoticPlugin_Creator_i:public HypothesisCreator_i<T>
{
  // as we have 'module HexoticPlugin' in HexoticPlugin_Algorithm.idl
  virtual std::string GetModuleName() { return "HexoticPlugin"; }
};

//=============================================================================
/*!
 *
 */
//=============================================================================

extern "C"
{
  GenericHypothesisCreator_i* GetHypothesisCreator (const char* aHypName)
  {
    MESSAGE("GetHypothesisCreator " << aHypName);

    GenericHypothesisCreator_i* aCreator = 0;

    // Algorithms
    if (strcmp(aHypName, "Hexotic_3D") == 0)
      aCreator = new HexoticPlugin_Creator_i<HexoticPlugin_Hexotic_i>;
    // Hypotheses
    else if (strcmp(aHypName, "Hexotic_Parameters") == 0)
      aCreator = new HexoticPlugin_Creator_i<HexoticPlugin_Hypothesis_i>;
    else ;

    return aCreator;
  }
}
