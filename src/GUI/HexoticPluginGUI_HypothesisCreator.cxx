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
#include "HexoticPluginGUI_Dlg.h"

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


HexoticPluginGUI_HypothesisCreator::HexoticPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs3D( true )
{
}

HexoticPluginGUI_HypothesisCreator::~HexoticPluginGUI_HypothesisCreator()
{
}

bool HexoticPluginGUI_HypothesisCreator::checkParams(QString& msg) const
{
  msg.clear();
  HexoticHypothesisData data_old, data_new;
  readParamsFromHypo( data_old );
  readParamsFromWidgets( data_new );

  bool res = storeParamsToHypo( data_new );
  if ( !res ) {
    storeParamsToHypo( data_old );
    return res;
  }

  res = data_new.myMinSize <= data_new.myMaxSize;
  if ( !res ) {
    msg = tr(QString("Min size (%1) is higher than max size (%2)").arg(data_new.myMinSize).arg(data_new.myMaxSize).toStdString().c_str());
    return res;
  }

  res = data_new.myHexesMinLevel == 0  || \
      ( data_new.myHexesMinLevel != 0  && (data_new.myHexesMinLevel < data_new.myHexesMaxLevel) );
  if ( !res ) {
    msg = tr(QString("Min hexes level (%1) is higher than max hexes level (%2)").arg(data_new.myHexesMinLevel).arg(data_new.myHexesMaxLevel).toStdString().c_str());
    return res;
  }

  return true;
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
    l->addWidget( new QLabel( tr( "SMESH_NAME" ), GroupC1 ), row, 0, 1, 1 );
    myName = new QLineEdit( GroupC1 );
    l->addWidget( myName, row++, 1, 1, 2 );
    myName->setMinimumWidth( 150 );
  }

  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
  HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );
  
  myStdWidget = new HexoticPluginGUI_StdWidget(GroupC1);
  l->addWidget( myStdWidget, row++, 0, 1, 3 );
  myStdWidget->onSdModeSelected(SD_MODE_4);

  myIs3D = true;
//  resizeEvent();
  return fr;
}

//=================================================================================
// function : resizeEvent [REDEFINED]
// purpose  :
//=================================================================================
void HexoticPluginGUI_HypothesisCreator::resizeEvent(QResizeEvent */*event*/) {
    QSize scaledSize = myStdWidget->imageSdMode.size();
    scaledSize.scale(myStdWidget->sdModeLabel->size(), Qt::KeepAspectRatioByExpanding);
    if (!myStdWidget->sdModeLabel->pixmap() || scaledSize != myStdWidget->sdModeLabel->pixmap()->size())
    	myStdWidget->sdModeLabel->setPixmap(myStdWidget->imageSdMode.scaled(myStdWidget->sdModeLabel->size(),
    			Qt::KeepAspectRatio,
    			Qt::SmoothTransformation));
}

void HexoticPluginGUI_HypothesisCreator::retrieveParams() const
{
  HexoticHypothesisData data;
  readParamsFromHypo( data );
  printData(data);

  if( myName )
    myName->setText( data.myName );

  myStdWidget->myMinSize->setCleared(data.myMinSize == 0);
  if (data.myMinSize == 0)
    myStdWidget->myMinSize->setText("");
  else
    myStdWidget->myMinSize->setValue( data.myMinSize );

  myStdWidget->myMaxSize->setCleared(data.myMaxSize == 0);
  if (data.myMaxSize == 0)
    myStdWidget->myMaxSize->setText("");
  else
    myStdWidget->myMaxSize->setValue( data.myMaxSize );

  myStdWidget->myHexesMinLevel->setCleared(data.myHexesMinLevel == 0);
  if (data.myHexesMinLevel == 0)
    myStdWidget->myHexesMinLevel->setText("");
  else
    myStdWidget->myHexesMinLevel->setValue( data.myHexesMinLevel );

  myStdWidget->myHexesMaxLevel->setCleared(data.myHexesMaxLevel == 0);
  if (data.myHexesMaxLevel == 0)
    myStdWidget->myHexesMaxLevel->setText("");
  else
    myStdWidget->myHexesMaxLevel->setValue( data.myHexesMaxLevel );

  myStdWidget->myHexoticIgnoreRidges->setChecked( data.myHexoticIgnoreRidges );
  myStdWidget->myHexoticInvalidElements->setChecked( data.myHexoticInvalidElements );
  
  myStdWidget->myHexoticSharpAngleThreshold->setCleared(data.myHexoticSharpAngleThreshold == 0);
  if (data.myHexoticSharpAngleThreshold == 0)
    myStdWidget->myHexoticSharpAngleThreshold->setText("");
  else
    myStdWidget->myHexoticSharpAngleThreshold->setValue( data.myHexoticSharpAngleThreshold );

  myStdWidget->myHexoticNbProc->setValue( data.myHexoticNbProc );
  myStdWidget->myHexoticWorkingDir->setText( data.myHexoticWorkingDir );

  myStdWidget->myHexoticVerbosity->setValue( data.myHexoticVerbosity );

  myStdWidget->myHexoticMaxMemory->setValue( data.myHexoticMaxMemory );

  myStdWidget->myHexoticSdMode->setCurrentIndex(data.myHexoticSdMode);

  std::cout << "myStdWidget->myMinSize->value(): " << myStdWidget->myMinSize->value() << std::endl;
  std::cout << "myStdWidget->myMaxSize->value(): " << myStdWidget->myMaxSize->value() << std::endl;
  std::cout << "myStdWidget->myHexesMinLevel->value(): " << myStdWidget->myHexesMinLevel->value() << std::endl;
  std::cout << "myStdWidget->myHexesMaxLevel->value(): " << myStdWidget->myHexesMaxLevel->value() << std::endl;
  std::cout << "myStdWidget->myHexoticSharpAngleThreshold->value(): " << myStdWidget->myHexoticSharpAngleThreshold->value() << std::endl;

}

void HexoticPluginGUI_HypothesisCreator::printData( HexoticHypothesisData& data) const
{
  QString valStr;
  valStr += tr("Hexotic_MIN_SIZE") + " = " + QString::number( data.myMinSize )   + "; ";
  valStr += tr("Hexotic_MAX_SIZE") + " = " + QString::number( data.myMaxSize ) + "; ";
  valStr += tr("Hexotic_HEXES_MIN_LEVEL") + " = " + QString::number( data.myHexesMinLevel )   + "; ";
  valStr += tr("Hexotic_HEXES_MAX_LEVEL") + " = " + QString::number( data.myHexesMaxLevel ) + "; ";
  valStr += tr("Hexotic_IGNORE_RIDGES")  + " = " + QString::number( data.myHexoticIgnoreRidges ) + "; ";
  valStr += tr("Hexotic_INVALID_ELEMENTS")  + " = " + QString::number( data.myHexoticInvalidElements ) + "; ";
  valStr += tr("Hexotic_SHARP_ANGLE_THRESHOLD") + " = " + QString::number( data.myHexoticSharpAngleThreshold ) + "; ";
  valStr += tr("Hexotic_NB_PROC") + " = " + QString::number( data.myHexoticNbProc ) + "; ";
  valStr += tr("Hexotic_WORKING_DIR") + " = " + data.myHexoticWorkingDir + "; ";
  valStr += tr("Hexotic_VERBOSITY") + " = " + QString::number( data.myHexoticVerbosity ) + "; ";
  valStr += tr("Hexotic_MAX_MEMORY") + " = " + QString::number( data.myHexoticMaxMemory ) + "; ";
  valStr += tr("Hexotic_SD_MODE") + " = " + QString::number( data.myHexoticSdMode ) + "; ";

  std::cout << "Data: " << valStr.toStdString() << std::endl;
}

QString HexoticPluginGUI_HypothesisCreator::storeParams() const
{
  HexoticHypothesisData data;
  readParamsFromWidgets( data );
  storeParamsToHypo( data );

  QString valStr;
  valStr += tr("Hexotic_MIN_SIZE") + " = " + QString::number( data.myMinSize )   + "; ";
  valStr += tr("Hexotic_MAX_SIZE") + " = " + QString::number( data.myMaxSize ) + "; ";
  valStr += tr("Hexotic_HEXES_MIN_LEVEL") + " = " + QString::number( data.myHexesMinLevel )   + "; ";
  valStr += tr("Hexotic_HEXES_MAX_LEVEL") + " = " + QString::number( data.myHexesMaxLevel ) + "; ";
  valStr += tr("Hexotic_IGNORE_RIDGES")  + " = " + QString::number( data.myHexoticIgnoreRidges ) + "; ";
  valStr += tr("Hexotic_INVALID_ELEMENTS")  + " = " + QString::number( data.myHexoticInvalidElements ) + "; ";
  valStr += tr("Hexotic_SHARP_ANGLE_THRESHOLD") + " = " + QString::number( data.myHexoticSharpAngleThreshold ) + "; ";
  valStr += tr("Hexotic_NB_PROC") + " = " + QString::number( data.myHexoticNbProc ) + "; ";
  valStr += tr("Hexotic_WORKING_DIR") + " = " + data.myHexoticWorkingDir + "; ";
  valStr += tr("Hexotic_VERBOSITY") + " = " + QString::number( data.myHexoticVerbosity) + "; ";
  valStr += tr("Hexotic_MAX_MEMORY") + " = " + QString::number( data.myHexoticMaxMemory ) + "; ";
  valStr += tr("Hexotic_SD_MODE") + " = " + QString::number( data.myHexoticSdMode) + "; ";

//  std::cout << "Data: " << valStr.toStdString() << std::endl;

  return valStr;
}

bool HexoticPluginGUI_HypothesisCreator::readParamsFromHypo( HexoticHypothesisData& h_data ) const
{
  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
    HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );

  HypothesisData* data = SMESH::GetHypothesisData( hypType() );
  h_data.myName = isCreation() && data ? data->Label : "";
  h_data.myMinSize = h->GetMinSize();
  h_data.myMaxSize = h->GetMaxSize();
  h_data.myHexesMinLevel = h->GetHexesMinLevel();
  h_data.myHexesMaxLevel = h->GetHexesMaxLevel();
  h_data.myHexoticIgnoreRidges = h->GetHexoticIgnoreRidges();
  h_data.myHexoticInvalidElements = h->GetHexoticInvalidElements();
  h_data.myHexoticSharpAngleThreshold = h->GetHexoticSharpAngleThreshold();
  h_data.myHexoticNbProc = h->GetHexoticNbProc();
  h_data.myHexoticWorkingDir = h->GetHexoticWorkingDirectory();
  h_data.myHexoticVerbosity = h->GetHexoticVerbosity();
  h_data.myHexoticMaxMemory = h->GetHexoticMaxMemory();
  h_data.myHexoticSdMode = h->GetHexoticSdMode()-1;

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

    h->SetMinSize( h_data.myMinSize );
    h->SetMaxSize( h_data.myMaxSize );
    h->SetHexesMinLevel( h_data.myHexesMinLevel );
    h->SetHexesMaxLevel( h_data.myHexesMaxLevel );
    h->SetHexoticIgnoreRidges( h_data.myHexoticIgnoreRidges );
    h->SetHexoticInvalidElements( h_data.myHexoticInvalidElements );
    h->SetHexoticSharpAngleThreshold( h_data.myHexoticSharpAngleThreshold );
    h->SetHexoticNbProc( h_data.myHexoticNbProc );
    h->SetHexoticWorkingDirectory( h_data.myHexoticWorkingDir.toLatin1().constData() );
    h->SetHexoticVerbosity( h_data.myHexoticVerbosity );
    h->SetHexoticMaxMemory( h_data.myHexoticMaxMemory );
    h->SetHexoticSdMode( h_data.myHexoticSdMode+1 );
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
  h_data.myName    = myName ? myName->text() : "";

  h_data.myHexoticIgnoreRidges = myStdWidget->myHexoticIgnoreRidges->isChecked();
  h_data.myHexoticInvalidElements = myStdWidget->myHexoticInvalidElements->isChecked();

  h_data.myHexoticNbProc = myStdWidget->myHexoticNbProc->value();
  h_data.myHexoticWorkingDir = myStdWidget->myHexoticWorkingDir->text();
  h_data.myHexoticVerbosity = myStdWidget->myHexoticVerbosity->value();
  h_data.myHexoticMaxMemory = myStdWidget->myHexoticMaxMemory->value();
  h_data.myHexoticSdMode = myStdWidget->myHexoticSdMode->currentIndex();

  h_data.myMinSize = myStdWidget->myMinSize->text().isEmpty() ? 0.0 : myStdWidget->myMinSize->value();
  h_data.myMaxSize = myStdWidget->myMaxSize->text().isEmpty() ? 0.0 : myStdWidget->myMaxSize->value();
  h_data.myHexesMinLevel = myStdWidget->myHexesMinLevel->text().isEmpty() ? 0 : myStdWidget->myHexesMinLevel->value();
  h_data.myHexesMaxLevel = myStdWidget->myHexesMaxLevel->text().isEmpty() ? 0 : myStdWidget->myHexesMaxLevel->value();
  h_data.myHexoticSharpAngleThreshold = myStdWidget->myHexoticSharpAngleThreshold->text().isEmpty() ? 0 : myStdWidget->myHexoticSharpAngleThreshold->value();

  printData(h_data);

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
