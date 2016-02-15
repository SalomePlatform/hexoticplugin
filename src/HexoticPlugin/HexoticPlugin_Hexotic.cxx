// Copyright (C) 2007-2015  CEA/DEN, EDF R&D
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
// File   : HexoticPlugin_Hexotic.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hexotic.hxx"
#include "HexoticPlugin_Hypothesis.hxx"

#include "utilities.h"

#ifndef WIN32
#include <sys/sysinfo.h>
#else
#include <errno.h>
#include <process.h>
#endif

#ifdef _DEBUG_
#define DUMP(txt) \
//  cout << txt
#else
#define DUMP(txt)
#endif

#include <SMESHDS_Mesh.hxx>
#include <SMESHDS_GroupBase.hxx>
#include <SMESH_ComputeError.hxx>
#include <SMESH_File.hxx>
#include <SMESH_Gen.hxx>
#include <SMESH_HypoFilter.hxx>
#include <SMESH_MesherHelper.hxx>
#include <SMESH_subMesh.hxx>

#include <list>
#include <cstdlib>
#include <iostream>

#include <Standard_ProgramError.hxx>

#include <BRep_Tool.hxx>
#include <BRepBndLib.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <OSD_File.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>
#include <gp_Lin.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <gp_Ax3.hxx>
#include <GeomAdaptor_Curve.hxx>

#include <Basics_Utils.hxx>
#include "GEOMImpl_Types.hxx"
#include "GEOM_wrap.hxx"

static void removeFile( const TCollection_AsciiString& fileName )
{
  try {
    OSD_File( fileName ).Remove();
  }
  catch ( Standard_ProgramError ) {
    MESSAGE("Can't remove file: " << fileName.ToCString() << " ; file does not exist or permission denied");
  }
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

HexoticPlugin_Hexotic::HexoticPlugin_Hexotic(int hypId, int studyId, SMESH_Gen* gen)
  : SMESH_3D_Algo(hypId, studyId, gen)
{
  MESSAGE("HexoticPlugin_Hexotic::HexoticPlugin_Hexotic");
  _name = "MG-Hexa";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
//   _onlyUnaryInput = false;
  _requireShape = false;
  _iShape=0;
  _nbShape=0;
  _hexoticFilesKept=false;
  _compatibleHypothesis.push_back( HexoticPlugin_Hypothesis::GetHypType() );
#ifdef WITH_BLSURFPLUGIN
  _blsurfHypo = NULL;
#endif
  _compute_canceled = false;
  
  // Copy of what is done in BLSURFPLugin TODO : share the code
  smeshGen_i = SMESH_Gen_i::GetSMESHGen();
  CORBA::Object_var anObject = smeshGen_i->GetNS()->Resolve("/myStudyManager");
  SALOMEDS::StudyManager_var aStudyMgr = SALOMEDS::StudyManager::_narrow(anObject);
  
  myStudy = NULL;
  myStudy = aStudyMgr->GetStudyByID(_studyId);
  if (myStudy)
    MESSAGE("myStudy->StudyId() = " << myStudy->StudyId());
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

HexoticPlugin_Hexotic::~HexoticPlugin_Hexotic()
{
  MESSAGE("HexoticPlugin_Hexotic::~HexoticPlugin_Hexotic");
}


#ifdef WITH_BLSURFPLUGIN
bool HexoticPlugin_Hexotic::CheckBLSURFHypothesis( SMESH_Mesh&         aMesh,
                                                   const TopoDS_Shape& aShape )
{
  // MESSAGE("HexoticPlugin_Hexotic::CheckBLSURFHypothesis");
  _blsurfHypo = NULL;

  std::list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  // If a BLSURF hypothesis is applied, get it
  SMESH_HypoFilter blsurfFilter;
  blsurfFilter.Init( blsurfFilter.HasName( BLSURFPlugin_Hypothesis::GetHypType() ));
  std::list<const SMESHDS_Hypothesis *> appliedHyps;
  aMesh.GetHypotheses( aShape, blsurfFilter, appliedHyps, false );

  if ( appliedHyps.size() > 0 ) {
    itl = appliedHyps.begin();
    theHyp = (*itl); // use only the first hypothesis
    std::string hypName = theHyp->GetName();
    if (hypName == BLSURFPlugin_Hypothesis::GetHypType()) {
      _blsurfHypo = static_cast<const BLSURFPlugin_Hypothesis*> (theHyp);
      ASSERT(_blsurfHypo);
      return true;
    }
  }
  return false;
}
#endif

//=============================================================================
/*!
 *  
 */
//=============================================================================

bool HexoticPlugin_Hexotic::CheckHypothesis( SMESH_Mesh&                          aMesh,
                                             const TopoDS_Shape&                  aShape,
                                             SMESH_Hypothesis::Hypothesis_Status& aStatus )
{
  // MESSAGE("HexoticPlugin_Hexotic::CheckHypothesis");
  _hypothesis = NULL;

  std::list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const std::list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape, false);
  int nbHyp = hyps.size();
  if (!nbHyp) {
    aStatus = SMESH_Hypothesis::HYP_OK;
    return true;  // can work with no hypothesis
  }

  itl = hyps.begin();
  theHyp = (*itl); // use only the first hypothesis

  std::string hypName = theHyp->GetName();
  if (hypName == HexoticPlugin_Hypothesis::GetHypType() ) {
    _hypothesis = static_cast<const HexoticPlugin_Hypothesis*> (theHyp);
    ASSERT(_hypothesis);
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;
  
#ifdef WITH_BLSURFPLUGIN
  CheckBLSURFHypothesis(aMesh, aShape);
#endif
  
  return aStatus == SMESH_Hypothesis::HYP_OK;
}

//=======================================================================
//function : findShape
//purpose  :
//=======================================================================

static TopoDS_Shape findShape(SMDS_MeshNode**     t_Node,
                              TopoDS_Shape        aShape,
                              const TopoDS_Shape* t_Shape,
                              double**            t_Box,
                              const int           nShape)
{
  double pntCoor[3];
  int iShape, nbNode = 8;

  for ( int i=0; i<3; i++ ) {
    pntCoor[i] = 0;
    for ( int j=0; j<nbNode; j++ ) {
      if ( i == 0) pntCoor[i] += t_Node[j]->X();
      if ( i == 1) pntCoor[i] += t_Node[j]->Y();
      if ( i == 2) pntCoor[i] += t_Node[j]->Z();
    }
    pntCoor[i] /= nbNode;
  }
  gp_Pnt aPnt(pntCoor[0], pntCoor[1], pntCoor[2]);

  if ( aShape.IsNull() ) aShape = t_Shape[0];
  BRepClass3d_SolidClassifier SC (aShape, aPnt, Precision::Confusion());
  if ( !(SC.State() == TopAbs_IN) ) {
    aShape.Nullify();
    for (iShape = 0; iShape < nShape && aShape.IsNull(); iShape++) {
      if ( !( pntCoor[0] < t_Box[iShape][0] || t_Box[iShape][1] < pntCoor[0] ||
              pntCoor[1] < t_Box[iShape][2] || t_Box[iShape][3] < pntCoor[1] ||
              pntCoor[2] < t_Box[iShape][4] || t_Box[iShape][5] < pntCoor[2]) ) {
        BRepClass3d_SolidClassifier SC (t_Shape[iShape], aPnt, Precision::Confusion());
        if (SC.State() == TopAbs_IN)
          aShape = t_Shape[iShape];
      }
    }
  }
  return aShape;
}

//=======================================================================
//function : getNbShape
//purpose  :
//=======================================================================

static int getNbShape(std::string aFile, std::string aString, int defaultValue=0) {
  int number = defaultValue;
  std::string aLine;
  std::ifstream file(aFile.c_str());
  while ( !file.eof() ) {
    getline( file, aLine);
    if ( aLine == aString ) {
      getline( file, aLine);
      std::istringstream stringFlux( aLine );
      stringFlux >> number;
      number = ( number + defaultValue + std::abs(number - defaultValue) ) / 2;
      break;
    }
  }
  file.close();
  return number;
}

//=======================================================================
//function : countShape
//purpose  :
//=======================================================================

template < class Mesh, class Shape >
static int countShape( Mesh* mesh, Shape shape ) {
  TopExp_Explorer expShape ( mesh->ShapeToMesh(), shape );
  TopTools_MapOfShape mapShape;
  int nbShape = 0;
  for ( ; expShape.More(); expShape.Next() ) {
    if (mapShape.Add(expShape.Current())) {
      nbShape++;
    }
  }
  return nbShape;
}

//=======================================================================
//function : getShape
//purpose  :
//=======================================================================

template < class Mesh, class Shape, class Tab >
void getShape(Mesh* mesh, Shape shape, Tab *t_Shape) {
  TopExp_Explorer expShape ( mesh->ShapeToMesh(), shape );
  TopTools_MapOfShape mapShape;
  for ( int i=0; expShape.More(); expShape.Next() ) {
    if (mapShape.Add(expShape.Current())) {
      t_Shape[i] = expShape.Current();
      i++;
    }
  }
  return;
}

//=======================================================================
//function : printWarning
//purpose  :
//=======================================================================

static void printWarning(const int nbExpected, std::string aString, const int nbFound) {
  cout << std::endl;
  cout << "WARNING : " << nbExpected << " " << aString << " expected, MG-Hexa has found " << nbFound << std::endl;
  cout << "=======" << std::endl;
  cout << std::endl;
  return;
}

//=======================================================================
//function : removeHexoticFiles
//purpose  :
//=======================================================================

static void removeHexoticFiles(TCollection_AsciiString file_In, TCollection_AsciiString file_Out) {
  removeFile( file_In );
  removeFile( file_Out );
}

//=======================================================================
//function : splitQuads
//purpose  : splits all quadrangles into triangles
//=======================================================================

static void splitQuads(SMESH_Mesh& aMesh)
{
  SMESH_MeshEditor spliter( &aMesh );

  TIDSortedElemSet elems;
  SMDS_ElemIteratorPtr eIt = aMesh.GetMeshDS()->elementsIterator();
  while( eIt->more() )
    elems.insert( elems.end(), eIt->next() );
  
  spliter.QuadToTri ( elems, /*the13Diag=*/true);
}

//=======================================================================
//function : readResult
//purpose  : Read GMF file in case of a mesh with geometry
//=======================================================================

static bool readResult(std::string         theFile,
                       HexoticPlugin_Hexotic*  theAlgo,
                       SMESHDS_Mesh*       theMesh,
                       const int           nbShape,
                       const TopoDS_Shape* tabShape,
                       double**            tabBox)
{
  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  TopoDS_Shape aShape;
  TopoDS_Vertex aVertex;
  std::string token;
  int EndOfFile = 0, nbElem = 0, nField = 9, nbRef = 0;
  int aHexoticNodeID = 0, shapeID, hexoticShapeID;
  const int IdShapeRef = 2;
  int *tabID, *tabRef, *nodeAssigne;
  bool *tabDummy, hasDummy = false;
  double epsilon = Precision::Confusion();
  std::map <std::string,int> mapField;
  SMDS_MeshNode** HexoticNode;
  TopoDS_Shape *tabCorner, *tabEdge;

  const int nbDomains = countShape( theMesh, TopAbs_SHELL );
  const int holeID = -1;

  // tabID    = new int[nbShape];
  tabID    = new int[nbDomains];
  tabRef   = new int[nField];
  tabDummy = new bool[nField];

  for (int i=0; i<nbDomains; i++)
    tabID[i] = 0;
  if ( nbDomains == 1 )
    tabID[0] = theMesh->ShapeToIndex( tabShape[0] );

  mapField["MeshVersionFormatted"] = 0; tabRef[0] = 0; tabDummy[0] = false;
  mapField["Dimension"]            = 1; tabRef[1] = 0; tabDummy[1] = false;
  mapField["Vertices"]             = 2; tabRef[2] = 3; tabDummy[2] = true;
  mapField["Corners"]              = 3; tabRef[3] = 1; tabDummy[3] = false;
  mapField["Edges"]                = 4; tabRef[4] = 2; tabDummy[4] = true;
  mapField["Ridges"]               = 5; tabRef[5] = 1; tabDummy[5] = false;
  mapField["Quadrilaterals"]       = 6; tabRef[6] = 4; tabDummy[6] = true;
  mapField["Hexahedra"]            = 7; tabRef[7] = 8; tabDummy[7] = true;
  mapField["End"]                  = 8; tabRef[8] = 0; tabDummy[0] = false;

  SMDS_NodeIteratorPtr itOnHexoticInputNode = theMesh->nodesIterator();
  while ( itOnHexoticInputNode->more() )
    theMesh->RemoveNode( itOnHexoticInputNode->next() );

  int nbVertices   = getNbShape(theFile, "Vertices");
  int nbCorners    = getNbShape(theFile, "Corners", countShape( theMesh, TopAbs_VERTEX ));
  int nbShapeEdge  = countShape( theMesh, TopAbs_EDGE );

  tabCorner   = new TopoDS_Shape[ nbCorners ];
  tabEdge     = new TopoDS_Shape[ nbShapeEdge ];
  nodeAssigne = new int[ nbVertices + 1 ];
  HexoticNode = new SMDS_MeshNode*[ nbVertices + 1 ];

  getShape(theMesh, TopAbs_VERTEX, tabCorner);
  getShape(theMesh, TopAbs_EDGE,   tabEdge);

  MESSAGE("Read " << theFile << " file");
  std::ifstream fileRes(theFile.c_str());
  ASSERT(fileRes);

  while ( EndOfFile == 0  ) {
    int dummy;
    fileRes >> token;

    if (mapField.count(token)) {
      nField   = mapField[token];
      nbRef    = tabRef[nField];
      hasDummy = tabDummy[nField];
    }
    else {
      nField = -1;
      nbRef = 0;
    }

    nbElem = 0;
    if ( nField < int(mapField.size() - 1) && nField >= 0 )
      fileRes >> nbElem;

    switch (nField) {
      case 0: { // "MeshVersionFormatted"
        MESSAGE(token << " " << nbElem);
        break;
      }
      case 1: { // "Dimension"
        MESSAGE("Mesh dimension " << nbElem << "D");
        break;
      }
      case 2: { // "Vertices"
        MESSAGE("Read " << nbElem << " " << token);
        int aHexoticID;
        double *coord;
        SMDS_MeshNode * aHexoticNode;

        coord = new double[nbRef];
        for ( int iElem = 0; iElem < nbElem; iElem++ ) {
          if(theAlgo->computeCanceled())
            {
              return false;
            }
          aHexoticID = iElem + 1;
          for ( int iCoord = 0; iCoord < 3; iCoord++ )
            fileRes >> coord[ iCoord ];
          fileRes >> dummy;
          aHexoticNode = theMesh->AddNode(coord[0], coord[1], coord[2]);
          HexoticNode[ aHexoticID ] = aHexoticNode;
          nodeAssigne[ aHexoticID ] = 0;
        }
        delete [] coord;
        break;
      }
      case 3: // "Corners"
      case 4: // "Edges"
      case 5: // "Ridges"
      case 6: // "Quadrilaterals"
      case 7: { // "Hexahedra"
        MESSAGE("Read " << nbElem << " " << token);
        SMDS_MeshNode** node;
        int nodeDim, *nodeID;
        SMDS_MeshElement * aHexoticElement = 0;

        node   = new SMDS_MeshNode*[ nbRef ];
        nodeID = new int[ nbRef ];
        for ( int iElem = 0; iElem < nbElem; iElem++ ) {
          if(theAlgo->computeCanceled())
            {
              return false;
            }
          for ( int iRef = 0; iRef < nbRef; iRef++ ) {
            fileRes >> aHexoticNodeID;                          // read nbRef aHexoticNodeID
            node[ iRef ]   = HexoticNode[ aHexoticNodeID ];
            nodeID[ iRef ] = aHexoticNodeID;
          }
          if ( hasDummy )
            fileRes >> dummy;
          switch (nField) {
            case 3: { // "Corners"
              nodeDim = 1;
              gp_Pnt HexoticPnt ( node[0]->X(), node[0]->Y(), node[0]->Z() );
              for ( int i=0; i<nbElem; i++ ) {
                aVertex = TopoDS::Vertex( tabCorner[i] );
                gp_Pnt aPnt = BRep_Tool::Pnt( aVertex );
                if ( aPnt.Distance( HexoticPnt ) < epsilon )
                  break;
              }
              if ( nodeAssigne[ nodeID[0] ] != 0 ) { // because "Edges" go before "Corners"
                theMesh->UnSetNodeOnShape( node[0] );
                nodeAssigne[ nodeID[0] ] = 0;
              }
              break;
            }
            case 4: { // "Edges"
              nodeDim = 2;
              aHexoticElement = theMesh->AddEdge( node[0], node[1] );
              int iNode = 1;
              if ( nodeAssigne[ nodeID[0] ] == 0 || nodeAssigne[ nodeID[0] ] == 2 )
                iNode = 0;
              shapeID = dummy;
              break;
            }
            case 5: { // "Ridges"
              break;
            }
            case 6: { // "Quadrilaterals"
              nodeDim = 3;
              aHexoticElement = theMesh->AddFace( node[0], node[1], node[2], node[3] );
              shapeID = dummy;
              break;
            }
            case 7: { // "Hexahedra"
              nodeDim = 4;
              if ( nbDomains > 1 ) {
                hexoticShapeID = dummy - IdShapeRef;
                if ( tabID[ hexoticShapeID ] == 0 ) {
                  aShape = findShape(node, aShape, tabShape, tabBox, nbShape);
                  shapeID = aShape.IsNull() ? holeID : theMesh->ShapeToIndex( aShape );
                  tabID[ hexoticShapeID ] = shapeID;
                }
                else
                  shapeID = tabID[ hexoticShapeID ];
                if ( iElem == (nbElem - 1) ) {
                  int shapeAssociated = 0;
                  for ( int i=0; i<nbDomains; i++ ) {
                    if (tabID[i] > 0 )
                      shapeAssociated += 1;
                  }
                  if ( shapeAssociated != nbShape )
                    printWarning(nbShape, "domains", shapeAssociated);
                }
              }
              else {
                shapeID = tabID[0];
              }
              if ( shapeID != holeID )
                aHexoticElement = theMesh->AddVolume( node[0], node[3], node[2], node[1], node[4], node[7], node[6], node[5] );
              break;
            }
          } // switch (nField)

          if ( token != "Ridges" && ( shapeID > 0 || token == "Corners")) {
            for ( int i=0; i<nbRef; i++ ) {
              if ( nodeAssigne[ nodeID[i] ] == 0 ) {
                if      ( token == "Corners" )        theMesh->SetNodeOnVertex( node[0], aVertex );
                else if ( token == "Edges" )          theMesh->SetNodeOnEdge( node[i], shapeID );
                else if ( token == "Quadrilaterals" ) theMesh->SetNodeOnFace( node[i], shapeID );
                else if ( token == "Hexahedra" )      theMesh->SetNodeInVolume( node[i], shapeID );
                nodeAssigne[ nodeID[i] ] = nodeDim;
              }
            }
            if ( token != "Corners" && aHexoticElement )
              theMesh->SetMeshElementOnShape( aHexoticElement, shapeID );
          }
        }
        delete [] node;
        delete [] nodeID;
        break;
      }
      case 8: { // "End"
        EndOfFile = 1;
        MESSAGE("End of " << theFile << " file");
        break;
      }
      default: {
        MESSAGE("Unknown Token: " << token);
      }
    }
  }
  cout << std::endl;

  // remove nodes in holes
  if ( nbDomains > 1 )
  {
    SMESHDS_SubMesh* subMesh;
    for ( int i = 1; i <= nbVertices; ++i )
      if ( HexoticNode[i]->NbInverseElements() == 0 )
      {
        subMesh =  HexoticNode[i]->getshapeId() > 0 ? theMesh->MeshElements(HexoticNode[i]->getshapeId() ) : 0;
        theMesh->RemoveFreeNode( HexoticNode[i], subMesh, /*fromGroups=*/false );
      }
  }
  delete [] tabID;
  delete [] tabRef;
  delete [] tabDummy;
  delete [] tabCorner;
  delete [] tabEdge;
  delete [] nodeAssigne;
  delete [] HexoticNode;
  return true;
}


//=======================================================================
//function : readResult
//purpose  : Read GMF file in case of a mesh w/o geometry
//=======================================================================

static bool readResult(std::string theFile,
                       HexoticPlugin_Hexotic*  theAlgo,
                       SMESH_MesherHelper* theHelper)
{
  SMESHDS_Mesh* theMesh = theHelper->GetMeshDS();

  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  std::string token;
  const int nbField = 9;
  int nField, EndOfFile = 0, nbElem = 0, nbRef = 0;
  int aHexoticNodeID = 0, shapeID;
  int tabRef[nbField], *nodeAssigne;
  bool tabDummy[nbField], hasDummy = false;
  std::map <std::string,int> mapField;
  SMDS_MeshNode** HexoticNode;

  mapField["MeshVersionFormatted"] = 0; tabRef[0] = 0; tabDummy[0] = false;
  mapField["Dimension"]            = 1; tabRef[1] = 0; tabDummy[1] = false;
  mapField["Vertices"]             = 2; tabRef[2] = 3; tabDummy[2] = true;
  mapField["Corners"]              = 3; tabRef[3] = 1; tabDummy[3] = false;
  mapField["Edges"]                = 4; tabRef[4] = 2; tabDummy[4] = true;
  mapField["Ridges"]               = 5; tabRef[5] = 1; tabDummy[5] = false;
  mapField["Quadrilaterals"]       = 6; tabRef[6] = 4; tabDummy[6] = true;
  mapField["Hexahedra"]            = 7; tabRef[7] = 8; tabDummy[7] = true;
  mapField["End"]                  = 8; tabRef[8] = 0; tabDummy[8] = false;

  {
    // theMesh->Clear(); -- this does not remove imported mesh
    SMDS_ElemIteratorPtr eIt = theMesh->elementsIterator();
    while( eIt->more() )
      theMesh->RemoveFreeElement( eIt->next(), /*sm=*/0 );
    SMDS_NodeIteratorPtr nIt = theMesh->nodesIterator();
    while ( nIt->more() )
      theMesh->RemoveFreeNode( nIt->next(), /*sm=*/0 );
  }

  int nbVertices = getNbShape(theFile, "Vertices");
  HexoticNode = new SMDS_MeshNode*[ nbVertices + 1 ];
  nodeAssigne = new int[ nbVertices + 1 ];

  MESSAGE("Read " << theFile << " file");
  std::ifstream fileRes(theFile.c_str());
  ASSERT(fileRes);

  while ( !EndOfFile  )
  {
    int dummy;
    fileRes >> token;

    if (mapField.count(token)) {
      nField   = mapField[token];
      nbRef    = tabRef[nField];
      hasDummy = tabDummy[nField];
    }
    else {
      nField = -1;
      nbRef = 0;
    }

    nbElem = 0;
    if ( nField < int(mapField.size() - 1) && nField >= 0 )
      fileRes >> nbElem;

    switch (nField) {
    case 0: { // "MeshVersionFormatted"
      MESSAGE(token << " " << nbElem);
      break;
    }
    case 1: { // "Dimension"
      MESSAGE("Mesh dimension " << nbElem << "D");
      break;
    }
    case 2: { // "Vertices"
      MESSAGE("Read " << nbElem << " " << token);
      int aHexoticID;
      double coord[3];
      SMDS_MeshNode * aHexoticNode;

      for ( int iElem = 0; iElem < nbElem; iElem++ ) {
        if(theAlgo->computeCanceled())
          {
            return false;
          }
        aHexoticID = iElem + 1;
        for ( int iCoord = 0; iCoord < 3; iCoord++ )
          fileRes >> coord[ iCoord ];
        fileRes >> dummy;
        aHexoticNode = theMesh->AddNode(coord[0], coord[1], coord[2]);
        HexoticNode[ aHexoticID ] = aHexoticNode;
        nodeAssigne[ aHexoticID ] = 0;
      }
      break;
    }
    case 3: // "Corners"
    case 4: // "Edges"
    case 5: // "Ridges"
    case 6: // "Quadrilaterals"
    case 7: { // "Hexahedra"
      MESSAGE("Read " << nbElem << " " << token);
      std::vector< SMDS_MeshNode* > node( nbRef );
      std::vector< int >          nodeID( nbRef );

      for ( int iElem = 0; iElem < nbElem; iElem++ )
      {
        if(theAlgo->computeCanceled())
          {
            return false;
          }
        for ( int iRef = 0; iRef < nbRef; iRef++ )
        {
          fileRes >> aHexoticNodeID;                          // read nbRef aHexoticNodeID
          node  [ iRef ] = HexoticNode[ aHexoticNodeID ];
          nodeID[ iRef ] = aHexoticNodeID;
        }
        if ( hasDummy )
          fileRes >> dummy;
        switch (nField)
        {
        case 4: // "Edges"
          theHelper->AddEdge( node[0], node[1] ); break;
        case 6:  // "Quadrilaterals"
          theMesh->AddFace( node[0], node[1], node[2], node[3] ); break;
        case 7: // "Hexahedra"
          theHelper->AddVolume( node[0], node[3], node[2], node[1],
                                node[4], node[7], node[6], node[5] ); break;
        default: continue;
        }
        if ( nField == 6 )
          for ( int iRef = 0; iRef < nbRef; iRef++ )
            nodeAssigne[ nodeID[ iRef ]] = 1;
      }
      break;
    }
    case 8: { // "End"
      EndOfFile = 1;
      MESSAGE("End of " << theFile << " file");
      break;
    }
    default: {
      MESSAGE("Unknown Token: " << token);
    }
    }
  }
  cout << std::endl;

  shapeID = theHelper->GetSubShapeID();
  for ( int i = 0; i < nbVertices; ++i )
    if ( !nodeAssigne[ i+1 ])
      theMesh->SetNodeInVolume( HexoticNode[ i+1 ], shapeID );

  delete [] HexoticNode;
  delete [] nodeAssigne;
  return true;
}

//=============================================================================
/*!
 * Pass parameters to MG-Hexa
 */
//=============================================================================

void HexoticPlugin_Hexotic::SetParameters(const HexoticPlugin_Hypothesis* hyp) {

  MESSAGE("HexoticPlugin_Hexotic::SetParameters");
  if (hyp) {
    _hexesMinLevel = hyp->GetHexesMinLevel();
    _hexesMaxLevel = hyp->GetHexesMaxLevel();
    _hexesMinSize = hyp->GetMinSize();
    _hexesMaxSize = hyp->GetMaxSize();
    _hexoticIgnoreRidges = hyp->GetHexoticIgnoreRidges();
    _hexoticInvalidElements = hyp->GetHexoticInvalidElements();
    _hexoticSharpAngleThreshold = hyp->GetHexoticSharpAngleThreshold();
    _hexoticNbProc = hyp->GetHexoticNbProc();
    _hexoticWorkingDirectory = hyp->GetHexoticWorkingDirectory();
    _hexoticVerbosity = hyp->GetHexoticVerbosity();
    _hexoticMaxMemory = hyp->GetHexoticMaxMemory();
    _hexoticSdMode = hyp->GetHexoticSdMode();
    _textOptions = hyp->GetTextOptions();
    _sizeMaps = hyp->GetSizeMaps();
    _nbLayers = hyp->GetNbLayers();
    _firstLayerSize = hyp->GetFirstLayerSize();
    _direction = hyp->GetDirection();
    _growth = hyp->GetGrowth();
    _facesWithLayers = hyp->GetFacesWithLayers();
    _imprintedFaces = hyp->GetImprintedFaces();
  }
  else {
    cout << std::endl;
    cout << "WARNING : The MG-Hexa default parameters are taken into account" << std::endl;
    cout << "=======" << std::endl;
    _hexesMinLevel = hyp->GetDefaultHexesMinLevel();
    _hexesMaxLevel = hyp->GetDefaultHexesMaxLevel();
    _hexesMinSize = hyp->GetDefaultMinSize();
    _hexesMaxSize = hyp->GetDefaultMaxSize();
    _hexoticIgnoreRidges = hyp->GetDefaultHexoticIgnoreRidges();
    _hexoticInvalidElements = hyp->GetDefaultHexoticInvalidElements();
    _hexoticSharpAngleThreshold = hyp->GetDefaultHexoticSharpAngleThreshold();
    _hexoticNbProc = hyp->GetDefaultHexoticNbProc();
    _hexoticWorkingDirectory = hyp->GetDefaultHexoticWorkingDirectory();
    _hexoticVerbosity = hyp->GetDefaultHexoticVerbosity();
    _hexoticMaxMemory = hyp->GetDefaultHexoticMaxMemory();
    _hexoticSdMode = hyp->GetDefaultHexoticSdMode();
    _textOptions = hyp->GetDefaultTextOptions();
    _sizeMaps = hyp->GetDefaultHexoticSizeMaps();
    _nbLayers = hyp->GetDefaultNbLayers();
    _firstLayerSize = hyp->GetDefaultFirstLayerSize();
    _direction = hyp->GetDefaultDirection();
    _growth = hyp->GetDefaultGrowth();
    _facesWithLayers = hyp->GetDefaultFacesWithLayers();
    _imprintedFaces = hyp->GetDefaultImprintedFaces();
  }
}

//=======================================================================
//function : getTmpDir
//purpose  :
//=======================================================================

// static TCollection_AsciiString getTmpDir()
// {
//   TCollection_AsciiString aTmpDir;

//   char *Tmp_dir = getenv("SALOME_TMP_DIR");
// #ifdef WIN32
//   if(Tmp_dir == NULL) {
//     Tmp_dir = getenv("TEMP");
//     if( Tmp_dir== NULL )
//       Tmp_dir = getenv("TMP");
//   }
// #endif

//   if(Tmp_dir != NULL) {
//     aTmpDir = Tmp_dir;
// #ifdef WIN32
//     if(aTmpDir.Value(aTmpDir.Length()) != '\\') aTmpDir+='\\';
// #else
//     if(aTmpDir.Value(aTmpDir.Length()) != '/') aTmpDir+='/';
// #endif
//   }
//   else {
// #ifdef WIN32
//     aTmpDir = TCollection_AsciiString("C:\\");
// #else
//     aTmpDir = TCollection_AsciiString("/tmp/");
// #endif
//   }
//   return aTmpDir;
// }

//=======================================================================
//function : getSuffix
//purpose  : Returns a suffix that will be unique for the current process
//=======================================================================

static TCollection_AsciiString getSuffix()
{
  TCollection_AsciiString aSuffix = "";
  aSuffix += "_";
#ifndef WIN32
  aSuffix += getenv("USER");
#else
  aSuffix += getenv("USERNAME");
#endif
  aSuffix += "_";
  aSuffix += Kernel_Utils::GetHostname().c_str();
  aSuffix += "_";
#ifndef WIN32
  aSuffix += getpid();
#else
  aSuffix += _getpid();
#endif

  return aSuffix;
}

//================================================================================
/*!
 * \brief Returns a command to run MG-Hexa mesher
 */
//================================================================================

std::string HexoticPlugin_Hexotic::getHexoticCommand(const TCollection_AsciiString& Hexotic_In,
                                                     const TCollection_AsciiString& Hexotic_Out,
                                                     const TCollection_AsciiString& Hexotic_SizeMap_Prefix) const
{
  cout << std::endl;
  cout << "MG-Hexa execution..." << std::endl;
  cout << _name << " parameters :" << std::endl;
  cout << "    " << _name << " Verbosity = " << _hexoticVerbosity << std::endl;
  cout << "    " << _name << " Max Memory = " << _hexoticMaxMemory << std::endl;
  cout << "    " << _name << " Segments Min Level = " << _hexesMinLevel << std::endl;
  cout << "    " << _name << " Segments Max Level = " << _hexesMaxLevel << std::endl;
  cout << "    " << _name << " Segments Min Size = " << _hexesMinSize << std::endl;
  cout << "    " << _name << " Segments Max Size = " << _hexesMaxSize << std::endl;
  cout << "    " << "MG-Hexa can ignore ridges : " << (_hexoticIgnoreRidges ? "yes":"no") << std::endl;
  cout << "    " << "MG-Hexa authorize invalide elements : " << ( _hexoticInvalidElements ? "yes":"no") << std::endl;
  cout << "    " << _name << " Sharp angle threshold = " << _hexoticSharpAngleThreshold << " degrees" << std::endl;
  cout << "    " << _name << " Number of threads = " << _hexoticNbProc << std::endl;
  cout << "    " << _name << " Working directory = \"" << _hexoticWorkingDirectory << "\"" << std::endl;
  cout << "    " << _name << " Sub. Dom mode = " << _hexoticSdMode << std::endl;
  cout << "    " << _name << " Text options = \"" << _textOptions << "\"" << std::endl;
  cout << "    " << _name << " Number of layers = " << _nbLayers << std::endl;
  cout << "    " << _name << " Size of the first layer  = " << _firstLayerSize << std::endl;
  cout << "    " << _name << " Direction of the layers = " << ( _direction ? "Inward" : "Outward" ) << std::endl;
  cout << "    " << _name << " Growth = " << _growth << std::endl;
  if (!_facesWithLayers.empty()) {
    cout << "    " << _name << " Faces with layers = ";
    for (size_t i = 0; i < _facesWithLayers.size(); i++)
    {
      cout << _facesWithLayers.at(i);
      if ((i + 1) != _facesWithLayers.size())
        cout << ", ";
    }
    cout << std::endl;
  }
  if (!_imprintedFaces.empty()) {
    cout << "    " << _name << " Imprinted faces = ";
    for (size_t i = 0; i < _imprintedFaces.size(); i++)
    {
      cout << _imprintedFaces.at(i);
      if ((i + 1) != _imprintedFaces.size())
        cout << ", ";
    }
    cout << std::endl;
  }

  TCollection_AsciiString run_Hexotic( "mg-hexa.exe" );

  TCollection_AsciiString minl = " --min_level ", maxl = " --max_level ", angle = " --ridge_angle ";
  TCollection_AsciiString mins = " --min_size ", maxs = " --max_size ";
  TCollection_AsciiString in   = " --in ",   out  = " --out ";
  TCollection_AsciiString sizeMap = " --read_sizemap ";
  TCollection_AsciiString ignoreRidges = " --compute_ridges no ", invalideElements = " --allow_invalid_elements yes ";
  TCollection_AsciiString subdom = " --components ";
#ifndef WIN32
  TCollection_AsciiString proc = " --max_number_of_threads ";
#endif
  TCollection_AsciiString verb = " --verbose ";
  TCollection_AsciiString maxmem = " --max_memory ";

  TCollection_AsciiString comNbLayers = " --number_of_boundary_layers ";
  TCollection_AsciiString comFirstLayerSize = " --height_of_the_first_layer ";
  TCollection_AsciiString comDirection = " --boundary_layers_subdomain_direction ";
  TCollection_AsciiString comGrowth = " --boundary_layers_geometric_progression ";
  TCollection_AsciiString comFacesWithLayers = " --boundary_layers_surface_ids ";
  TCollection_AsciiString comImptintedFaces = " --imprinted_surface_ids ";

  TCollection_AsciiString minLevel, maxLevel, minSize, maxSize, sharpAngle, mode, nbproc, verbosity, maxMemory,
                          textOptions, nbLayers, firstLayerSize, direction, growth, facesWithLayers, imprintedFaces;
  minLevel = _hexesMinLevel;
  maxLevel = _hexesMaxLevel;
  minSize = _hexesMinSize;
  maxSize = _hexesMaxSize;
  sharpAngle = _hexoticSharpAngleThreshold;
  // Mode translation for mg-tetra 1.1
  switch ( _hexoticSdMode )
  {
    case 1:
      mode = "outside_skin_only";
      break;
    case 2:
      mode = "outside_components";
      break;
    case 3:
      mode = "all";
      break;
    case 4:
      mode = "all --manifold_geometry no";
      break;
  }
  nbproc = _hexoticNbProc;
  verbosity = _hexoticVerbosity;
  maxMemory = _hexoticMaxMemory;
  textOptions = (" " + _textOptions + " ").c_str();
  nbLayers = _nbLayers;
  firstLayerSize = _firstLayerSize;
  direction = _direction ? "1" : "-1";
  growth = _growth;
  for (size_t i = 0; i < _facesWithLayers.size(); i++)
  {
    facesWithLayers += _facesWithLayers[i];
    if ((i + 1) != _facesWithLayers.size())
      facesWithLayers += ",";
  }
  for (size_t i = 0; i < _imprintedFaces.size(); i++)
  {
    imprintedFaces += _imprintedFaces[i];
    if ((i + 1) != _imprintedFaces.size())
      imprintedFaces += ",";
  }

  if (_hexoticIgnoreRidges)
    run_Hexotic +=  ignoreRidges;

  if (_hexoticInvalidElements)
    run_Hexotic +=  invalideElements;

  if (_hexesMinSize > 0)
    run_Hexotic +=  mins + minSize;

  if (_hexesMaxSize > 0)
    run_Hexotic +=  maxs + maxSize;

  if (_hexesMinLevel > 0)
    run_Hexotic +=  minl + minLevel;

  if (_hexesMaxLevel > 0)
    run_Hexotic +=  maxl + maxLevel;

  if (_hexoticSharpAngleThreshold > 0)
    run_Hexotic +=  angle + sharpAngle;
  
  if (_sizeMaps.begin() != _sizeMaps.end())
    run_Hexotic += sizeMap + Hexotic_SizeMap_Prefix;

  if (_nbLayers       > 0 &&
      _firstLayerSize > 0 &&
      _growth         > 0 &&
      !_facesWithLayers.empty())
  {
    run_Hexotic += comNbLayers + nbLayers;
    run_Hexotic += comFirstLayerSize + firstLayerSize;
    run_Hexotic += comDirection + direction;
    run_Hexotic += comGrowth + growth;
    run_Hexotic += comFacesWithLayers + facesWithLayers;
    if (!_imprintedFaces.empty())
      run_Hexotic += comImptintedFaces + imprintedFaces;
  }
  run_Hexotic += in + Hexotic_In + out + Hexotic_Out;
  run_Hexotic += subdom + mode;
#ifndef WIN32
  run_Hexotic += proc + nbproc;
#endif
  run_Hexotic += verb + verbosity;
  run_Hexotic += maxmem + maxMemory;

  if (!_textOptions.empty())
    run_Hexotic += textOptions;

  return run_Hexotic.ToCString();
}

// TODO : this is a duplication of some code found in BLSURFPlugin_BLSURF find a proper
// way to share it
TopoDS_Shape HexoticPlugin_Hexotic::entryToShape(std::string entry)
{
  MESSAGE("HexoticPlugin_Hexotic::entryToShape "<<entry );
  GEOM::GEOM_Object_var aGeomObj;
  TopoDS_Shape S = TopoDS_Shape();
  SALOMEDS::SObject_var aSObj = myStudy->FindObjectID( entry.c_str() );
  if (!aSObj->_is_nil()) {
    CORBA::Object_var obj = aSObj->GetObject();
    aGeomObj = GEOM::GEOM_Object::_narrow(obj);
    aSObj->UnRegister();
  }
  if ( !aGeomObj->_is_nil() )
    S = smeshGen_i->GeomObjectToShape( aGeomObj.in() );
  return S;
}

//================================================================================
/*!
 * \brief Produces a .mesh file with the size maps informations to give to Hexotic
 */
//================================================================================
std::vector<std::string> HexoticPlugin_Hexotic::writeSizeMapFile( std::string sizeMapPrefix )
{
  HexoticPlugin_Hypothesis::THexoticSizeMaps::iterator it ;
  
  // The GMF driver will be used to write the size map file
  DriverGMF_Write aWriter;
  aWriter.SetSizeMapPrefix( sizeMapPrefix ); 
  
  std::vector<Control_Pnt> points;
  // Iterate on the size maps
  for (it=_sizeMaps.begin(); it!=_sizeMaps.end(); it++)
  {
    // Step 1 : Get the GEOM object entry and the size 
    // from the _sizeMaps infos
    std::string anEntry = it->first;
    double aLocalSize = it->second;
    TopoDS_Shape aShape = entryToShape( anEntry );
    
    // Step 2 : Create the points
    createControlPoints( aShape, aLocalSize, points );
  }
  // Write the .mesh size map file
  aWriter.PerformSizeMap(points); 
  return aWriter.GetSizeMapFiles();
}

//================================================================================
/*!
 * \brief Fills a vector of points from which a size map input file can be written
 */
//================================================================================
void HexoticPlugin_Hexotic::createControlPoints( const TopoDS_Shape& aShape, 
                                                 const double& theSize, 
                                                 std::vector<Control_Pnt>& thePoints )
{ 
  if ( aShape.ShapeType() == TopAbs_VERTEX )
  {
    gp_Pnt aPnt = BRep_Tool::Pnt( TopoDS::Vertex(aShape) );
    Control_Pnt aControl_Pnt( aPnt, theSize );
    thePoints.push_back( aControl_Pnt );
  }
  if ( aShape.ShapeType() == TopAbs_EDGE )
  {
    createPointsSampleFromEdge( aShape, theSize, thePoints );  
  }
  else if ( aShape.ShapeType() == TopAbs_WIRE )
  {
    TopExp_Explorer Ex;
    for (Ex.Init(aShape,TopAbs_EDGE); Ex.More(); Ex.Next()) 
    {
      createPointsSampleFromEdge( Ex.Current(), theSize, thePoints );
    } 
  }
  else if ( aShape.ShapeType() ==  TopAbs_FACE )
  {
    createPointsSampleFromFace( aShape, theSize, thePoints ); 
  }
  else if ( aShape.ShapeType() ==  TopAbs_SOLID )
  {
    createPointsSampleFromSolid( aShape, theSize, thePoints ); 
  }
  else if ( aShape.ShapeType() == TopAbs_COMPOUND )
  {
    TopoDS_Iterator it( aShape );
    for(; it.More(); it.Next())
    {
      createControlPoints( it.Value(), theSize, thePoints );
    }
  }
}

//================================================================================
/*!
 * \brief Fills a vector of points with point samples approximately 
 * \brief spaced with a given size
 */
//================================================================================
void HexoticPlugin_Hexotic::createPointsSampleFromEdge( const TopoDS_Shape& aShape, 
                                                        const double& theSize, 
                                                        std::vector<Control_Pnt>& thePoints )
{
  double step = theSize;
  double first, last;  
  Handle( Geom_Curve ) aCurve = BRep_Tool::Curve( TopoDS::Edge( aShape ), first, last );
  GeomAdaptor_Curve C ( aCurve );
  GCPnts_UniformAbscissa DiscretisationAlgo(C, step , first, last, Precision::Confusion());
  int nbPoints = DiscretisationAlgo.NbPoints();
  
  for ( int i = 1; i <= nbPoints; i++ )
  {
    double param = DiscretisationAlgo.Parameter( i );
    Control_Pnt aPnt;
    aCurve->D0( param, aPnt );
    aPnt.SetSize(theSize);
    thePoints.push_back( aPnt );
  }  
}

//================================================================================
/*!
 * \brief Fills a vector of points with point samples approximately 
 * \brief spaced with a given size
 */
//================================================================================
void HexoticPlugin_Hexotic::createPointsSampleFromFace( const TopoDS_Shape& aShape, 
                                                        const double& theSize, 
                                                        std::vector<Control_Pnt>& thePoints )
{
  BRepMesh_IncrementalMesh M(aShape, 0.01, Standard_True);
  TopLoc_Location aLocation;
  TopoDS_Face aFace = TopoDS::Face(aShape);

  // Triangulate the face
  Handle(Poly_Triangulation) aTri = BRep_Tool::Triangulation (aFace, aLocation);
  
  // Get the transformation associated to the face location
  gp_Trsf aTrsf = aLocation.Transformation();
  
  // Get triangles
  int nbTriangles = aTri->NbTriangles();
  Poly_Array1OfTriangle triangles(1,nbTriangles);
  triangles=aTri->Triangles();
  
  // GetNodes
  int nbNodes = aTri->NbNodes();
  TColgp_Array1OfPnt nodes(1,nbNodes);
  nodes = aTri->Nodes();

  // Iterate on triangles and subdivide them
  for(int i=1; i<=nbTriangles; i++)
  {
     Poly_Triangle aTriangle = triangles.Value(i);
     gp_Pnt p1 = nodes.Value(aTriangle.Value(1));
     gp_Pnt p2 = nodes.Value(aTriangle.Value(2));
     gp_Pnt p3 = nodes.Value(aTriangle.Value(3));
     
     p1.Transform(aTrsf);
     p2.Transform(aTrsf);
     p3.Transform(aTrsf);
     
     subdivideTriangle(p1, p2, p3, theSize, thePoints);  
  }
}

//================================================================================
/*!
 * \brief Fills a vector of points with point samples approximately 
 * \brief spaced with a given size
 */
//================================================================================
void HexoticPlugin_Hexotic::createPointsSampleFromSolid( const TopoDS_Shape& aShape, 
                                                         const double& theSize, 
                                                         std::vector<Control_Pnt>& thePoints )
{
  // Compute the bounding box
  double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
  Bnd_Box B;               
  BRepBndLib::Add(aShape, B);
  B.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
  
  // Create the points
  double step = theSize;
  
  for ( double x=Xmin; x-Xmax<Precision::Confusion(); x=x+step )
  {
    for ( double y=Ymin; y-Ymax<Precision::Confusion(); y=y+step )
    {
      // Step1 : generate the Zmin -> Zmax line
      gp_Pnt startPnt(x, y, Zmin);
      gp_Pnt endPnt(x, y, Zmax);
      gp_Vec aVec(startPnt, endPnt);
      gp_Lin aLine(startPnt, aVec);
      double endParam = Zmax - Zmin;
      
      // Step2 : for each face of aShape:
      std::set<double> intersections;
      std::set<double>::iterator it = intersections.begin();
      
      TopExp_Explorer Ex;
      for (Ex.Init(aShape,TopAbs_FACE); Ex.More(); Ex.Next()) 
      { 
        // check if there is an intersection
        IntCurvesFace_Intersector anIntersector(TopoDS::Face(Ex.Current()), Precision::Confusion());
        anIntersector.Perform(aLine, 0, endParam);
        
        // get the intersection's parameter and store it
        int nbPoints = anIntersector.NbPnt();
        for(int i = 0 ; i < nbPoints ; i++ )
        {
          it = intersections.insert( it, anIntersector.WParameter(i+1) );
        }
      }
      // Step3 : go through the line chunk by chunk 
      if ( intersections.begin() != intersections.end() )
      {
        std::set<double>::iterator intersectionsIterator=intersections.begin();
        double first = *intersectionsIterator;
        intersectionsIterator++;
        bool innerPoints = true; 
        for ( ; intersectionsIterator!=intersections.end() ; intersectionsIterator++ )
        {
          double second = *intersectionsIterator;
          if ( innerPoints )
          {
            // If the last chunk was outside of the shape or this is the first chunk
            // add the points in the range [first, second] to the points vector
            double localStep = (second -first) / ceil( (second - first) / step );
            for ( double z = Zmin + first; z < Zmin + second; z = z + localStep )
            {
              thePoints.push_back(Control_Pnt( x, y, z, theSize ));
            }
            thePoints.push_back(Control_Pnt( x, y, Zmin + second, theSize ));
          }
          first = second;
          innerPoints = !innerPoints;
        }
      }
    }
  }
}

//================================================================================
/*!
 * \brief Subdivides a triangle until it reaches a certain size (recursive function)
 */
//================================================================================
void HexoticPlugin_Hexotic::subdivideTriangle( const gp_Pnt& p1, 
                                               const gp_Pnt& p2, 
                                               const gp_Pnt& p3, 
                                               const double& theSize, 
                                               std::vector<Control_Pnt>& thePoints)
{
  // Size threshold to stop subdividing
  // This value ensures that two control points are distant no more than 2*theSize
  // as shown below
  //
  // The greater distance D of the mass center M to each Edge is 1/3 * Median 
  // and Median < sqrt(3/4) * a  where a is the greater side (by using Apollonius' thorem). 
  // So D < 1/3 * sqrt(3/4) * a and if a < sqrt(3) * S then D < S/2
  // and the distance between two mass centers of two neighbouring triangles 
  // sharing an edge is < 2 * 1/2 * S = S
  // If the traingles share a Vertex and no Edge the distance of the mass centers 
  // to the Vertices is 2*D < S so the mass centers are distant of less than 2*S 
  
  double threshold = sqrt( 3. ) * theSize;
  
  if ( (p1.Distance(p2) > threshold ||
        p2.Distance(p3) > threshold ||
        p3.Distance(p1) > threshold))
  { 
    std::vector<gp_Pnt> midPoints = computePointsForSplitting(p1, p2, p3);
 
    subdivideTriangle( midPoints[0], midPoints[1], midPoints[2], theSize, thePoints );
    subdivideTriangle( midPoints[0], p2, midPoints[1], theSize, thePoints );
    subdivideTriangle( midPoints[2], midPoints[1], p3, theSize, thePoints );
    subdivideTriangle( p1, midPoints[0], midPoints[2], theSize, thePoints );
  }
  else
  {
    double x = (p1.X() + p2.X() + p3.X()) / 3 ;
    double y = (p1.Y() + p2.Y() + p3.Y()) / 3 ;
    double z = (p1.Z() + p2.Z() + p3.Z()) / 3 ;
    
    Control_Pnt massCenter( x ,y ,z, theSize );
    thePoints.push_back( massCenter );
  }
}

//================================================================================
/*!
 * \brief Returns the appropriate points for splitting a triangle
 * \brief the tangency points of the incircle are used in order to have mostly
 * \brief well-shaped sub-triangles
 */
//================================================================================
std::vector<gp_Pnt> HexoticPlugin_Hexotic::computePointsForSplitting( const gp_Pnt& p1, 
                                                                      const gp_Pnt& p2, 
                                                                      const gp_Pnt& p3 )
{
  std::vector<gp_Pnt> midPoints;
  //Change coordinates
  gp_Trsf Trsf_1;            // Identity transformation
  gp_Ax3 reference_system(gp::Origin(), gp::DZ(), gp::DX());   // OXY
 
  gp_Vec Vx(p1, p3);
  gp_Vec Vaux(p1, p2);
  gp_Dir Dx(Vx);
  gp_Dir Daux(Vaux);
  gp_Dir Dz = Dx.Crossed(Daux);
  gp_Ax3 current_system(p1, Dz, Dx);
  
  Trsf_1.SetTransformation( reference_system, current_system );
  
  gp_Pnt A = p1.Transformed(Trsf_1);
  gp_Pnt B = p2.Transformed(Trsf_1);
  gp_Pnt C = p3.Transformed(Trsf_1);
  
  double a =  B.Distance(C) ;
  double b =  A.Distance(C) ;
  double c =  B.Distance(A) ;
  
  // Incenter coordinates
  // see http://mathworld.wolfram.com/Incenter.html
  double Xi = ( b*B.X() + c*C.X() ) / ( a + b + c );
  double Yi = ( b*B.Y() ) / ( a + b + c );
  gp_Pnt Center(Xi, Yi, 0);
  
  // Calculate the tangency points of the incircle
  gp_Pnt T1 = tangencyPoint( A, B, Center);
  gp_Pnt T2 = tangencyPoint( B, C, Center);
  gp_Pnt T3 = tangencyPoint( C, A, Center);
  
  gp_Pnt p1_2 = T1.Transformed(Trsf_1.Inverted());
  gp_Pnt p2_3 = T2.Transformed(Trsf_1.Inverted());
  gp_Pnt p3_1 = T3.Transformed(Trsf_1.Inverted());

  midPoints.push_back(p1_2);
  midPoints.push_back(p2_3);
  midPoints.push_back(p3_1);
  
  return midPoints;
}

//================================================================================
/*!
 * \brief Computes the tangency points of the circle of center Center with
 * \brief the straight line (p1 p2)
 */
//================================================================================
gp_Pnt HexoticPlugin_Hexotic::tangencyPoint(const gp_Pnt& p1,
                                            const gp_Pnt& p2,
                                            const gp_Pnt& Center)
{
  double Xt = 0;
  double Yt = 0;
  
  // The tangency point is the intersection of the straight line (p1 p2)
  // and the straight line (Center T) which is orthogonal to (p1 p2)
  if ( fabs(p1.X() - p2.X()) <= Precision::Confusion() )
  {
    Xt=p1.X();     // T is on (p1 p2)
    Yt=Center.Y(); // (Center T) is orthogonal to (p1 p2)
  }
  else if ( fabs(p1.Y() - p2.Y()) <= Precision::Confusion() )
  {
    Yt=p1.Y();     // T is on (p1 p2) 
    Xt=Center.X(); // (Center T) is orthogonal to (p1 p2)
  }
  else
  {
    // First straight line coefficients (equation y=a*x+b)
    double a = (p2.Y() - p1.Y()) / (p2.X() - p1.X())  ;
    double b = p1.Y() - a*p1.X();         // p1 is on this straight line
    
    // Second straight line coefficients (equation y=c*x+d)
    double c = -1 / a;                    // The 2 lines are orthogonal
    double d = Center.Y() - c*Center.X(); // Center is on this straight line
    
    Xt = (d - b) / (a - c);
    Yt = a*Xt + b;
  }
  
  return gp_Pnt( Xt, Yt, 0 );
}

//=============================================================================
/*!
 * Here we are going to use the MG-Hexa mesher
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Compute(SMESH_Mesh&          aMesh,
                                    const TopoDS_Shape& aShape)
{
  _compute_canceled = false;
  bool Ok = true;
  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  TCollection_AsciiString hexahedraMessage;

  if (_iShape == 0 && _nbShape == 0) {
    _nbShape = countShape( meshDS, TopAbs_SOLID );  // we count the number of shapes
  }

  // to prevent from displaying error message after computing,
  // SetIsAlwaysComputed( true ) to empty sub-meshes
  std::vector< SMESH_subMesh* > subMeshesAlwaysComp;
  for ( int i = 0; i < _nbShape; ++i )
    if ( SMESH_subMesh* sm = aMesh.GetSubMeshContaining( aShape ))
    {
      SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(/*includeSelf=*/true,
                                                               /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        sm = smIt->next();
        if ( !sm->IsMeshComputed() )
        {
          sm->SetIsAlwaysComputed( true );
          subMeshesAlwaysComp.push_back( sm );
        }
      }
    }

  _iShape++;

  if (_iShape == _nbShape ) {

    // create bounding box for each shape of the compound

    int iShape = 0;
    TopoDS_Shape *tabShape;
    double **tabBox;

    tabShape = new TopoDS_Shape[_nbShape];
    tabBox   = new double*[_nbShape];
    for (int i=0; i<_nbShape; i++)
      tabBox[i] = new double[6];
    double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;

    TopExp_Explorer expBox (meshDS->ShapeToMesh(), TopAbs_SOLID);
    for (; expBox.More(); expBox.Next()) {
      tabShape[iShape] = expBox.Current();
      Bnd_Box BoundingBox;
      BRepBndLib::Add(expBox.Current(), BoundingBox);
      BoundingBox.Get(Xmin, Ymin, Zmin, Xmax, Ymax, Zmax);
      tabBox[iShape][0] = Xmin; tabBox[iShape][1] = Xmax;
      tabBox[iShape][2] = Ymin; tabBox[iShape][3] = Ymax;
      tabBox[iShape][4] = Zmin; tabBox[iShape][5] = Zmax;
      iShape++;
    }

    SetParameters(_hypothesis);

//     TCollection_AsciiString aTmpDir = getTmpDir();
    TCollection_AsciiString aTmpDir = _hexoticWorkingDirectory.c_str();
#ifdef WIN32
    if ( aTmpDir.Value(aTmpDir.Length()) != '\\' ) aTmpDir += '\\';
#else
    if ( aTmpDir.Value(aTmpDir.Length()) != '/' ) aTmpDir += '/';
#endif
    TCollection_AsciiString Hexotic_In(""), Hexotic_Out, Hexotic_SizeMap_Prefix;
    TCollection_AsciiString modeFile_In( "chmod 666 " ), modeFile_Out( "chmod 666 " );    TCollection_AsciiString aLogFileName = aTmpDir + "Hexotic"+getSuffix()+".log";    // log

    std::map <int,int> aSmdsToHexoticIdMap;
    std::map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

    Hexotic_Out = aTmpDir + "Hexotic"+getSuffix()+"_Out.mesh";
#ifdef WITH_BLSURFPLUGIN
    bool defaultInputFile = true;
    if (_blsurfHypo && !_blsurfHypo->GetQuadAllowed()) {
      Hexotic_In = _blsurfHypo->GetGMFFile().c_str();
      if ( !Hexotic_In.IsEmpty() &&
           SMESH_File( _blsurfHypo->GetGMFFile() ).exists() )
        defaultInputFile = false;
    }
    if (defaultInputFile) {
#endif
      Hexotic_In  = aTmpDir + "Hexotic"+getSuffix()+"_In.mesh";
      removeHexoticFiles(Hexotic_In, Hexotic_Out);
      splitQuads(aMesh); // quadrangles are no longer acceptable as input
      cout << std::endl;
      cout << "Creating MG-Hexa input mesh file : " << Hexotic_In << std::endl;
      aMesh.ExportGMF(Hexotic_In.ToCString(), meshDS, true);
#ifdef WITH_BLSURFPLUGIN
    }
    else {
      removeFile( Hexotic_Out );
    }
#endif
    
    Hexotic_SizeMap_Prefix = aTmpDir + "Hexotic_SizeMap" + getSuffix();
    std::vector<std::string> sizeMapFiles = writeSizeMapFile( Hexotic_SizeMap_Prefix.ToCString() );
    
    std::string run_Hexotic = getHexoticCommand(Hexotic_In, Hexotic_Out, Hexotic_SizeMap_Prefix);
    run_Hexotic += std::string(" 1> ") + aLogFileName.ToCString();  // dump into file

    cout << std::endl;
    cout << "MG-Hexa command : " << run_Hexotic << std::endl;

#ifndef WIN32    
    modeFile_In += Hexotic_In;
    system( modeFile_In.ToCString() );
#endif
    aSmdsToHexoticIdMap.clear();
    aHexoticIdToNodeMap.clear();

    MESSAGE("HexoticPlugin_Hexotic::Compute");

    int status = system( run_Hexotic.data() );

    // --------------
    // read a result
    // --------------

    std::ifstream fileRes( Hexotic_Out.ToCString() );
#ifndef WIN32  
    modeFile_Out += Hexotic_Out;
    system( modeFile_Out.ToCString() );
#endif
    if ( ! fileRes.fail() ) {
      Ok = readResult( Hexotic_Out.ToCString(),
                       this,
                       meshDS, _nbShape, tabShape, tabBox );
      if(Ok) {
/*********************
// TODO: Detect and remove elements in holes in case of sd mode = 4
      // Remove previous nodes and elements
      SMDS_ElemIteratorPtr itElement = meshDS->elementsIterator();
      SMDS_NodeIteratorPtr itNode = meshDS->nodesIterator();
    
      while ( itElement->more() )
        meshDS->RemoveElement( itElement->next() );
      while ( itNode->more() )
        meshDS->RemoveNode( itNode->next() );
  
      SMESH_ComputeErrorPtr myError = aMesh.GMFToMesh(Hexotic_Out.ToCString());
      if (myError)
*/
        hexahedraMessage = "success";
        #ifndef _DEBUG_
        removeFile(Hexotic_Out);
        removeFile(Hexotic_In);
        removeFile(aLogFileName);
        for( int i=0; i<sizeMapFiles.size(); i++)
        {
          removeFile( TCollection_AsciiString( sizeMapFiles[i].c_str() ) );
        }
        #endif
      }
      else {
        hexahedraMessage = "failed"; 
      }
    }
    else {
      hexahedraMessage = "failed";
      cout << "Problem with MG-Hexa output file " << Hexotic_Out.ToCString() << std::endl;
      Ok = false;
      // analyse log file
      SMESH_File logFile( aLogFileName.ToCString() );
      if ( !logFile.eof() )
      {
        char msgLic[] = " Dlim ";
        const char* fileBeg = logFile.getPos(), *fileEnd = fileBeg + logFile.size();
        if ( std::search( fileBeg, fileEnd, msgLic, msgLic+strlen(msgLic)) != fileEnd )
          error("Licence problems.");
      }
#ifndef WIN32
      if ( status > 0 && WEXITSTATUS(status) == 127 )
        error("mg-hexa.exe: command not found");
#else
      int err = errno;
      if ( status == 0 && err == ENOENT ) {
        error("mg-hexa.exe: command not found");
      }
#endif
    }
    cout << "Hexahedra meshing " << hexahedraMessage << std::endl;
    cout << std::endl;

    // restore "always computed" flag of sub-meshes (0022127)
    for  ( size_t iSM = 0; iSM < subMeshesAlwaysComp.size(); ++iSM )
      subMeshesAlwaysComp[ iSM ]->SetIsAlwaysComputed( false );

    delete [] tabShape;
    for (int i=0; i<_nbShape; i++)
      delete [] tabBox[i];
    delete [] tabBox;
    _nbShape = 0;
    _iShape  = 0;
  }
  if(_compute_canceled)
    return error(SMESH_Comment("interruption initiated by user"));
  return Ok;
}

//=============================================================================
/*!
 * \brief Computes mesh without geometry
 *  \param aMesh - the mesh
 *  \param aHelper - helper that must be used for adding elements to \aaMesh
 *  \retval bool - is a success
 *
 * The method is called if ( !aMesh->HasShapeToMesh() )
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Compute(SMESH_Mesh & aMesh, SMESH_MesherHelper* aHelper)
{
  _compute_canceled = false;
/*
  SMESH_ComputeErrorPtr myError = SMESH_ComputeError::New();
*/
  bool Ok = true;
  TCollection_AsciiString hexahedraMessage;

  SetParameters(_hypothesis);

  TCollection_AsciiString aTmpDir = _hexoticWorkingDirectory.c_str();//getTmpDir();
  TCollection_AsciiString Hexotic_In, Hexotic_Out, Hexotic_SizeMap_Prefix;
  TCollection_AsciiString modeFile_In( "chmod 666 " ), modeFile_Out( "chmod 666 " );
  TCollection_AsciiString aLogFileName = aTmpDir + "Hexotic"+getSuffix()+".log";    // log

  std::map <int,int> aSmdsToHexoticIdMap;
  std::map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

  Hexotic_In  = aTmpDir + "Hexotic"+getSuffix()+"_In.mesh";
  Hexotic_Out = aTmpDir + "Hexotic"+getSuffix()+"_Out.mesh";
  Hexotic_SizeMap_Prefix = aTmpDir + "Hexotic_SizeMap";
 
  
  std::vector<std::string> sizeMapFiles = writeSizeMapFile( Hexotic_SizeMap_Prefix.ToCString() );

  std::string run_Hexotic = getHexoticCommand(Hexotic_In, Hexotic_Out, Hexotic_SizeMap_Prefix);
  run_Hexotic += std::string(" 1> ") + aLogFileName.ToCString();  // dump into file

  removeHexoticFiles(Hexotic_In, Hexotic_Out);

  splitQuads(aMesh); // quadrangles are no longer acceptable as input

  cout << std::endl;
  cout << "Creating MG-Hexa input mesh file : " << Hexotic_In << std::endl;
  aMesh.ExportGMF(Hexotic_In.ToCString(), aHelper->GetMeshDS());
#ifndef WIN32    
  modeFile_In += Hexotic_In;
  system( modeFile_In.ToCString() );
#endif
  aSmdsToHexoticIdMap.clear();
  aHexoticIdToNodeMap.clear();

  MESSAGE("HexoticPlugin_Hexotic::Compute");

  cout << std::endl;
  cout << "MG-Hexa command : " << run_Hexotic << std::endl;
  system( run_Hexotic.data() );

  // --------------
  // read a result
  // --------------

  std::ifstream fileRes( Hexotic_Out.ToCString() );
  modeFile_Out += Hexotic_Out;
  system( modeFile_Out.ToCString() );
  if ( ! fileRes.fail() ) {
    Ok = readResult( Hexotic_Out.ToCString(),
                     this,
                     aHelper );
    if(Ok)
/*
    // Remove previous nodes and elements
    SMDS_ElemIteratorPtr itElement = aHelper->GetMeshDS()->elementsIterator();
    SMDS_NodeIteratorPtr itNode = aHelper->GetMeshDS()->nodesIterator();
    
    while ( itElement->more() )
      aHelper->GetMeshDS()->RemoveElement( itElement->next() );
    while ( itNode->more() )
      aHelper->GetMeshDS()->RemoveNode( itNode->next() );

    // Import GMF mesh
    myError = aMesh.GMFToMesh(Hexotic_Out.ToCString());
    
    itElement = aHelper->GetMeshDS()->elementsIterator();
    itNode = aHelper->GetMeshDS()->nodesIterator();

    // Assign nodes and elements to the pseudo shape
    while ( itNode->more() )
      aHelper->GetMeshDS()->SetNodeInVolume(itNode->next(), 1);
    while ( itElement->more() )
      aHelper->GetMeshDS()->SetMeshElementOnShape(itElement->next(), 1);

    if(myError->IsOK())
*/
      hexahedraMessage = "success";
    else
      hexahedraMessage = "failed";
  }
  else {
/*
    myError->myName = COMPERR_EXCEPTION;
*/
    hexahedraMessage = "failed";
    cout << "Problem with MG-Hexa output file " << Hexotic_Out << std::endl;
    // analyse log file
    SMESH_File logFile( aLogFileName.ToCString() );
    if ( !logFile.eof() )
    {
      char msgLic[] = " Dlim ";
      const char* fileBeg = logFile.getPos(), *fileEnd = fileBeg + logFile.size();
      if ( std::search( fileBeg, fileEnd, msgLic, msgLic+strlen(msgLic)) != fileEnd )
        return error("Licence problems.");
    }
    return error(SMESH_Comment("Problem with MG-Hexa output file ")<<Hexotic_Out);
  }
  cout << "Hexahedra meshing " << hexahedraMessage << std::endl;
  cout << std::endl;

  if(_compute_canceled)
    return error(SMESH_Comment("interruption initiated by user"));
  removeFile(Hexotic_Out);
  removeFile(Hexotic_In);
  removeFile(aLogFileName);
  for( size_t i=0; i<sizeMapFiles.size(); i++)
  {
    removeFile( TCollection_AsciiString(sizeMapFiles[i].c_str()) );
  }
  return Ok;
/*
  return myError->IsOK();
*/
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Evaluate(SMESH_Mesh& aMesh,
                                     const TopoDS_Shape& aShape,
                                     MapShapeNbElems& aResMap)
{
  std::vector<int> aResVec(SMDSEntity_Last);
  for(int i=SMDSEntity_Node; i<SMDSEntity_Last; i++) aResVec[i] = 0;
  SMESH_subMesh * sm = aMesh.GetSubMesh(aShape);
  aResMap.insert(std::make_pair(sm,aResVec));

  SMESH_ComputeErrorPtr& smError = sm->GetComputeError();
  smError.reset( new SMESH_ComputeError(COMPERR_ALGO_FAILED,"Evaluation is not implemented",this));

  return true;
}

void HexoticPlugin_Hexotic::CancelCompute()
{
  _compute_canceled = true;
#ifdef WIN32
#else
  TCollection_AsciiString aTmpDir = _hexoticWorkingDirectory.c_str(); //getTmpDir();
  TCollection_AsciiString Hexotic_In = aTmpDir + "Hexotic_In.mesh";
  TCollection_AsciiString cmd = TCollection_AsciiString("ps ux | grep ") + Hexotic_In;
  cmd += TCollection_AsciiString(" | grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>&1");
  system( cmd.ToCString() );
#endif
}
