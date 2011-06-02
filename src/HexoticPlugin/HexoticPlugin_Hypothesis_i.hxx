// Copyright (C) 2007-2011  CEA/DEN, EDF R&D
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
// File   : HexoticPlugin_Hypothesis_i.hxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#ifndef _HexoticPlugin_Hypothesis_i_HXX_
#define _HexoticPlugin_Hypothesis_i_HXX_

#include <SALOMEconfig.h>
#include CORBA_SERVER_HEADER(HexoticPlugin_Algorithm)

#include "SMESH_Hypothesis_i.hxx"
#include "HexoticPlugin_Hypothesis.hxx"

class SMESH_Gen;

// HexoticPlugin parameters hypothesis

class HexoticPlugin_Hypothesis_i:
  public virtual POA_HexoticPlugin::HexoticPlugin_Hypothesis,
  public virtual SMESH_Hypothesis_i
{
 public:
  // Constructor
  HexoticPlugin_Hypothesis_i (PortableServer::POA_ptr thePOA,
                             int                     theStudyId,
                             ::SMESH_Gen*            theGenImpl);
  // Destructor
  virtual ~HexoticPlugin_Hypothesis_i();

  // Methods for setting and getting parameters values
  void SetHexesMinLevel(CORBA::Long theVal);
  CORBA::Long GetHexesMinLevel();

  void SetHexesMaxLevel(CORBA::Long theVal);
  CORBA::Long GetHexesMaxLevel();

  void SetHexoticQuadrangles(CORBA::Boolean theVal);
  CORBA::Boolean GetHexoticQuadrangles();

  void SetHexoticIgnoreRidges(CORBA::Boolean theVal);
  CORBA::Boolean GetHexoticIgnoreRidges();
  
  void SetHexoticInvalidElements(CORBA::Boolean theVal);
  CORBA::Boolean GetHexoticInvalidElements();
  
  void SetHexoticSharpAngleThreshold(CORBA::Long theVal);
  CORBA::Long GetHexoticSharpAngleThreshold();

  // Get implementation
  ::HexoticPlugin_Hypothesis* GetImpl();
  
  // Verify whether hypothesis supports given entity type 
  CORBA::Boolean IsDimSupported( SMESH::Dimension type );
};

#endif
