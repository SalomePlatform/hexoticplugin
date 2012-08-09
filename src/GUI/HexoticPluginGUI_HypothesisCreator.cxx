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
// File   : HexoticPluginGUI_HypothesisCreator.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPluginGUI_HypothesisCreator.h"

#include <SMESHGUI_Utils.h>
#include <SMESHGUI_HypothesesUtils.h>

#include CORBA_SERVER_HEADER(HexoticPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_FileDlg.h>
#include <SalomeApp_Tools.h>
#include <QtxIntSpinBox.h>

#include <QFrame>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

enum Fineness {
  VeryCoarse,
  Coarse,
  Moderate,
  Fine,
  VeryFine,
  UserDefined
};

HexoticPluginGUI_HypothesisCreator::HexoticPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs3D( true )
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
  QFrame* fr = new QFrame( 0 );
  QVBoxLayout* lay = new QVBoxLayout( fr );
  lay->setMargin( 0 );
  lay->setSpacing( 6 );

  QGroupBox* GroupC1 = new QGroupBox( tr( "SMESH_ARGUMENTS" ), fr );
  lay->addWidget( GroupC1 );
  
  QGridLayout* l = new QGridLayout( GroupC1 );
  l->setSpacing( 6 );
  l->setMargin( 11 );
  
  int row = 0;
  myName = 0;
  if( isCreation() ) {
    l->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row, 0, 1, 2 );
    myName = new QLineEdit( GroupC1 );
    l->addWidget( myName, row++, 2, 1, 1 );
    myName->setMinimumWidth( 150 );
  }

  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
  HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );
  
  l->addWidget( new QLabel( tr( "Hexotic_HEXES_MIN_LEVEL" ), GroupC1 ), row, 0, 1, 2 );
  myHexesMinLevel = new QtxIntSpinBox( GroupC1 );
  myHexesMinLevel->setMinimum( 3 );
  //myHexesMinLevel->setMinimum( h->GetHexesMinLevel() );
  myHexesMinLevel->setMaximum( 10 );
  myHexesMinLevel->setSingleStep( 1 );
  l->addWidget( myHexesMinLevel, row++, 2, 1, 1 );
  
  l->addWidget( new QLabel( tr( "Hexotic_HEXES_MAX_LEVEL" ), GroupC1 ), row, 0, 1, 2 );
  myHexesMaxLevel = new QtxIntSpinBox( GroupC1 );
  myHexesMaxLevel->setMinimum( 3 );
  myHexesMaxLevel->setMaximum( 10 );
  myHexesMaxLevel->setSingleStep( 1 );
  l->addWidget( myHexesMaxLevel, row++, 2, 1, 1 );

  myHexoticQuadrangles = new QCheckBox( tr( "Hexotic_QUADRANGLES" ), GroupC1 );
  l->addWidget( myHexoticQuadrangles, row++, 0, 1, 3 );
  myIs3D = true;

  myHexoticIgnoreRidges = new QCheckBox( tr( "Hexotic_IGNORE_RIDGES" ), GroupC1 );
  l->addWidget( myHexoticIgnoreRidges, row++, 0, 1, 3 );

  myHexoticInvalidElements = new QCheckBox( tr( "Hexotic_INVALID_ELEMENTS" ), GroupC1 );
  l->addWidget( myHexoticInvalidElements, row++, 0, 1, 3 );

  l->addWidget( new QLabel( tr( "Hexotic_SHARP_ANGLE_THRESHOLD" ), GroupC1 ), row, 0, 1, 2 );
  myHexoticSharpAngleThreshold = new QtxIntSpinBox( GroupC1 );
  myHexoticSharpAngleThreshold->setMinimum( 0 );
  myHexoticSharpAngleThreshold->setMaximum( 90 );
  myHexoticSharpAngleThreshold->setSingleStep( 1 );
  l->addWidget( myHexoticSharpAngleThreshold, row++, 2, 1, 1 );

  l->addWidget( new QLabel( tr( "Hexotic_NB_PROC" ), GroupC1 ), row, 0, 1, 2 );
  myHexoticNbProc = new QtxIntSpinBox( GroupC1 );
  myHexoticNbProc->setMinimum( 1 );
  myHexoticNbProc->setMaximum( 256 );
  myHexoticNbProc->setSingleStep( 1 );
  l->addWidget( myHexoticNbProc, row++, 2, 1, 1 );

  l->addWidget( new QLabel( tr( "Hexotic_WORKING_DIR" ), GroupC1 ), row, 0, 1, 1 );
  QPushButton* dirBtn = new QPushButton( tr( "Hexotic_SELECT_DIR" ), GroupC1 );
  dirBtn->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  l->addWidget( dirBtn, row, 1, 1, 1 );  
  myHexoticWorkingDir = new QLineEdit( GroupC1 );
  l->addWidget( myHexoticWorkingDir, row++, 2, 1, 1 );

  connect( dirBtn,                  SIGNAL( clicked() ),       this, SLOT( onDirBtnClicked() ) );
  
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
  
  myHexoticNbProc->setValue( data.myHexoticNbProc );
  myHexoticWorkingDir->setText( data.myHexoticWorkingDir );
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
  valStr += tr("Hexotic_NB_PROC") + " = " + QString::number( data.myHexoticNbProc ) + "; ";
  valStr += tr("Hexotic_WORKING_DIR") + " = " + data.myHexoticWorkingDir + "; ";

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
  h_data.myHexoticNbProc = h->GetHexoticNbProc();
  h_data.myHexoticWorkingDir = h->GetHexoticWorkingDirectory();

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
      SMESH::SetName( SMESH::FindSObject( h ), h_data.myName.toLatin1().constData() );

    h->SetHexesMinLevel( h_data.myHexesMinLevel );
    h->SetHexesMaxLevel( h_data.myHexesMaxLevel );
    h->SetHexoticQuadrangles( h_data.myHexoticQuadrangles );
    h->SetHexoticIgnoreRidges( h_data.myHexoticIgnoreRidges );
    h->SetHexoticInvalidElements( h_data.myHexoticInvalidElements );
    h->SetHexoticSharpAngleThreshold( h_data.myHexoticSharpAngleThreshold );
    h->SetHexoticNbProc( h_data.myHexoticNbProc );
    h->SetHexoticWorkingDirectory( h_data.myHexoticWorkingDir.toLatin1().constData() );
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
  h_data.myHexoticNbProc = myHexoticNbProc->value();
  h_data.myHexoticWorkingDir = myHexoticWorkingDir->text();

  return true;
}

QString HexoticPluginGUI_HypothesisCreator::caption() const
{
  return myIs3D ? tr( "Hexotic_3D_TITLE" ) : tr( "Hexotic_3D_TITLE" ); // ??? 3D/2D ???
}

QPixmap HexoticPluginGUI_HypothesisCreator::icon() const
{
  QString hypIconName = myIs3D ? tr( "ICON_DLG_Hexotic_PARAMETERS" ) : tr( "ICON_DLG_Hexotic_PARAMETERS" );
  return SUIT_Session::session()->resourceMgr()->loadPixmap( "HexoticPLUGIN", hypIconName );
}

QString HexoticPluginGUI_HypothesisCreator::type() const
{
  return myIs3D ? tr( "Hexotic_3D_HYPOTHESIS" ) : tr( "Hexotic_3D_HYPOTHESIS" ); // ??? 3D/2D ???
}

QString HexoticPluginGUI_HypothesisCreator::helpPage() const
{
  return "hexotic_hypo_page.html";
}

void HexoticPluginGUI_HypothesisCreator::onDirBtnClicked()
{
  QString dir = SUIT_FileDlg::getExistingDirectory( dlg(), myHexoticWorkingDir->text(), QString() );
  if ( !dir.isEmpty() )
    myHexoticWorkingDir->setText( dir );
}
