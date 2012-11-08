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
// File   : HexoticPlugin_Hexotic.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hexotic.hxx"
#include "HexoticPlugin_Hypothesis.hxx"

#include "utilities.h"

#ifndef WIN32
#include <sys/sysinfo.h>
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
  _name = "Hexotic_3D";
  _shapeType = (1 << TopAbs_SHELL) | (1 << TopAbs_SOLID);// 1 bit /shape type
//   _onlyUnaryInput = false;
  _requireShape = false;
  _iShape=0;
  _nbShape=0;
  _hexoticFilesKept=false;
  _compatibleHypothesis.push_back("Hexotic_Parameters");
#ifdef WITH_BLSURFPLUGIN
  _blsurfHypo = NULL;
#endif
#ifdef WITH_SMESH_CANCEL_COMPUTE
  _compute_canceled = false;
#endif
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
  blsurfFilter.Init( blsurfFilter.HasName( "BLSURF_Parameters" ));
  std::list<const SMESHDS_Hypothesis *> appliedHyps;
  aMesh.GetHypotheses( aShape, blsurfFilter, appliedHyps, false );

  if ( appliedHyps.size() > 0 ) {
    itl = appliedHyps.begin();
    theHyp = (*itl); // use only the first hypothesis
    std::string hypName = theHyp->GetName();
    if (hypName == "BLSURF_Parameters") {
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
  if (hypName == "Hexotic_Parameters") {
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
//function : findEdge
//purpose  :
//=======================================================================

static int findEdge(const SMDS_MeshNode* aNode,
                    const SMESHDS_Mesh*  aMesh,
                    const int            nEdge,
                    const TopoDS_Shape*  t_Edge) {

  TopoDS_Shape aPntShape, foundEdge;
  TopoDS_Vertex aVertex;
  gp_Pnt aPnt( aNode->X(), aNode->Y(), aNode->Z() );

  int foundInd, ind;
  double nearest = RealLast(), *t_Dist;
  double epsilon = Precision::Confusion();

  t_Dist = new double[ nEdge ];
  aPntShape = BRepBuilderAPI_MakeVertex( aPnt ).Shape();
  aVertex   = TopoDS::Vertex( aPntShape );

  for ( ind=0; ind < nEdge; ind++ ) {
    BRepExtrema_DistShapeShape aDistance ( aVertex, t_Edge[ind] );
    t_Dist[ind] = aDistance.Value();
    if ( t_Dist[ind] < nearest ) {
      nearest   = t_Dist[ind];
      foundEdge = t_Edge[ind];
      foundInd  = ind;
      if ( nearest < epsilon )
        ind = nEdge;
    }
  }

  delete [] t_Dist;
  return aMesh->ShapeToIndex( foundEdge );
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
  cout << "WARNING : " << nbExpected << " " << aString << " expected, Hexotic has found " << nbFound << std::endl;
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
//function : readResult
//purpose  : Read GMF file in case of a mesh with geometry
//=======================================================================

static bool readResult(std::string         theFile,
#ifdef WITH_SMESH_CANCEL_COMPUTE
                       HexoticPlugin_Hexotic*  theAlgo,
#endif
                       SMESHDS_Mesh*       theMesh,
                       const int           nbShape,
                       const TopoDS_Shape* tabShape,
                       double**            tabBox)
{
  // ---------------------------------
  // Optimisation of the plugin ...
  // Retrieve the correspondance edge --> shape
  // (which is very costly) only when user
  // has defined at least one group of edges
  // which should be rare for a 3d mesh !
  // ---------------------------------
  
  bool retrieve_edges = false;
  const std::set<SMESHDS_GroupBase*>& aGroups = theMesh->GetGroups();
  set<SMESHDS_GroupBase*>::const_iterator GrIt = aGroups.begin();
  for (; GrIt != aGroups.end(); GrIt++)
    {
      SMESHDS_GroupBase* aGrp = *GrIt;
      if ( !aGrp )
        continue;
      if ( aGrp->GetType() == SMDSAbs_Edge )
        {
          retrieve_edges = true;
          break;
        }
    }
  
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
    if ( nField < (mapField.size() - 1) && nField >= 0 )
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
          if(theAlgo->computeCanceled())
            {
              return false;
            }
#endif
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
          if(theAlgo->computeCanceled())
            {
              return false;
            }
#endif
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
              break;
            }
            case 4: { // "Edges"
              nodeDim = 2;
              aHexoticElement = theMesh->AddEdge( node[0], node[1] );
              int iNode = 1;
              if ( nodeAssigne[ nodeID[0] ] == 0 || nodeAssigne[ nodeID[0] ] == 2 )
                iNode = 0;
              if(retrieve_edges)
                shapeID = findEdge( node[iNode], theMesh, nbShapeEdge, tabEdge );
              else
                shapeID = 0;
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
                       HexoticPlugin_Hexotic*  theAlgo,
#endif
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

  theHelper->GetMesh()->Clear();

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
    if ( nField < (mapField.size() - 1) && nField >= 0 )
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
        if(theAlgo->computeCanceled())
          {
            return false;
          }
#endif
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
        if(theAlgo->computeCanceled())
          {
            return false;
          }
#endif
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
 * Pass parameters to Hexotic
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
  }
  else {
    cout << std::endl;
    cout << "WARNING : The Hexotic default parameters are taken into account" << std::endl;
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
  }
}

//=======================================================================
//function : getTmpDir
//purpose  :
//=======================================================================

static TCollection_AsciiString getTmpDir()
{
  TCollection_AsciiString aTmpDir;

  char *Tmp_dir = getenv("SALOME_TMP_DIR");
  if(Tmp_dir != NULL) {
    aTmpDir = Tmp_dir;
    #ifdef WIN32
    if(aTmpDir.Value(aTmpDir.Length()) != '\\') aTmpDir+='\\';
#else
    if(aTmpDir.Value(aTmpDir.Length()) != '/') aTmpDir+='/';
#endif
  }
  else {
#ifdef WIN32
    aTmpDir = TCollection_AsciiString("C:\\");
#else
    aTmpDir = TCollection_AsciiString("/tmp/");
#endif
  }
  return aTmpDir;
}

//================================================================================
/*!
 * \brief Returns a command to run Hexotic mesher
 */
//================================================================================

std::string HexoticPlugin_Hexotic::getHexoticCommand(const TCollection_AsciiString& Hexotic_In,
                                                     const TCollection_AsciiString& Hexotic_Out) const
{
  cout << std::endl;
  cout << "Hexotic execution..." << std::endl;
  cout << _name << " parameters :" << std::endl;
  cout << "    " << _name << " Verbosity = " << _hexoticVerbosity << std::endl;
  cout << "    " << _name << " Max Memory = " << _hexoticMaxMemory << std::endl;
  cout << "    " << _name << " Segments Min Level = " << _hexesMinLevel << std::endl;
  cout << "    " << _name << " Segments Max Level = " << _hexesMaxLevel << std::endl;
  cout << "    " << _name << " Segments Min Size = " << _hexesMinSize << std::endl;
  cout << "    " << _name << " Segments Max Size = " << _hexesMaxSize << std::endl;
  cout << "    " << "Hexotic can ignore ridges : " << (_hexoticIgnoreRidges ? "yes":"no") << std::endl;
  cout << "    " << "Hexotic authorize invalide elements : " << ( _hexoticInvalidElements ? "yes":"no") << std::endl;
  cout << "    " << _name << " Sharp angle threshold = " << _hexoticSharpAngleThreshold << " degrees" << std::endl;
  cout << "    " << _name << " Number of threads = " << _hexoticNbProc << std::endl;
  cout << "    " << _name << " Working directory = \"" << _hexoticWorkingDirectory << "\"" << std::endl;
  cout << "    " << _name << " Sub. Dom mode = " << _hexoticSdMode << std::endl;

  TCollection_AsciiString run_Hexotic( "hexotic" );

  TCollection_AsciiString minl = " -minl ", maxl = " -maxl ", angle = " -ra ";
  TCollection_AsciiString mins = " -mins ", maxs = " -maxs ";
  TCollection_AsciiString in   = " -in ",   out  = " -out ";
  TCollection_AsciiString ignoreRidges = " -nr ", invalideElements = " -inv ";
  TCollection_AsciiString subdom = " -sd ", sharp = " -sharp ";
  TCollection_AsciiString proc = " -nproc ";
  TCollection_AsciiString verb = " -v ";
  TCollection_AsciiString maxmem = " -m ";

  TCollection_AsciiString minLevel, maxLevel, minSize, maxSize, sharpAngle, mode, nbproc, verbosity, maxMemory;
  minLevel = _hexesMinLevel;
  maxLevel = _hexesMaxLevel;
  minSize = _hexesMinSize;
  maxSize = _hexesMaxSize;
  sharpAngle = _hexoticSharpAngleThreshold;
  mode = _hexoticSdMode;
  nbproc = _hexoticNbProc;
  verbosity = _hexoticVerbosity;
  maxMemory = _hexoticMaxMemory;

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

  run_Hexotic += in + Hexotic_In + out + Hexotic_Out;
  run_Hexotic += subdom + mode;
  run_Hexotic += proc + nbproc;
  run_Hexotic += verb + verbosity;
  run_Hexotic += maxmem + maxMemory;

  return run_Hexotic.ToCString();
}

//=============================================================================
/*!
 * Here we are going to use the Hexotic mesher
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Compute(SMESH_Mesh&          aMesh,
                                     const TopoDS_Shape& aShape)
{
#ifdef WITH_SMESH_CANCEL_COMPUTE
  _compute_canceled = false;
#endif
  bool Ok = true;
  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  TCollection_AsciiString hexahedraMessage;

  if (_iShape == 0 && _nbShape == 0) {
    _nbShape = countShape( meshDS, TopAbs_SOLID );  // we count the number of shapes
    //_tabNode = new SMDS_MeshNode*[_nbShape];        // we declare the size of the node array
  }

  // to prevent from displaying error message after computing,
  // SetIsAlwaysComputed( true ) to empty sub-meshes
  for ( int i = 0; i < _nbShape; ++i )
    if ( SMESH_subMesh* sm = aMesh.GetSubMeshContaining( aShape ))
    {
      SMESH_subMeshIteratorPtr smIt = sm->getDependsOnIterator(/*includeSelf=*/true,
                                                               /*complexShapeFirst=*/false);
      while ( smIt->more() )
      {
        sm = smIt->next();
        if ( !sm->IsMeshComputed() )
          sm->SetIsAlwaysComputed( true );
      }
    }

  _iShape++;

  if (_iShape == _nbShape ) {

    // for (int i=0; i<_nbShape; i++)        // we destroy the (_nbShape - 1) nodes created and used
    //   meshDS->RemoveNode( _tabNode[i] );  // to simulate successful mesh computing.
    // delete [] _tabNode;

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
    TCollection_AsciiString aTmpDir = TCollection_AsciiString(_hexoticWorkingDirectory.c_str());
#ifdef WIN32
    if ( aTmpDir.Value(aTmpDir.Length()) != '\\' ) aTmpDir += '\\';
#else
    if ( aTmpDir.Value(aTmpDir.Length()) != '/' ) aTmpDir += '/';
#endif
    TCollection_AsciiString Hexotic_In(""), Hexotic_Out;
    TCollection_AsciiString modeFile_In( "chmod 666 " ), modeFile_Out( "chmod 666 " );
    TCollection_AsciiString aLogFileName = aTmpDir + "Hexotic.log";    // log

    std::map <int,int> aSmdsToHexoticIdMap;
    std::map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

    Hexotic_Out = aTmpDir + "Hexotic_Out.mesh";
#ifdef WITH_BLSURFPLUGIN
    bool defaultInputFile = true;
    if (_blsurfHypo && !_blsurfHypo->GetQuadAllowed()) {
      Hexotic_In = TCollection_AsciiString(_blsurfHypo->GetGMFFile().c_str());
      if (Hexotic_In != "")
        defaultInputFile = false;
    }
    if (defaultInputFile) {
#endif
      Hexotic_In  = aTmpDir + "Hexotic_In.mesh";
      removeHexoticFiles(Hexotic_In, Hexotic_Out);
      cout << std::endl;
      cout << "Creating Hexotic input mesh file : " << Hexotic_In << std::endl;
      aMesh.ExportGMF(Hexotic_In.ToCString(), meshDS);
#ifdef WITH_BLSURFPLUGIN
    }
    else {
      removeFile( Hexotic_Out );
    }
#endif
    

    std::string run_Hexotic = getHexoticCommand(Hexotic_In, Hexotic_Out);
    run_Hexotic += std::string(" 1 > ") + aLogFileName.ToCString();  // dump into file

    cout << std::endl;
    cout << "Hexotic command : " << run_Hexotic << std::endl;

    modeFile_In += Hexotic_In;
    system( modeFile_In.ToCString() );
    aSmdsToHexoticIdMap.clear();
    aHexoticIdToNodeMap.clear();

    MESSAGE("HexoticPlugin_Hexotic::Compute");

    system( run_Hexotic.data() );

    // --------------
    // read a result
    // --------------

    std::ifstream fileRes( Hexotic_Out.ToCString() );
    modeFile_Out += Hexotic_Out;
    system( modeFile_Out.ToCString() );
    if ( ! fileRes.fail() ) {
      Ok = readResult( Hexotic_Out.ToCString(),
#ifdef WITH_SMESH_CANCEL_COMPUTE
                       this,
#endif
                       meshDS, _nbShape, tabShape, tabBox );
      if(Ok)
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
      else
        hexahedraMessage = "failed";
    }
    else {
      hexahedraMessage = "failed";
      cout << "Problem with Hexotic output file " << Hexotic_Out.ToCString() << std::endl;
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
    }
    cout << "Hexahedra meshing " << hexahedraMessage << std::endl;
    cout << std::endl;

    delete [] tabShape;
    for (int i=0; i<_nbShape; i++)
      delete [] tabBox[i];
    delete [] tabBox;
    _nbShape = 0;
    _iShape  = 0;
  }
#ifdef WITH_SMESH_CANCEL_COMPUTE
  if(_compute_canceled)
    return error(SMESH_Comment("interruption initiated by user"));
#endif
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
#ifdef WITH_SMESH_CANCEL_COMPUTE
  _compute_canceled = false;
#endif
/*
  SMESH_ComputeErrorPtr myError = SMESH_ComputeError::New();
*/
  bool Ok = true;
  TCollection_AsciiString hexahedraMessage;

  SetParameters(_hypothesis);

  TCollection_AsciiString aTmpDir = getTmpDir();
  TCollection_AsciiString Hexotic_In, Hexotic_Out;
  TCollection_AsciiString modeFile_In( "chmod 666 " ), modeFile_Out( "chmod 666 " );
  TCollection_AsciiString aLogFileName = aTmpDir + "Hexotic.log";    // log

  std::map <int,int> aSmdsToHexoticIdMap;
  std::map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

  Hexotic_In  = aTmpDir + "Hexotic_In.mesh";
  Hexotic_Out = aTmpDir + "Hexotic_Out.mesh";

  std::string run_Hexotic = getHexoticCommand(Hexotic_In, Hexotic_Out);
  run_Hexotic += std::string(" 1 > ") + aLogFileName.ToCString();  // dump into file

  removeHexoticFiles(Hexotic_In, Hexotic_Out);

  cout << std::endl;
  cout << "Creating Hexotic input mesh file : " << Hexotic_In << std::endl;
  aMesh.ExportGMF(Hexotic_In.ToCString(), aHelper->GetMeshDS());
  modeFile_In += Hexotic_In;
  system( modeFile_In.ToCString() );
  aSmdsToHexoticIdMap.clear();
  aHexoticIdToNodeMap.clear();

  MESSAGE("HexoticPlugin_Hexotic::Compute");

  cout << std::endl;
  cout << "Hexotic command : " << run_Hexotic << std::endl;
  system( run_Hexotic.data() );

  // --------------
  // read a result
  // --------------

  std::ifstream fileRes( Hexotic_Out.ToCString() );
  modeFile_Out += Hexotic_Out;
  system( modeFile_Out.ToCString() );
  if ( ! fileRes.fail() ) {
    Ok = readResult( Hexotic_Out.ToCString(),
#ifdef WITH_SMESH_CANCEL_COMPUTE
                     this,
#endif
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
    cout << "Problem with Hexotic output file " << Hexotic_Out << std::endl;
    // analyse log file
    SMESH_File logFile( aLogFileName.ToCString() );
    if ( !logFile.eof() )
    {
      char msgLic[] = " Dlim ";
      const char* fileBeg = logFile.getPos(), *fileEnd = fileBeg + logFile.size();
      if ( std::search( fileBeg, fileEnd, msgLic, msgLic+strlen(msgLic)) != fileEnd )
        return error("Licence problems.");
    }
    return error(SMESH_Comment("Problem with Hexotic output file ")<<Hexotic_Out);
  }
  cout << "Hexahedra meshing " << hexahedraMessage << std::endl;
  cout << std::endl;

#ifdef WITH_SMESH_CANCEL_COMPUTE
  if(_compute_canceled)
    return error(SMESH_Comment("interruption initiated by user"));
#endif
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

#ifdef WITH_SMESH_CANCEL_COMPUTE
void HexoticPlugin_Hexotic::CancelCompute()
{
  _compute_canceled = true;
#ifdef WNT
#else
  TCollection_AsciiString aTmpDir = getTmpDir();
  TCollection_AsciiString Hexotic_In = aTmpDir + "Hexotic_In.mesh";
  TCollection_AsciiString cmd = TCollection_AsciiString("ps ux | grep ") + Hexotic_In;
  cmd += TCollection_AsciiString(" | grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>&1");
  system( cmd.ToCString() );
#endif
}
#endif
