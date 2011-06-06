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
// File   : HexoticPluginGUI_HypothesisCreator.h
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#ifndef HexoticPLUGINGUI_HypothesisCreator_H
#define HexoticPLUGINGUI_HypothesisCreator_H

#include <SMESHGUI_Hypotheses.h>

class QtxIntSpinBox;
class QCheckBox;
class QLineEdit;

typedef struct
{
  QString  myName;
  int      myHexesMinLevel, myHexesMaxLevel;
  bool     myHexoticQuadrangles;
  bool     myHexoticInvalidElements;
  bool     myHexoticIgnoreRidges;
  int      myHexoticSharpAngleThreshold;
} HexoticHypothesisData;

/*!
 * \brief Class for creation of Hexotic hypotheses
*/
class HexoticPluginGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  HexoticPluginGUI_HypothesisCreator( const QString& );
  virtual ~HexoticPluginGUI_HypothesisCreator();

  virtual bool checkParams() const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual QString  storeParams   () const;
  
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;

private:
  bool readParamsFromHypo( HexoticHypothesisData& ) const;
  bool readParamsFromWidgets( HexoticHypothesisData& ) const;
  bool storeParamsToHypo( const HexoticHypothesisData& ) const;

private:
 QLineEdit*       myName;
 QtxIntSpinBox*   myHexesMinLevel;
 QtxIntSpinBox*   myHexesMaxLevel;
 QCheckBox*       myHexoticQuadrangles;
 QCheckBox*       myHexoticIgnoreRidges;
 QCheckBox*       myHexoticInvalidElements;
 QtxIntSpinBox*   myHexoticSharpAngleThreshold;

 bool             myIs3D;
};

#endif
