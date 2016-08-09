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
// File   : HexoticPlugin_Hypothesis.hxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#ifndef _HexoticPlugin_Hypothesis_HXX_
#define _HexoticPlugin_Hypothesis_HXX_

#include "HexoticPlugin_Defs.hxx"

#include "SMESH_Hypothesis.hxx"
#include "Utils_SALOME_Exception.hxx"

#include <map>
#include <vector>

// class HexoticSizeMap
// {
// public:
//   SizeMap(const TopoDS_Shape& theShape, double theSize)
//   {
//     shape=theShape;
//     size=theSize;
//   };
//   TopoDS_Shape shape;
//   double size;
// }
//  Parameters for work of MG-Hexa
//

class HEXOTICPLUGIN_EXPORT HexoticPlugin_Hypothesis: public SMESH_Hypothesis
{
public:

  HexoticPlugin_Hypothesis(int hypId, int studyId, SMESH_Gen* gen);

  static const char* GetHypType() { return "MG-Hexa Parameters"; }

  void SetHexesMinLevel(int theVal);
  int GetHexesMinLevel() const { return _hexesMinLevel; }

  void SetHexesMaxLevel(int theVal);
  int GetHexesMaxLevel() const { return _hexesMaxLevel; }

  void SetMinSize(double theVal);
  double GetMinSize() const { return _minSize; }

  void SetMaxSize(double theVal);
  double GetMaxSize() const { return _maxSize; }

  void SetHexoticIgnoreRidges(bool theVal);
  bool GetHexoticIgnoreRidges() const { return _hexoticIgnoreRidges; }

  void SetHexoticInvalidElements(bool theVal);
  bool GetHexoticInvalidElements() const { return _hexoticInvalidElements; }
   
  void SetHexoticSharpAngleThreshold(double theVal);
  double GetHexoticSharpAngleThreshold() const { return _hexoticSharpAngleThreshold; }
   
  void SetHexoticNbProc(int theVal);
  int GetHexoticNbProc() const { return _hexoticNbProc; }
  
  void SetHexoticWorkingDirectory(const std::string& path);
  std::string GetHexoticWorkingDirectory() const { return _hexoticWorkingDirectory; }

  void SetHexoticSdMode(int theVal);
  int GetHexoticSdMode() const { return _hexoticSdMode; }

  void SetHexoticVerbosity(int theVal);
  int GetHexoticVerbosity() const { return _hexoticVerbosity; }

  void SetHexoticMaxMemory(int theVal);
  int GetHexoticMaxMemory() const { return _hexoticMaxMemory; }
  
  void SetAdvancedOption(const std::string& theOptions);
  std::string GetAdvancedOption() const { return _textOptions; }
  void SetTextOptions(const std::string& theOptions); // obsolete
  std::string GetTextOptions() const { return _textOptions; }

  // Size Maps
  typedef std::map<std::string,double> THexoticSizeMaps;
  
  // For the GUI HexoticPluginGUI_HypothesisCreator::storeParamToHypo
  THexoticSizeMaps GetSizeMaps() const { return _sizeMaps; }; 
  
  // Add one size map to the collection of size maps (user interface)
  bool AddSizeMap(std::string theEntry, double theSize);
  bool UnsetSizeMap(std::string theEntry);

  void SetNbLayers(int theVal);
  int GetNbLayers() const { return _nbLayers; }

  void SetFirstLayerSize(double theVal);
  double GetFirstLayerSize() const { return _firstLayerSize; }

  void SetDirection(bool theVal);
  bool GetDirection() const { return _direction; }

  void SetGrowth(double theVal);
  double GetGrowth() const { return _growth; }

  bool SetFacesWithLayers(const std::vector<int>& theVal);
  std::vector<int> GetFacesWithLayers() const { return _facesWithLayers; }

  bool SetImprintedFaces(const std::vector<int>& theVal);
  std::vector<int> GetImprintedFaces() const { return _imprintedFaces; }

  // the parameters default values 
  static int GetDefaultHexesMinLevel();
  static int GetDefaultHexesMaxLevel();
  static double GetDefaultMinSize();
  static double GetDefaultMaxSize();
  static bool GetDefaultHexoticIgnoreRidges();
  static bool GetDefaultHexoticInvalidElements();
  static double GetDefaultHexoticSharpAngleThreshold();
  static int GetDefaultHexoticNbProc();
  static std::string GetDefaultHexoticWorkingDirectory();
  static int GetDefaultHexoticSdMode();
  static int GetDefaultHexoticVerbosity();
  static int GetDefaultHexoticMaxMemory();
  static std::string GetDefaultTextOptions();
  static THexoticSizeMaps GetDefaultHexoticSizeMaps();
  static int GetDefaultNbLayers();
  static double GetDefaultFirstLayerSize();
  static bool GetDefaultDirection();
  static double GetDefaultGrowth();
  static std::vector<int> GetDefaultFacesWithLayers();
  static std::vector<int> GetDefaultImprintedFaces();

  // Persistence
  virtual std::ostream& SaveTo(std::ostream& save);
  virtual std::istream& LoadFrom(std::istream& load);
  friend  std::ostream& operator <<(std::ostream& save, HexoticPlugin_Hypothesis& hyp);
  friend  std::istream& operator >>(std::istream& load, HexoticPlugin_Hypothesis& hyp);

  /*!
   * \brief Does nothing
   * \param theMesh - the built mesh
   * \param theShape - the geometry of interest
   * \retval bool - always false
   */
  virtual bool SetParametersByMesh(const SMESH_Mesh* theMesh, const TopoDS_Shape& theShape);

  /*!
   * \brief Initialize my parameter values by default parameters.
   *  \retval bool - true if parameter values have been successfully defined
   */
  virtual bool SetParametersByDefaults(const TDefaults& dflts, const SMESH_Mesh* theMesh=0);

private:
  int    _hexesMinLevel;
  int    _hexesMaxLevel;
  double _minSize;
  double _maxSize;
  bool   _hexoticIgnoreRidges;
  bool   _hexoticInvalidElements;
  double _hexoticSharpAngleThreshold;
  int    _hexoticNbProc;
  int    _hexoticSdMode;
  int    _hexoticVerbosity;
  int    _hexoticMaxMemory;
  std::string _textOptions;
  THexoticSizeMaps _sizeMaps;
  std::string _hexoticWorkingDirectory;
  int    _nbLayers;
  double _firstLayerSize;
  bool   _direction;
  double _growth;
  std::vector<int> _facesWithLayers;
  std::vector<int> _imprintedFaces;
};

#endif
