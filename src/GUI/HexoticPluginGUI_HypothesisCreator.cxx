// Copyright (C) 2007-2016  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
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
#include <SMESH_NumberFilter.hxx>
#include <SMESH_AdvOptionsWdg.h>

#include "utilities.h"

#include CORBA_SERVER_HEADER(HexoticPlugin_Algorithm)

#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_MessageBox.h>
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

#include "SMESH_Gen_i.hxx"

// OCC includes
#include <TColStd_MapOfInteger.hxx>
#include <TopAbs.hxx>

// Main widget tabs identification
enum {
  STD_TAB = 0,
  ADV_TAB,
  SMP_TAB,
  VL_TAB
};

// Size maps tab, table columns order
enum {
  ENTRY_COL = 0,
  NAME_COL,
  SIZE_COL
};

//
// Size map table widget delegate
//

SizeMapsTableWidgetDelegate::SizeMapsTableWidgetDelegate(QObject *parent)
     : QItemDelegate(parent)
{
}

QWidget* SizeMapsTableWidgetDelegate::createEditor(QWidget *parent,
                                                   const QStyleOptionViewItem &/* option */,
                                                   const QModelIndex &/* index */) const
{
  SMESHGUI_SpinBox *editor = new SMESHGUI_SpinBox(parent);
  editor->RangeStepAndValidator(0.0, COORD_MAX, 10.0, "length_precision");
  return editor;
}

void SizeMapsTableWidgetDelegate::setEditorData(QWidget *editor,
                                                const QModelIndex &index) const
{
  double value = index.model()->data(index, Qt::EditRole).toDouble();
  SMESHGUI_SpinBox *spinBox = static_cast<SMESHGUI_SpinBox*>(editor);
  spinBox->setValue(value);
}

void SizeMapsTableWidgetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex &index) const
{
  SMESHGUI_SpinBox *spinBox = static_cast<SMESHGUI_SpinBox*>(editor);
  spinBox->interpretText();
  double value = spinBox->value();
  if ( value == 0 ) 
    SUIT_MessageBox::critical( spinBox, tr( "SMESH_ERROR" ), tr( "Hexotic_NULL_LOCAL_SIZE" ) ); 
  else
    model->setData(index, value, Qt::EditRole);
}

void SizeMapsTableWidgetDelegate::updateEditorGeometry(QWidget *editor,
                                                       const QStyleOptionViewItem &option, 
                                                       const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

// END Delegate



HexoticPluginGUI_HypothesisCreator::HexoticPluginGUI_HypothesisCreator( const QString& theHypType )
: SMESHGUI_GenericHypothesisCreator( theHypType ),
  myIs3D( true ),
  mySizeMapsToRemove()
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
  
  bool res = readParamsFromWidgets( data_new );
  if ( !res ){
    return res;
  }

  res = storeParamsToHypo( data_new );
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
  
  // main TabWidget of the dialog
  QTabWidget* aTabWidget = new QTabWidget( fr );
  aTabWidget->setTabShape( QTabWidget::Rounded );
  aTabWidget->setTabPosition( QTabWidget::North );
  lay->addWidget( aTabWidget );

  // Standard arguments tab
  QWidget* aStdGroup = new QWidget();
  QGridLayout* l = new QGridLayout( aStdGroup );
  l->setSpacing( 6 );
  l->setMargin( 11 );
 
  int row = 0;
  myName = 0;
  if( isCreation() ) {
    l->addWidget( new QLabel( tr( "SMESH_NAME" ), aStdGroup ), row, 0, 1, 1 );
    myName = new QLineEdit( aStdGroup );
    l->addWidget( myName, row++, 1, 1, 2 );
    myName->setMinimumWidth( 150 );
  }

  HexoticPlugin::HexoticPlugin_Hypothesis_var h =
  HexoticPlugin::HexoticPlugin_Hypothesis::_narrow( initParamsHypothesis() );
  
  myStdWidget = new HexoticPluginGUI_StdWidget(aStdGroup);
#ifdef WIN32
  myStdWidget->label_6->hide();
  myStdWidget->myHexoticNbProc->hide();
#endif
  l->addWidget( myStdWidget, row++, 0, 1, 3 );
  myStdWidget->onSdModeSelected(SD_MODE_4);

  myAdvWidget = new SMESH_AdvOptionsWdg( aTabWidget );
  
  // SIZE MAPS TAB
  QWidget* aSmpGroup = new QWidget();
  lay->addWidget( aSmpGroup );
  
  // Size map widget creation and initialisation
  mySmpWidget = new HexoticPluginGUI_SizeMapsWidget(aSmpGroup);
  mySmpWidget->doubleSpinBox->RangeStepAndValidator(0.0, COORD_MAX, 1.0, "length_precision");
  mySmpWidget->doubleSpinBox->setValue(0.0);
  
  // Filters of selection
  TColStd_MapOfInteger SM_ShapeTypes; 
  SM_ShapeTypes.Add( TopAbs_VERTEX );
  SM_ShapeTypes.Add( TopAbs_EDGE );
  SM_ShapeTypes.Add( TopAbs_WIRE );
  SM_ShapeTypes.Add( TopAbs_FACE );
  SM_ShapeTypes.Add( TopAbs_SOLID );
  SM_ShapeTypes.Add( TopAbs_COMPOUND );  
  SMESH_NumberFilter* aFilter = new SMESH_NumberFilter("GEOM", TopAbs_SHAPE, 0, SM_ShapeTypes);
  
  // Selection widget
  myGeomSelWdg = new StdMeshersGUI_ObjectReferenceParamWdg( aFilter, mySmpWidget, /*multiSel=*/false);
  myGeomSelWdg->SetDefaultText(tr("Hexotic_SEL_SHAPE"), "QLineEdit { color: grey }");
  mySmpWidget->gridLayout->addWidget(myGeomSelWdg, 0, 1);
  
  // Configuration of the table widget
  QStringList headerLabels;
  headerLabels << tr("Hexotic_ENTRY")<< tr("Hexotic_NAME")<< tr("Hexotic_SIZE");
  mySmpWidget->tableWidget->setHorizontalHeaderLabels(headerLabels);
  mySmpWidget->tableWidget->resizeColumnsToContents();
  mySmpWidget->tableWidget->hideColumn( 0 );
  mySmpWidget->label->setText(tr("LOCAL_SIZE"));
  mySmpWidget->pushButton_1->setText(tr("Hexotic_ADD"));
  mySmpWidget->pushButton_2->setText(tr("Hexotic_REMOVE"));
  
  // Setting a custom delegate for the size column
  SizeMapsTableWidgetDelegate* delegate = new SizeMapsTableWidgetDelegate();
  mySmpWidget->tableWidget->setItemDelegateForColumn(SIZE_COL, delegate);
  
  // Add the size maps widget to a layout
  QHBoxLayout* aSmpLayout = new QHBoxLayout( aSmpGroup );
  aSmpLayout->setMargin( 0 );
  aSmpLayout->addWidget( mySmpWidget);
  
  // Viscous Layers tab
  QWidget* aVLGroup = new QWidget();
  lay->addWidget( aVLGroup );

  // Viscous layers widget creation and initialisation
  myVLWidget = new HexoticPluginGUI_ViscousLayersWidget(aVLGroup);

  QString aMainEntry = SMESHGUI_GenericHypothesisCreator::getMainShapeEntry();
  QString aSubEntry  = SMESHGUI_GenericHypothesisCreator::getShapeEntry();

  if ( !aMainEntry.isEmpty() )
  {
    myVLWidget->myFacesWithLayers->SetGeomShapeEntry( aSubEntry, aMainEntry );
    myVLWidget->myImprintedFaces->SetGeomShapeEntry( aSubEntry, aMainEntry );
  }
  else
  {
        myVLWidget->labelFacesWithLayers->setVisible(false);
    myVLWidget->myFacesWithLayers->setVisible(false);
    myVLWidget->labelImprintedFaces->setVisible(false);
    myVLWidget->myImprintedFaces->setVisible(false);
  }

  // Add the viscous layers widget to a layout
  QHBoxLayout* aVLLayout = new QHBoxLayout( aVLGroup );
  aVLLayout->setSpacing( 6 );
  aVLLayout->setMargin( 11 );
  aVLLayout->addWidget( myVLWidget );

//  resizeEvent();
  
  aTabWidget->insertTab( STD_TAB, aStdGroup, tr( "SMESH_ARGUMENTS" ));
  aTabWidget->insertTab( ADV_TAB, myAdvWidget, tr( "SMESH_ADVANCED" ));
  aTabWidget->insertTab( SMP_TAB, aSmpGroup, tr( "LOCAL_SIZE" ));
  aTabWidget->insertTab( VL_TAB, aVLGroup, tr( "Hexotic_VISCOUS_LAYERS"));
  
  myIs3D = true;
  
  // Size Maps
  mySizeMapsToRemove.clear();
  connect( mySmpWidget->pushButton_1, SIGNAL( clicked() ),          this, SLOT( onAddLocalSize() ) );
  connect( mySmpWidget->pushButton_2, SIGNAL( clicked() ),          this, SLOT( onRemoveLocalSize() ) );
  connect( aTabWidget,                SIGNAL( currentChanged(int)), this, SLOT( onTabChanged( int ) ) );
  return fr;
}

void HexoticPluginGUI_HypothesisCreator::onAddLocalSize()
{
  int rowCount = mySmpWidget->tableWidget->rowCount();
  //int columnCount = mySmpWidget->tableWidget->columnCount();
  
  // Get the selected object properties
  GEOM::GEOM_Object_var sizeMapObject = myGeomSelWdg->GetObject< GEOM::GEOM_Object >(0);
  if (sizeMapObject->_is_nil())
    return;
  
  std::string entry, shapeName;
  entry = (std::string) sizeMapObject->GetStudyEntry();
  shapeName = sizeMapObject->GetName();
  
  // Check if the object is already in the widget
  QList<QTableWidgetItem *> listFound = mySmpWidget->tableWidget
                                        ->findItems( QString(entry.c_str()), Qt::MatchExactly );
  if ( !listFound.isEmpty() )
    return;
  
  // Get the size value
  double size = mySmpWidget->doubleSpinBox->value();
  if (size == 0)
  {
    SUIT_MessageBox::critical( mySmpWidget, tr( "SMESH_ERROR" ), tr( "Hexotic_NULL_LOCAL_SIZE" ) );
    return;
  }
  
  // Set items for the inserted row
  insertLocalSizeInWidget( entry, shapeName, size, rowCount );
}

void HexoticPluginGUI_HypothesisCreator::insertLocalSizeInWidget( std::string entry, 
                                                                  std::string shapeName, 
                                                                  double size, 
                                                                  int row ) const
{
  MESSAGE("HexoticPluginGUI_HypothesisCreator:insertLocalSizeInWidget")
  int columnCount = mySmpWidget->tableWidget->columnCount();
  
  // Add a row at the end of the table
  mySmpWidget->tableWidget->insertRow(row);
  
  QVariant value;
  for (int col = 0; col<columnCount; col++)
  {
    QTableWidgetItem* item = new QTableWidgetItem();
    switch ( col )
    {
      case ENTRY_COL:
        item->setFlags( 0 );
        value = QVariant( entry.c_str() );
        item->setData(Qt::DisplayRole, value );
        break;  
      case NAME_COL:
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        value = QVariant( shapeName.c_str() );
        item->setData(Qt::DisplayRole, value );
        break;
      case SIZE_COL:
        item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        value = QVariant( size );
        item->setData(Qt::EditRole, value );
        break;
    }       
    mySmpWidget->tableWidget->setItem(row,col,item);
  }
}

void HexoticPluginGUI_HypothesisCreator::onRemoveLocalSize()
{
  // Remove the selected rows in the table
  QList<QTableWidgetSelectionRange> ranges = mySmpWidget->tableWidget->selectedRanges();
  if ( ranges.isEmpty() ) // If none is selected remove the last one
  {
    int lastRow = mySmpWidget->tableWidget->rowCount() - 1;
    std::string entry = mySmpWidget->tableWidget->item( lastRow, ENTRY_COL )->text().toStdString();
    mySizeMapsToRemove.push_back(entry);
    mySmpWidget->tableWidget->removeRow( lastRow ); 
  }
  else
  {
    QList<QTableWidgetSelectionRange>::iterator it;
    for ( it = ranges.begin(); it != ranges.end(); ++it )
    {
      for ( int row = it->topRow(); row <= it->bottomRow(); row++ )
      {
        std::string entry = mySmpWidget->tableWidget->item( row, ENTRY_COL )->text().toStdString();
        mySizeMapsToRemove.push_back(entry);
        MESSAGE("ADDING entry : "<<entry<<"to the Size Maps to remove")
      }
      mySmpWidget->tableWidget->model()->removeRows(it->topRow(), it->rowCount());
    }
  }
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
#ifndef WIN32
  myStdWidget->myHexoticNbProc->setValue( data.myHexoticNbProc );
#endif
  myStdWidget->myHexoticWorkingDir->setText( data.myHexoticWorkingDir );

  myStdWidget->myHexoticVerbosity->setValue( data.myHexoticVerbosity );

  myStdWidget->myHexoticMaxMemory->setValue( data.myHexoticMaxMemory );

  myStdWidget->myHexoticSdMode->setCurrentIndex(data.myHexoticSdMode);
  
  myAdvWidget->SetCustomOptions(data.myTextOptions);

  HexoticPlugin_Hypothesis::THexoticSizeMaps::const_iterator it = data.mySizeMaps.begin();
  for ( int row = 0; it != data.mySizeMaps.end(); it++, row++ )
  {
    std::string entry = it->first;
    double size = it->second;
    GEOM::GEOM_Object_var anObject = entryToObject( entry );
    std::string shapeName = anObject->GetName();

    MESSAGE(" Insert local size, entry : "<<entry<<", size : "<<size<<", at row : "<<row) 
    insertLocalSizeInWidget( entry, shapeName, size , row );
  }

  myVLWidget->myNbLayers->setCleared(data.myNbLayers == 0);
  if (data.myNbLayers == 0)
    myVLWidget->myNbLayers->setText("");
  else
    myVLWidget->myNbLayers->setValue( data.myNbLayers );

  myVLWidget->myFirstLayerSize->setCleared(data.myFirstLayerSize == 0);
  if (data.myFirstLayerSize == 0)
    myVLWidget->myFirstLayerSize->setText("");
  else
    myVLWidget->myFirstLayerSize->setValue( data.myFirstLayerSize );

  myVLWidget->myDirection->setCurrentIndex( data.myDirection ? 0 : 1 );
  myVLWidget->myGrowth->setCleared(data.myGrowth == 0);
  if (data.myGrowth == 0)
    myVLWidget->myGrowth->setText("");
  else
    myVLWidget->myGrowth->setValue( data.myGrowth );

  std::vector<int> vector = data.myFacesWithLayers;
  SMESH::long_array_var aVec = new SMESH::long_array;
  aVec->length(vector.size());
  for (size_t i = 0; i < vector.size(); i++)
    aVec[i]=vector.at(i);
  myVLWidget->myFacesWithLayers->SetListOfIDs(aVec);
  vector = data.myImprintedFaces;
  aVec = new SMESH::long_array;
  aVec->length(vector.size());
  for (size_t i = 0; i < vector.size(); i++)
    aVec[i]=vector.at(i);
  myVLWidget->myImprintedFaces->SetListOfIDs(aVec);


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
  valStr += tr("Hexotic_TEXT_OPTIONS") + " = " + data.myTextOptions + ";";

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
  valStr += tr("Hexotic_TEXT_OPTIONS") + " = " + data.myTextOptions + "; ";

  valStr += tr("Hexotic_NB_LAYERS") + " = " + QString::number(data.myNbLayers) + ";";
  valStr += tr("Hexotic_FIRST_LAYER_SIZE") + " = " + QString::number(data.myFirstLayerSize) + ";";
  valStr += tr("Hexotic_DIRECTION") + " = " + QString::number(data.myDirection) + ";";
  valStr += tr("Hexotic_GROWTH") + " = " + QString::number(data.myGrowth) + ";";

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
  h_data.myTextOptions = h->GetAdvancedOption();
  
  // Size maps
  HexoticPlugin::HexoticPluginSizeMapsList_var sizeMaps = h->GetSizeMaps();
  for ( CORBA::ULong i = 0 ; i < sizeMaps->length() ; i++) 
  {
    HexoticPlugin::HexoticPluginSizeMap aSizeMap = sizeMaps[i];
    std::string entry = CORBA::string_dup(aSizeMap.entry.in());
    double size = aSizeMap.size;
    h_data.mySizeMaps[ entry ] = size;
    MESSAGE("READING Size map : entry "<<entry<<" size : "<<size)
  }
  
  // Viscous layers
  h_data.myNbLayers = h->GetNbLayers();
  h_data.myFirstLayerSize = h->GetFirstLayerSize();
  h_data.myDirection = h->GetDirection();
  h_data.myGrowth = h->GetGrowth();
  SMESH::long_array_var vector = h->GetFacesWithLayers();
  for ( CORBA::ULong i = 0; i < vector->length(); i++)
    h_data.myFacesWithLayers.push_back(vector[i]);
  vector = h->GetImprintedFaces();
  for ( CORBA::ULong i = 0; i < vector->length(); i++)
    h_data.myImprintedFaces.push_back(vector[i]);

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
    h->SetAdvancedOption( h_data.myTextOptions.toLatin1().constData() );
    
    HexoticPlugin_Hypothesis::THexoticSizeMaps::const_iterator it;
    
    for ( it =  h_data.mySizeMaps.begin(); it !=  h_data.mySizeMaps.end(); it++ )
    {
      h->SetSizeMapEntry( it->first.c_str(), it->second );
      MESSAGE("STORING Size map : entry "<<it->first.c_str()<<" size : "<<it->second)
    }
    std::vector< std::string >::const_iterator entry_it;
    for ( entry_it = mySizeMapsToRemove.begin(); entry_it!= mySizeMapsToRemove.end(); entry_it++ )
    {
      h->UnsetSizeMapEntry(entry_it->c_str());
    }

    // Viscous layers
    h->SetNbLayers( h_data.myNbLayers );
    h->SetFirstLayerSize( h_data.myFirstLayerSize );
    h->SetDirection( h_data.myDirection );
    h->SetGrowth( h_data.myGrowth );
    
    std::vector<int> vector = h_data.myFacesWithLayers;
    SMESH::long_array_var aVec = new SMESH::long_array;
    aVec->length(vector.size());
    for ( size_t i = 0; i < vector.size(); i++)
      aVec[i]=vector.at(i);
    h->SetFacesWithLayers( aVec );
    
    vector = h_data.myImprintedFaces;
    aVec = new SMESH::long_array;
    aVec->length(vector.size());
    for ( size_t i = 0; i < vector.size(); i++)
      aVec[i]=vector.at(i);
    h->SetImprintedFaces( aVec );
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
#ifndef WIN32
  h_data.myHexoticNbProc = myStdWidget->myHexoticNbProc->value();
#endif
  h_data.myHexoticWorkingDir = myStdWidget->myHexoticWorkingDir->text();
  h_data.myHexoticVerbosity = myStdWidget->myHexoticVerbosity->value();
  h_data.myHexoticMaxMemory = myStdWidget->myHexoticMaxMemory->value();
  h_data.myHexoticSdMode = myStdWidget->myHexoticSdMode->currentIndex();
  h_data.myTextOptions = myAdvWidget->GetCustomOptions();

  h_data.myMinSize = myStdWidget->myMinSize->text().isEmpty() ? 0.0 : myStdWidget->myMinSize->value();
  h_data.myMaxSize = myStdWidget->myMaxSize->text().isEmpty() ? 0.0 : myStdWidget->myMaxSize->value();
  h_data.myHexesMinLevel = myStdWidget->myHexesMinLevel->text().isEmpty() ? 0 : myStdWidget->myHexesMinLevel->value();
  h_data.myHexesMaxLevel = myStdWidget->myHexesMaxLevel->text().isEmpty() ? 0 : myStdWidget->myHexesMaxLevel->value();
  h_data.myHexoticSharpAngleThreshold = myStdWidget->myHexoticSharpAngleThreshold->text().isEmpty() ? 0 : myStdWidget->myHexoticSharpAngleThreshold->value();

  // Size maps reading
  bool ok = readSizeMapsFromWidgets( h_data );
  if ( !ok )
    return false;
  
  h_data.myNbLayers = myVLWidget->myNbLayers->text().isEmpty() ? 0.0 : myVLWidget->myNbLayers->value();
  h_data.myFirstLayerSize = myVLWidget->myFirstLayerSize->text().isEmpty() ? 0.0 : myVLWidget->myFirstLayerSize->value();
  h_data.myDirection = myVLWidget->myDirection->currentIndex() == 0 ? true : false;
  h_data.myGrowth = myVLWidget->myGrowth->text().isEmpty() ? 0.0 : myVLWidget->myGrowth->value();
  SMESH::long_array_var ids = myVLWidget->myFacesWithLayers->GetListOfIDs();
  for ( CORBA::ULong i = 0; i < ids->length(); i++)
    h_data.myFacesWithLayers.push_back( ids[i] );
  ids = myVLWidget->myImprintedFaces->GetListOfIDs();
  for ( CORBA::ULong i = 0; i < ids->length(); i++)
    h_data.myImprintedFaces.push_back( ids[i] );

  printData(h_data);

  return true;
}

bool HexoticPluginGUI_HypothesisCreator::readSizeMapsFromWidgets( HexoticHypothesisData& h_data ) const
{
  int rowCount = mySmpWidget->tableWidget->rowCount();
  for ( int row = 0; row <  rowCount; row++ )
  {
    std::string entry     = mySmpWidget->tableWidget->item( row, ENTRY_COL )->text().toStdString();
    QVariant size_variant = mySmpWidget->tableWidget->item( row, SIZE_COL )->data(Qt::DisplayRole);
    
    // Convert the size to double
    bool ok = false;
    double size = size_variant.toDouble(&ok);
    if (!ok)
      return false;
    
    // Set the size maps
    h_data.mySizeMaps[ entry ] = size;
    MESSAGE("READING Size map from WIDGET: entry "<<entry<<" size : "<<size)
  }
  return true;
}

GEOM::GEOM_Object_var HexoticPluginGUI_HypothesisCreator::entryToObject( std::string entry) const
{
  SMESH_Gen_i* smeshGen_i = SMESH_Gen_i::GetSMESHGen();
  SALOMEDS::Study_var myStudy = smeshGen_i->GetCurrentStudy();
  GEOM::GEOM_Object_var aGeomObj;
  SALOMEDS::SObject_var aSObj = myStudy->FindObjectID( entry.c_str() );
  if (!aSObj->_is_nil()) {
    CORBA::Object_var obj = aSObj->GetObject();
    aGeomObj = GEOM::GEOM_Object::_narrow(obj);
    aSObj->UnRegister();
  }
  return aGeomObj;
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

void HexoticPluginGUI_HypothesisCreator::onTabChanged(int i)
{
  myVLWidget->myFacesWithLayers->ShowPreview( i == VL_TAB );
  myVLWidget->myImprintedFaces->ShowPreview( false );
}
