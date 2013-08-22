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

// File   : DlgRef.h
// Author : Vadim SANDLER, Open CASCADE S.A.S. (vadim.sandler@opencascade.com)

#ifndef HEXOTICPLUGINGUI_H
#define HEXOTICPLUGINGUI_H

class QPixmap;

enum {
  SD_MODE_1,
  SD_MODE_2,
  SD_MODE_3,
  SD_MODE_4
};

//////////////////////////////////////////
// HexoticPluginGUI_StdWidget
//////////////////////////////////////////

#include "ui_HexoticPluginGUI_StdWidget_QTD.h"
#include "ui_HexoticPluginGUI_SizeMapsWidget_QTD.h"
#include "HexoticPluginGUI.h"

class HEXOTICPLUGIN_GUI_EXPORT HexoticPluginGUI_StdWidget : public QWidget,
                                            public Ui::HexoticPluginGUI_StdWidget_QTD
{
  Q_OBJECT

public:
  HexoticPluginGUI_StdWidget( QWidget* = 0, Qt::WindowFlags = 0 );
  ~HexoticPluginGUI_StdWidget();

public slots:
  void onDirBtnClicked();
  void onSdModeSelected(int sdMode);

public:
  QPixmap imageSdMode;
};

class HEXOTICPLUGIN_GUI_EXPORT HexoticPluginGUI_SizeMapsWidget : public QWidget,
                                            public Ui::HexoticPluginGUI_SizeMapsWidget_QTD
{
  Q_OBJECT

public:
  HexoticPluginGUI_SizeMapsWidget( QWidget* = 0, Qt::WindowFlags = 0 );
  ~HexoticPluginGUI_SizeMapsWidget();
};


#endif
