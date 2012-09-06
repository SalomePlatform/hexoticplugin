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
// File   : HexoticPluginGUI_HypothesisCreator.h
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#ifndef HexoticPLUGINGUI_HypothesisCreator_H
#define HexoticPLUGINGUI_HypothesisCreator_H

#include "HexoticPluginGUI.h"

#include <SMESHGUI_Hypotheses.h>

class QtxIntSpinBox;
class QCheckBox;
class QLineEdit;

class HexoticPluginGUI_StdWidget;

typedef struct
{
  QString  myName;
  int      myHexesMinLevel, myHexesMaxLevel;
  double   myMinSize, myMaxSize;
  bool     myHexoticInvalidElements;
  bool     myHexoticIgnoreRidges;
  double   myHexoticSharpAngleThreshold;
  int      myHexoticNbProc;
  QString  myHexoticWorkingDir;
  int      myHexoticVerbosity;
  int      myHexoticSdMode;
} HexoticHypothesisData;

/*!
 * \brief Class for creation of Hexotic hypotheses
*/
class HEXOTICPLUGIN_GUI_EXPORT HexoticPluginGUI_HypothesisCreator : public SMESHGUI_GenericHypothesisCreator
{
  Q_OBJECT

public:
  HexoticPluginGUI_HypothesisCreator( const QString& );
  virtual ~HexoticPluginGUI_HypothesisCreator();

  virtual bool checkParams(QString&) const;
  virtual QString  helpPage() const;

protected:
  virtual QFrame*  buildFrame    ();
  virtual void     retrieveParams() const;
  virtual QString  storeParams   () const;
  
  virtual QString  caption() const;
  virtual QPixmap  icon() const;
  virtual QString  type() const;
  
protected slots:
  void             onDirBtnClicked();

private:
  bool readParamsFromHypo( HexoticHypothesisData& ) const;
  bool readParamsFromWidgets( HexoticHypothesisData& ) const;
  bool storeParamsToHypo( const HexoticHypothesisData& ) const;
  void resizeEvent(QResizeEvent *event);
  void printData(HexoticHypothesisData&) const;

private:

//  QWidget* 		myStdGroup;
  QLineEdit* 	myName;
  HexoticPluginGUI_StdWidget*	myStdWidget;

 bool             myIs3D;
};

#endif
