// Copyright (C) 2004-2024  CEA, EDF
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

#ifndef __MG_Hexotic_IO_HXX__
#define __MG_Hexotic_IO_HXX__

extern "C"
{
#include "libmesh5.h"
}
#include <string>
#include <set>

/*!
 * \brief Class providing a transparent switch between MG_Hexotic usage as
 *        a library and as an executable. API of libmesh5 inherited.
 */
class MG_Hexotic_API
{
public:

  MG_Hexotic_API( volatile bool& cancelled_flag, double& progress );
  ~MG_Hexotic_API();

  bool IsLibrary();
  bool IsExecutable() { return !IsLibrary(); }
  void SetUseExecutable();
  void SetInputFile( const std::string mesh2DFile );
  void SetIsInputMesh(bool isMeshIn) { _isMesh = isMeshIn; }

  // IN to MESHGEMS
  int  GmfOpenMesh(const char* theFile, int rdOrWr, int ver, int dim);
  void GmfSetKwd(int iMesh, GmfKwdCod what, int nb );
  void GmfSetLin(int iMesh, GmfKwdCod what, double x, double y, double z, int domain);
  void GmfSetKwd(int iMesh, GmfKwdCod what, int nbNodes, int dummy, int type[] ); // sol type
  void GmfSetLin(int iMesh, GmfKwdCod what, double vals[]); // sol
  void GmfSetLin(int iMesh, GmfKwdCod what, int node1, int node2, int domain ); // edge
  void GmfSetLin(int iMesh, GmfKwdCod what, int id ); // required
  void GmfSetLin(int iMesh, GmfKwdCod what, int node1, int node2, int node3, int domain ); // tria

  bool Compute( const std::string& cmdLine, std::string& errStr );

  // OUT from MESHGEMS
  int  GmfOpenMesh(const char* theFile, int rdOrWr, int * ver, int * dim);
  int  GmfStatKwd( int iMesh, GmfKwdCod what );
  void GmfGotoKwd( int iMesh, GmfKwdCod what );
  void GmfGetLin( int iMesh, GmfKwdCod what, int* nbNodes, int* faceInd, int* ori, int* domain, int dummy );
  void GmfGetLin(int iMesh, GmfKwdCod what, float* x, float* y, float *z, int* domain );
  void GmfGetLin(int iMesh, GmfKwdCod what, double* x, double* y, double *z, int* domain );
  void GmfGetLin(int iMesh, GmfKwdCod what, int* node );
  void GmfGetLin(int iMesh, GmfKwdCod what, int* node1, int* node2, int* domain );
  void GmfGetLin(int iMesh, GmfKwdCod what, int* node1, int* node2, int* node3, int* domain );
  void GmfGetLin(int iMesh, GmfKwdCod what, int* node1, int* node2, int* node3, int* node4, int* domain );
  void GmfGetLin(int iMesh, GmfKwdCod what, int* node1, int* node2, int* node3, int* node4, int* node5, int* node6, int* node7, int* node8, int* domain );
  void GmfCloseMesh( int iMesh );

  void SetLogFile( const std::string& logFileName ) { _logFile = logFileName; }
  bool HasLog();
  std::string GetLog();


  struct LibData;

private:

  bool          _useLib;
  LibData*      _libData;
  std::set<int> _openFiles;
  std::string   _logFile;

  // count mesh entities for MG license key generation
  bool          _isMesh;
  int           _nbNodes;
  int           _nbEdges;
  int           _nbFaces;
  int           _nbVolumes;
};

#endif
