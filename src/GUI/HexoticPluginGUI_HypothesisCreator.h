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
// File   : HexoticPluginGUI_HypothesisCreator.h
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#ifndef HexoticPLUGINGUI_HypothesisCreator_H
#define HexoticPLUGINGUI_HypothesisCreator_H

#include "HexoticPluginGUI.h"

#include <SMESHGUI_Hypotheses.h>
#include "StdMeshersGUI_ObjectReferenceParamWdg.h"
#include "HexoticPlugin_Hypothesis.hxx"

class QtxIntSpinBox;
class QCheckBox;
class QLineEdit;

class HexoticPluginGUI_StdWidget;
class HexoticPluginGUI_SizeMapsWidget;

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
  int      myHexoticMaxMemory;
  int      myHexoticSdMode;
  HexoticPlugin_Hypothesis::THexoticSizeMaps mySizeMaps;
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
  
private:
  bool readParamsFromHypo( HexoticHypothesisData& ) const;
  bool readParamsFromWidgets( HexoticHypothesisData& ) const;
  bool readSizeMapsFromWidgets( HexoticHypothesisData& ) const;
  void insertLocalSizeInWidget( std::string entry, std::string shapeName, double size, int row ) const;
  bool storeParamsToHypo( const HexoticHypothesisData& ) const;
  void resizeEvent(QResizeEvent *event);
  void printData(HexoticHypothesisData&) const;
  
  GEOM::GEOM_Object_var entryToObject( std::string entry) const;

private:

//  QWidget* 		myStdGroup;
  QLineEdit* 	myName;
  HexoticPluginGUI_StdWidget*	         myStdWidget;
  HexoticPluginGUI_SizeMapsWidget*       mySmpWidget;
  StdMeshersGUI_ObjectReferenceParamWdg* myGeomSelWdg;

  bool                           myIs3D;
  std::vector< std::string >     mySizeMapsToRemove;
 
protected slots:
  void             onAddLocalSize();
  void             onRemoveLocalSize();
  
};

#endif
