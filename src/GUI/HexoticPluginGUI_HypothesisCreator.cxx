// Copyright (C) 2006-2008 OPEN CASCADE, CEA/DEN, EDF R&D
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
//
// File   : HexoticPluginGUI_HypothesisCreator.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
//
// ---

#include "HexoticPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include CORBA_SERVER_HEADER(HexoticPlugin_Algorithm)

#include <SUIT_Session.h>

#include <SalomeApp_Tools.h>

#include <QtxIntSpinBox.h>
#include <QtxDblSpinBox.h>

#include <QtxComboBox.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpixmap.h>

 enum Fineness
   {
     VeryCoarse,
     Coarse,
     Moderate,
     Fine,
     VeryFine,
     UserDefined
   };

HexoticPluginGUI_HypothesisCreator::HexoticPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs3D(true)
{
}

HexoticPluginGUI_HypothesisCreator::~HexoticPluginGUI_HypothesisCreator()
{
}

bool HexoticPluginGUI_HypothesisCreator::checkParams() const
{
  HexoticHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );
  bool res = storeParamsToHypo( data_new );
  return res;
}

QFrame* HexoticPluginGUI_HypothesisCreator::buildFrame()
{
  QFrame* fr = new QFrame( 0, "myframe" );
  QVBoxLayout* lay = new QVBoxLayout( fr, 7, 0 );

  QGroupBox* GroupC1 = new QGroupBox( 2, Qt::Horizontal, fr, "GroupC1" );
  lay->addWidget( GroupC1 );
  
  GroupC1->setTitle( tr( "SMESH_ARGUMENTS" ) );
  GroupC1->layout()->setSpacing( 6 );
  GroupC1->layout()->setMargin( 11 );
  
  myName = 0;
  if( isCreation() ) {
    new QLabel( tr( "SMESH_NAME" ), GroupC1 );
    myName = new QLineEdit( GroupC1 );
  }

  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
  HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  new QLabel( tr( "Hexotic_HEXES_MIN_LEVEL" ), GroupC1 );
  myHexesMinLevel = new QtxIntSpinBox( GroupC1 );
  // myHexesMinLevel->setMinValue( 3 );
  myHexesMinLevel->setMinValue( h->GetHexesMinLevel() );
  myHexesMinLevel->setMaxValue( 10 );
  myHexesMinLevel->setLineStep( 1 );
  
  new QLabel( tr( "Hexotic_HEXES_MAX_LEVEL" ), GroupC1 );
  myHexesMaxLevel = new QtxIntSpinBox( GroupC1 );
  myHexesMaxLevel->setMinValue( 3 );
  myHexesMaxLevel->setMaxValue( 10 );
  myHexesMaxLevel->setLineStep( 1 );

  myHexoticQuadrangles = new QCheckBox( tr( "Hexotic_QUADRANGLES" ), GroupC1 );
  GroupC1->addSpace(0);
  myIs3D = true;

  myHexoticIgnoreRidges = new QCheckBox( tr( "Hexotic_IGNORE_RIDGES" ), GroupC1 );
  GroupC1->addSpace(0);

  myHexoticInvalidElements = new QCheckBox( tr( "Hexotic_INVALID_ELEMENTS" ), GroupC1 );
  GroupC1->addSpace(0);

  new QLabel( tr( "Hexotic_SHARP_ANGLE_THRESHOLD" ), GroupC1 );
  myHexoticSharpAngleThreshold = new QtxIntSpinBox( GroupC1 );
  myHexoticSharpAngleThreshold->setMinValue( 0 );
  myHexoticSharpAngleThreshold->setMaxValue( 90 );
  myHexoticSharpAngleThreshold->setLineStep( 1 );

  return fr;
}

void HexoticPluginGUI_HypothesisCreator::retrieveParams() const
{
  HexoticHypothesisData data;
  readParamsFromHypo( data );

  if( myName )
    myName->setText( data.myName );
  myHexesMinLevel->setValue( data.myHexesMinLevel );
  myHexesMaxLevel->setValue( data.myHexesMaxLevel );
  myHexoticSharpAngleThreshold->setValue( data.myHexoticSharpAngleThreshold );

  myHexoticQuadrangles->setChecked( data.myHexoticQuadrangles );
  myHexoticIgnoreRidges->setChecked( data.myHexoticIgnoreRidges );
  myHexoticInvalidElements->setChecked( data.myHexoticInvalidElements );

  myHexesMinLevel->setEnabled(true);
  myHexesMaxLevel->setEnabled(true);
  myHexoticSharpAngleThreshold->setEnabled(true);
}

QString HexoticPluginGUI_HypothesisCreator::storeParams() const
{
  HexoticHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );

  QString valStr;
  valStr += tr("Hexotic_SEG_MIN_SIZE") + " = " + QString::number( data.myHexesMinLevel )   + "; ";
  valStr += tr("Hexotic_SEG_MAX_SIZE") + " = " + QString::number( data.myHexesMaxLevel ) + "; ";
  valStr += tr("Hexotic_QUADRANGLES")  + " = " + QString::number( data.myHexoticQuadrangles ) + "; ";
  valStr += tr("Hexotic_IGNORE_RIDGES")  + " = " + QString::number( data.myHexoticIgnoreRidges ) + "; ";
  valStr += tr("Hexotic_INVALID_ELEMENTS")  + " = " + QString::number( data.myHexoticInvalidElements ) + "; ";
  valStr += tr("Hexotic_SHARP_ANGLE_THRESHOLD") + " = " + QString::number( data.myHexoticSharpAngleThreshold ) + "; ";

  return valStr;
}

bool HexoticPluginGUI_HypothesisCreator::readParamsFromHypo( HexoticHypothesisData& h_data ) const
{
  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
    HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName = isCreation() && data ? data->Label : "";
  h_data.myHexesMinLevel = h->GetHexesMinLevel();
  h_data.myHexesMaxLevel = h->GetHexesMaxLevel();
  h_data.myHexoticQuadrangles = h->GetHexoticQuadrangles();
  h_data.myHexoticIgnoreRidges = h->GetHexoticIgnoreRidges();
  h_data.myHexoticInvalidElements = h->GetHexoticInvalidElements();
  h_data.myHexoticSharpAngleThreshold = h->GetHexoticSharpAngleThreshold();

  return true;
}

bool HexoticPluginGUI_HypothesisCreator::storeParamsToHypo( const HexoticHypothesisData& h_data ) const
{
  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
    HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( hypothesis() );

  bool ok = true;
  try
  {
    if( isCreation() )
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.latin1() );

    h->SetHexesMinLevel( h_data.myHexesMinLevel );
    h->SetHexesMaxLevel( h_data.myHexesMaxLevel );
    h->SetHexoticQuadrangles( h_data.myHexoticQuadrangles );
    h->SetHexoticIgnoreRidges( h_data.myHexoticIgnoreRidges );
    h->SetHexoticInvalidElements( h_data.myHexoticInvalidElements );
    h->SetHexoticSharpAngleThreshold( h_data.myHexoticSharpAngleThreshold );
  }
  catch(const SALOME::SALOME_Exception& ex)
  {
    SalomeApp_Tools::QtCatchCorbaException(ex);
    ok = false;
  }
  return ok;
}

bool HexoticPluginGUI_HypothesisCreator::readParamsFromWidgets( HexoticHypothesisData& h_data ) const
{
  h_data.myName          = myName ? myName->text() : "";
  h_data.myHexesMinLevel = myHexesMinLevel->value();
  h_data.myHexesMaxLevel = myHexesMaxLevel->value();
  h_data.myHexoticQuadrangles = myHexoticQuadrangles->isChecked();
  h_data.myHexoticIgnoreRidges = myHexoticIgnoreRidges->isChecked();
  h_data.myHexoticInvalidElements = myHexoticInvalidElements->isChecked();
  h_data.myHexoticSharpAngleThreshold = myHexoticSharpAngleThreshold->value();

  return true;
}

QString HexoticPluginGUI_HypothesisCreator::caption() const
{
  return tr( QString( "Hexotic_%1_TITLE" ).arg(myIs3D?QString("3D"):QString("3D")) );
}

QPixmap HexoticPluginGUI_HypothesisCreator::icon() const
{
  QString hypIconName = tr( QString("ICON_DLG_Hexotic_PARAMETERS%1").arg(myIs3D?QString(""):QString("")) );
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "HexoticPlugin", hypIconName );
}

QString HexoticPluginGUI_HypothesisCreator::type() const
{
  return tr( QString( "Hexotic_%1_HYPOTHESIS" ).arg(myIs3D?QString("3D"):QString("3D")) );
}
