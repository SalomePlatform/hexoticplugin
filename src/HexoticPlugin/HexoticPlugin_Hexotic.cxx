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
// File   : HexoticPlugin_Hexotic.cxx
// Author : Lioka RAZAFINDRAZAKA (CEA)
// ---
//
#include "HexoticPlugin_Hexotic.hxx"
#include "HexoticPlugin_Hypothesis.hxx"
#include "MG_Hexotic_API.hxx"

#include "utilities.h"

#ifdef _DEBUG_
#define DUMP(txt) \
//  cout << txt
#else
#define DUMP(txt)
#endif

#include <SMESHDS_GroupBase.hxx>
#include <SMESHDS_Mesh.hxx>
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

#include <BRepBndLib.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <IntCurvesFace_Intersector.hxx>
#include <OSD_File.hxx>
#include <Poly_Triangulation.hxx>
#include <Precision.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopoDS.hxx>
#include <gp_Ax3.hxx>
#include <gp_Lin.hxx>
#include <gp_Pnt.hxx>

#include <Basics_Utils.hxx>
#include <GEOMImpl_Types.hxx>
#include <GEOM_wrap.hxx>

#define GMFVERSION GmfDouble
#define GMFDIMENSION 3

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
  _onlyUnaryInput = false;
  _requireShape = false;
  _iShape=0;
  _nbShape=0;
  _hexoticFilesKept=false;
  _compatibleHypothesis.push_back( HexoticPlugin_Hypothesis::GetHypType() );
#ifdef WITH_BLSURFPLUGIN
  _blsurfHypo = NULL;
#endif
  _computeCanceled = false;
  
  // Copy of what is done in BLSURFPLugin TODO : share the code
  smeshGen_i = SMESH_Gen_i::GetSMESHGen();
  CORBA::Object_var anObject = smeshGen_i->GetNS()->Resolve("/myStudyManager");
  SALOMEDS::StudyManager_var aStudyMgr = SALOMEDS::StudyManager::_narrow(anObject);
  
  myStudy = NULL;
  myStudy = aStudyMgr->GetStudyByID(_studyId);
  if ( !myStudy->_is_nil() )
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

static int getNbShape(MG_Hexotic_API* hexaOutput, int iMesh, GmfKwdCod what, int defaultValue=0)
{
  int number = hexaOutput->GmfStatKwd( iMesh, what );
  if ( number > 0 )
  {
  // std::string aLine;
  // std::ifstream file(aFile.c_str());
  // while ( !file.eof() ) {
  //   getline( file, aLine);
  //   if ( aLine == aString ) {
  //     getline( file, aLine);
  //     std::istringstream stringFlux( aLine );
  //     stringFlux >> number;
      number = ( number + defaultValue + std::abs(number - defaultValue) ) / 2;
    //   break;
    // }
  }
  else
  {
    number = defaultValue;
  }
  //file.close();
  return number;
}

//=======================================================================
//function : countShape
//purpose  :
//=======================================================================

static int countShape( SMESH_Mesh* mesh, TopAbs_ShapeEnum shape )
{
  if ( !mesh->HasShapeToMesh() )
    return 0;
  TopExp_Explorer expShape ( mesh->GetShapeToMesh(), shape );
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
void getShape(Mesh* mesh, Shape shape, Tab *t_Shape)
{
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
//function : writeInput
//purpose  : pass a mesh to input of MG-Hexa
//=======================================================================

static void writeInput(MG_Hexotic_API*     theHexaInput,
                       const char*         theFile,
                       const SMESHDS_Mesh* theMeshDS)
{
  int meshID = theHexaInput->GmfOpenMesh( theFile, GmfWrite, GMFVERSION, GMFDIMENSION);
  
  // nodes
  int iN = 0, nbNodes = theMeshDS->NbNodes();
  theHexaInput->GmfSetKwd( meshID, GmfVertices, nbNodes );
  std::map< const SMDS_MeshNode*, int, TIDCompare > node2IdMap;
  SMDS_NodeIteratorPtr nodeIt = theMeshDS->nodesIterator();
  SMESH_TNodeXYZ n;
  while ( nodeIt->more() )
  {
    n.Set( nodeIt->next() );
    theHexaInput->GmfSetLin( meshID, GmfVertices, n.X(), n.Y(), n.Z(), n._node->getshapeId() );
    node2IdMap.insert( node2IdMap.end(), std::make_pair( n._node, ++iN ));
  }

  // edges
  SMDS_ElemIteratorPtr elemIt = theMeshDS->elementsIterator( SMDSAbs_Edge );
  if ( elemIt->more() )
  {
    int nbEdges = theMeshDS->GetMeshInfo().NbElements( SMDSAbs_Edge );
    theHexaInput->GmfSetKwd(meshID, GmfEdges, nbEdges );
    for ( int gmfID = 1; elemIt->more(); ++gmfID )
    {
      const SMDS_MeshElement* edge = elemIt->next();
      theHexaInput->GmfSetLin(meshID, GmfEdges, 
                              node2IdMap[ edge->GetNode( 0 )],
                              node2IdMap[ edge->GetNode( 1 )],
                              edge->getshapeId() );
    }
  }

  // triangles
  elemIt = theMeshDS->elementGeomIterator( SMDSGeom_TRIANGLE );
  if ( elemIt->more() )
  {
    int nbTria = theMeshDS->GetMeshInfo().NbElements( SMDSGeom_TRIANGLE );
    theHexaInput->GmfSetKwd(meshID, GmfTriangles, nbTria );
    for ( int gmfID = 1; elemIt->more(); ++gmfID )
    {
      const SMDS_MeshElement* tria = elemIt->next();
      theHexaInput->GmfSetLin(meshID, GmfTriangles, 
                              node2IdMap[ tria->GetNode( 0 )],
                              node2IdMap[ tria->GetNode( 1 )],
                              node2IdMap[ tria->GetNode( 2 )],
                              tria->getshapeId() );
    }
  }
  theHexaInput->GmfCloseMesh( meshID );
}

//=======================================================================
//function : readResult
//purpose  : Read GMF file in case of a mesh with geometry
//=======================================================================

static bool readResult(MG_Hexotic_API*       theHexaOutput,
                       const char*           theFile,
                       HexoticPlugin_Hexotic*theAlgo,
                       SMESH_MesherHelper*   theHelper,
                       const int             nbShape = 0,
                       const TopoDS_Shape*   tabShape = 0,
                       double**              tabBox = 0)
{
  SMESH_Mesh*     theMesh = theHelper->GetMesh();
  SMESHDS_Mesh* theMeshDS = theHelper->GetMeshDS();

  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  TopoDS_Shape aShape;
  TopoDS_Vertex aVertex;
  std::string token;
  int shapeID, hexoticShapeID;
  const int IdShapeRef = 2;
  int *tabID;
  double epsilon = Precision::Confusion();
  std::map <std::string,int> mapField;
  SMDS_MeshNode** HexoticNode;
  TopoDS_Shape *tabCorner;

  const int nbDomains = countShape( theMesh, TopAbs_SHELL );
  const int holeID = -1;

  if ( nbDomains > 0 )
  {
    tabID    = new int[nbDomains];

    for (int i=0; i<nbDomains; i++)
      tabID[i] = 0;
    if ( nbDomains == 1 )
      tabID[0] = theMeshDS->ShapeToIndex( tabShape[0] );
  }

  SMDS_ElemIteratorPtr eIt = theMeshDS->elementsIterator();
  while( eIt->more() )
    theMeshDS->RemoveFreeElement( eIt->next(), /*sm=*/0 );
  SMDS_NodeIteratorPtr nIt = theMeshDS->nodesIterator();
  while ( nIt->more() )
    theMeshDS->RemoveFreeNode( nIt->next(), /*sm=*/0 );

  int ver, dim;
  int meshID = theHexaOutput->GmfOpenMesh( theFile, GmfRead, &ver, &dim );

  int nbVertices  = getNbShape(theHexaOutput, meshID, GmfVertices );
  int nbCorners   = getNbShape(theHexaOutput, meshID, GmfCorners, countShape( theMesh, TopAbs_VERTEX ));
  if ( nbVertices == 0 )
    return false;

  tabCorner   = new TopoDS_Shape[ nbCorners ];
  HexoticNode = new SMDS_MeshNode*[ nbVertices + 1 ];

  getShape(theMeshDS, TopAbs_VERTEX, tabCorner);

  int nbNodes = theHexaOutput->GmfStatKwd( meshID, GmfVertices );
  if ( nbNodes > 0 )
  {
    theHexaOutput->GmfGotoKwd( meshID, GmfVertices );
    double x,y,z;
    for ( int aHexoticID = 1; aHexoticID <= nbNodes; ++aHexoticID )
    {
      if ( theAlgo->computeCanceled() )
        return false;
      theHexaOutput->GmfGetLin( meshID, GmfVertices, &x, &y, &z, &shapeID );
      HexoticNode[ aHexoticID ] = theHelper->AddNode( x,y,z );
    }
  }

  int nodeID[8];
  SMDS_MeshNode* node[8];
  SMDS_MeshElement * aHexoticElement;

  nbCorners = theHexaOutput->GmfStatKwd( meshID, GmfCorners );
  if ( nbCorners > 0 && nbDomains > 0 )
  {
    theHexaOutput->GmfGotoKwd( meshID, GmfCorners );
    for ( int iElem = 0; iElem < nbCorners; iElem++ )
    {
      if ( theAlgo->computeCanceled() )
        return false;
      theHexaOutput->GmfGetLin( meshID, GmfCorners, &nodeID[0] );
      node[0] = HexoticNode[ nodeID[0] ];
      gp_Pnt HexoticPnt ( node[0]->X(), node[0]->Y(), node[0]->Z() );
      for ( int i = 0; i < nbCorners; i++ )
      {
        aVertex = TopoDS::Vertex( tabCorner[i] );
        gp_Pnt aPnt = BRep_Tool::Pnt( aVertex );
        if ( aPnt.Distance( HexoticPnt ) < epsilon )
        {
          theMeshDS->SetNodeOnVertex( node[0], aVertex );
          break;
        }
      }
    }
  }

  int nbEdges = theHexaOutput->GmfStatKwd( meshID, GmfEdges );
  if ( nbEdges > 0 )
  {
    theHexaOutput->GmfGotoKwd( meshID, GmfEdges );
    for ( int iElem = 0; iElem < nbEdges; iElem++ )
    {
      if ( theAlgo->computeCanceled() )
        return false;
      theHexaOutput->GmfGetLin( meshID, GmfEdges, &nodeID[0], &nodeID[1], &shapeID );
      for ( int i = 0; i < 2; ++i )
      {
        node[i] = HexoticNode[ nodeID[i]];
        if ( node[i]->getshapeId() < 1 )
          theMeshDS->SetNodeOnEdge( node[i], shapeID );
      }
      aHexoticElement = theHelper->AddEdge( node[0], node[1] );
      if ( aHexoticElement->getshapeId() < 1 )
        theMeshDS->SetMeshElementOnShape( aHexoticElement, shapeID );
    }
  }

  int nbQuad = theHexaOutput->GmfStatKwd( meshID, GmfQuadrilaterals );
  if ( nbQuad > 0 )
  {
    theHexaOutput->GmfGotoKwd( meshID, GmfQuadrilaterals );
    for ( int iElem = 0; iElem < nbQuad; iElem++ )
    {
      if ( theAlgo->computeCanceled() )
        return false;
      theHexaOutput->GmfGetLin( meshID, GmfQuadrilaterals,
                                &nodeID[0], &nodeID[1], &nodeID[2], &nodeID[3], &shapeID );
      for ( int i = 0; i < 4; ++i )
      {
        node[i] = HexoticNode[ nodeID[i]];
        if ( shapeID > 0 && node[i]->getshapeId() < 1 )
          theMeshDS->SetNodeOnFace( node[i], shapeID );
      }
      aHexoticElement = theHelper->AddFace( node[0], node[1], node[2], node[3] );
      if ( shapeID > 0 && aHexoticElement->getshapeId() < 1 )
        theMeshDS->SetMeshElementOnShape( aHexoticElement, shapeID );
    }
  }

  int nbHexa = theHexaOutput->GmfStatKwd( meshID, GmfHexahedra );
  if ( nbHexa > 0 )
  {
    theHexaOutput->GmfGotoKwd( meshID, GmfHexahedra );
    for ( int iElem = 0; iElem < nbHexa; iElem++ )
    {
      if ( theAlgo->computeCanceled() )
        return false;
      theHexaOutput->GmfGetLin( meshID, GmfHexahedra,
                                &nodeID[0], &nodeID[1], &nodeID[2], &nodeID[3],
                                &nodeID[4], &nodeID[5], &nodeID[6], &nodeID[7],
                                &shapeID );
      for ( int i = 0; i < 8; ++i )
      {
        node[i] = HexoticNode[ nodeID[i]];
      }
      if ( nbDomains > 1 ) {
        hexoticShapeID = shapeID - IdShapeRef;
        if ( tabID[ hexoticShapeID ] == 0 ) {
          aShape = findShape(node, aShape, tabShape, tabBox, nbShape);
          shapeID = aShape.IsNull() ? holeID : theMeshDS->ShapeToIndex( aShape );
          tabID[ hexoticShapeID ] = shapeID;
        }
        else {
          shapeID = tabID[ hexoticShapeID ];
        }
        if ( iElem == ( nbHexa - 1) ) {
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
      {
        for ( int i = 0; i < 8; ++i )
        {
          if ( node[i]->getshapeId() < 1 )
            theMeshDS->SetNodeInVolume( node[i], shapeID );
        }
        aHexoticElement = theHelper->AddVolume( node[0], node[3], node[2], node[1],
                                                node[4], node[7], node[6], node[5]);
        if ( aHexoticElement->getshapeId() < 1 )
          theMeshDS->SetMeshElementOnShape( aHexoticElement, shapeID );
      }
    }
  }
  cout << std::endl;

  // remove nodes in holes
  if ( nbDomains > 1 )
  {
    SMESHDS_SubMesh* subMesh = 0;
    for ( int i = 1; i <= nbNodes; ++i )
      if ( HexoticNode[i]->NbInverseElements() == 0 )
      {
        theMeshDS->RemoveFreeNode( HexoticNode[i], subMesh, /*fromGroups=*/false );
      }
  }
  delete [] tabID;
  delete [] tabCorner;
  delete [] HexoticNode;
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
    _textOptions = hyp->GetAdvancedOption();
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
  std::string uname = std::string(getenv("USERNAME"));
  replace(uname.begin(), uname.end(), ' ', '_');
  aSuffix += uname.c_str();
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
                                                     const TCollection_AsciiString& Hexotic_SizeMap_Prefix,
                                                     const bool                     forExecutable) const
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

  TCollection_AsciiString run_Hexotic("mg-hexa.exe");

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
  
  if (_sizeMaps.begin() != _sizeMaps.end() && forExecutable )
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
  if ( forExecutable )
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
  if ( myStudy->_is_nil() )
    throw SALOME_Exception("MG-Hexa plugin can't work w/o publishing in the study");
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
std::vector<std::string> HexoticPlugin_Hexotic::writeSizeMapFile( MG_Hexotic_API* mgOutput,
                                                                  std::string     sizeMapPrefix )
{
  HexoticPlugin_Hypothesis::THexoticSizeMaps::iterator it;
  
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

  std::string myVerticesFile = sizeMapPrefix + ".mesh";
  std::string      mySolFile = sizeMapPrefix + ".sol";
  
  // Open files
  int verticesFileID =
    mgOutput->GmfOpenMesh( myVerticesFile.c_str(), GmfWrite, GMFVERSION, GMFDIMENSION );  
  int solFileID =
    mgOutput->GmfOpenMesh( mySolFile.c_str(), GmfWrite, GMFVERSION, GMFDIMENSION );
  
  int pointsNumber = points.size();
  
  // Vertices Keyword
  mgOutput->GmfSetKwd( verticesFileID, GmfVertices, pointsNumber );
  // SolAtVertices Keyword
  int TypTab[] = {GmfSca};
  mgOutput->GmfSetKwd(solFileID, GmfSolAtVertices, pointsNumber, 1, TypTab);
  
  // Read the control points information from the vector and write it into the files
  double ValTab[1];
  std::vector<Control_Pnt>::const_iterator points_it;
  for (points_it = points.begin(); points_it != points.end(); points_it++ )
  {
    mgOutput->GmfSetLin( verticesFileID, GmfVertices, points_it->X(), points_it->Y(), points_it->Z(), 0 );
    ValTab[0] = points_it->Size();
    mgOutput->GmfSetLin( solFileID, GmfSolAtVertices, ValTab);
  }

  // Close Files
  mgOutput->GmfCloseMesh( verticesFileID );
  mgOutput->GmfCloseMesh( solFileID );

  std::vector<std::string> fileNames(2);
  fileNames[0] = myVerticesFile;
  fileNames[1] = mySolFile;

  return fileNames;
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
  _computeCanceled = false;
  bool Ok = true;
  SMESHDS_Mesh* meshDS = aMesh.GetMeshDS();
  TCollection_AsciiString hexahedraMessage;

  _nbShape = countShape( &aMesh, TopAbs_SOLID );  // we count the number of shapes

  {
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

    TCollection_AsciiString aTmpDir = _hexoticWorkingDirectory.c_str();
    TCollection_AsciiString aQuote("");
#ifdef WIN32
    aQuote = "\"";
    if ( aTmpDir.Value(aTmpDir.Length()) != '\\' ) aTmpDir += '\\';
#else
    if ( aTmpDir.Value(aTmpDir.Length()) != '/' ) aTmpDir += '/';
#endif
    TCollection_AsciiString Hexotic_In(""), Hexotic_Out, Hexotic_SizeMap_Prefix;
    TCollection_AsciiString modeFile_In( "chmod 666 " ), modeFile_Out( "chmod 666 " );
    TCollection_AsciiString aLogFileName = aTmpDir + "Hexotic"+getSuffix()+".log";    // log

    std::map <int,int> aSmdsToHexoticIdMap;
    std::map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

    MG_Hexotic_API mgHexa( _computeCanceled, _progress );

    Hexotic_Out = aTmpDir + "Hexotic"+getSuffix()+"_Out.mesh";
#ifdef WITH_BLSURFPLUGIN
    bool defaultInputFile = true;
    if (_blsurfHypo && !_blsurfHypo->GetQuadAllowed()) {
      Hexotic_In = _blsurfHypo->GetGMFFile().c_str();
      if ( !Hexotic_In.IsEmpty() &&
           SMESH_File( _blsurfHypo->GetGMFFile() ).exists() )
      {
        mgHexa.SetUseExecutable();
        defaultInputFile = false;
      }
    }
    if (defaultInputFile) {
#endif
      Hexotic_In  = aTmpDir + "Hexotic"+getSuffix()+"_In.mesh";
      removeHexoticFiles(Hexotic_In, Hexotic_Out);
      splitQuads(aMesh); // quadrangles are no longer acceptable as input
      if ( mgHexa.IsExecutable() )
      {
        cout << std::endl;
        cout << "Creating MG-Hexa input mesh file : " << Hexotic_In << std::endl;
      }
      writeInput( &mgHexa, Hexotic_In.ToCString(), meshDS );
#ifdef WITH_BLSURFPLUGIN
    }
    else {
      removeFile( Hexotic_Out );
    }
#endif
    
    Hexotic_SizeMap_Prefix = aTmpDir + "Hexotic_SizeMap" + getSuffix();
    std::vector<std::string> sizeMapFiles = writeSizeMapFile( &mgHexa, Hexotic_SizeMap_Prefix.ToCString() );
    
    std::string run_Hexotic = getHexoticCommand(aQuote + Hexotic_In + aQuote, aQuote + Hexotic_Out + aQuote, Hexotic_SizeMap_Prefix, mgHexa.IsExecutable() );
    run_Hexotic += std::string(" 1> ") + aQuote.ToCString() + aLogFileName.ToCString() + aQuote.ToCString();  // dump into file
    mgHexa.SetLogFile( aLogFileName.ToCString() );
    cout << "Creating MG-Hexa log file : " << aLogFileName << std::endl;

    cout << std::endl;
    cout << "MG-Hexa command : " << run_Hexotic << std::endl;

    if ( mgHexa.IsExecutable() )
    {
#ifndef WIN32    
      modeFile_In += Hexotic_In;
      system( modeFile_In.ToCString() );
#endif
    }
    aSmdsToHexoticIdMap.clear();
    aHexoticIdToNodeMap.clear();

    MESSAGE("HexoticPlugin_Hexotic::Compute");

    
    std::string errStr;
    Ok = mgHexa.Compute( run_Hexotic, errStr ); // run


    // --------------
    // read a result
    // --------------

    if ( mgHexa.IsExecutable() )
    {
#ifndef WIN32
      modeFile_Out += Hexotic_Out;
      system( modeFile_Out.ToCString() );
#endif
    }
    SMESH_MesherHelper aHelper( aMesh );

    Ok = readResult( &mgHexa, Hexotic_Out.ToCString(),
                     this,
                     &aHelper, _nbShape, tabShape, tabBox );

    std::string log = mgHexa.GetLog();
    if ( Ok )
    {
      hexahedraMessage = "success";
#ifndef _DEBUG_
      removeFile(Hexotic_Out);
      removeFile(Hexotic_In);
      //removeFile(aLogFileName);
      for( size_t i=0; i<sizeMapFiles.size(); i++)
      {
        removeFile( TCollection_AsciiString( sizeMapFiles[i].c_str() ) );
      }
#endif
    }
    else
    {
      hexahedraMessage = "failed";
      if ( mgHexa.IsExecutable() )
        cout << "Problem with MG-Hexa output file " << Hexotic_Out.ToCString() << std::endl;
      // analyse log file
      if ( !log.empty() )
      {
        char msgLic[] = " Dlim ";
        std::string log = mgHexa.GetLog();
        const char* fileBeg = &log[0], *fileEnd = fileBeg + log.size();
        if ( std::search( fileBeg, fileEnd, msgLic, msgLic+strlen(msgLic)) != fileEnd )
          error("Licence problems.");
      }
      if ( !errStr.empty() )
        error(errStr);
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

  if(_computeCanceled)
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
  _computeCanceled = false;
/*
  SMESH_ComputeErrorPtr myError = SMESH_ComputeError::New();
*/
  bool Ok = true;
  TCollection_AsciiString hexahedraMessage;
  TCollection_AsciiString aQuote("");
#ifdef WIN32
    aQuote = "\"";
#endif
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
 
  MG_Hexotic_API mgHexa( _computeCanceled, _progress );

  std::vector<std::string> sizeMapFiles = writeSizeMapFile( &mgHexa, Hexotic_SizeMap_Prefix.ToCString() );

  std::string run_Hexotic = getHexoticCommand(aQuote + Hexotic_In + aQuote, aQuote + Hexotic_Out + aQuote, Hexotic_SizeMap_Prefix, mgHexa.IsExecutable());
  run_Hexotic += std::string(" 1> ") + aQuote.ToCString() + aLogFileName.ToCString() + aQuote.ToCString();  // dump into file
  mgHexa.SetLogFile( aLogFileName.ToCString() );
  cout << "Creating MG-Hexa log file : " << aLogFileName << std::endl;

  removeHexoticFiles(Hexotic_In, Hexotic_Out);

  splitQuads(aMesh); // quadrangles are no longer acceptable as input

  cout << std::endl;
  cout << "Creating MG-Hexa input mesh file : " << Hexotic_In << std::endl;
  writeInput( &mgHexa, Hexotic_In.ToCString(), aHelper->GetMeshDS() );
  if ( mgHexa.IsExecutable() )
  {
#ifndef WIN32    
    modeFile_In += Hexotic_In;
    system( modeFile_In.ToCString() );
#endif
  }
  aSmdsToHexoticIdMap.clear();
  aHexoticIdToNodeMap.clear();

  MESSAGE("HexoticPlugin_Hexotic::Compute");

  cout << std::endl;
  cout << "MG-Hexa command : " << run_Hexotic << std::endl;

  std::string errStr;
  Ok = mgHexa.Compute( run_Hexotic, errStr ); // run

  // --------------
  // read a result
  // --------------

  if ( mgHexa.IsExecutable() )
  {
    modeFile_Out += Hexotic_Out;
    system( modeFile_Out.ToCString() );
  }

  Ok = readResult( &mgHexa, Hexotic_Out.ToCString(),
                   this,
                   aHelper );

  std::string log = mgHexa.GetLog();
  if ( Ok )
  {
    hexahedraMessage = "success";
  }
  else
  {
    hexahedraMessage = "failed";
    if ( mgHexa.IsExecutable() )
      cout << "Problem with MG-Hexa output file " << Hexotic_Out << std::endl;
    // analyse log file
    if ( !log.empty() )
    {
      char msgLic[] = " Dlim ";
      const char* fileBeg = &log[0], *fileEnd = fileBeg + log.size();
      if ( std::search( fileBeg, fileEnd, msgLic, msgLic+strlen(msgLic)) != fileEnd )
        error("Licence problems.");
    }
    if ( !errStr.empty() )
      error(errStr);
  }
  cout << "Hexahedra meshing " << hexahedraMessage << std::endl;
  cout << std::endl;

  if(_computeCanceled)
    return error(SMESH_Comment("interruption initiated by user"));
  removeFile(Hexotic_Out);
  removeFile(Hexotic_In);
  removeFile(aLogFileName);
  for( size_t i=0; i<sizeMapFiles.size(); i++)
  {
    removeFile( TCollection_AsciiString(sizeMapFiles[i].c_str()) );
  }
  return Ok;
}

//=============================================================================
/*!
 *
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Evaluate(SMESH_Mesh&         aMesh,
                                     const TopoDS_Shape& aShape,
                                     MapShapeNbElems&    aResMap)
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
  _computeCanceled = true;
#ifdef WIN32
#else
  TCollection_AsciiString aTmpDir = _hexoticWorkingDirectory.c_str(); //getTmpDir();
  TCollection_AsciiString Hexotic_In = aTmpDir + "Hexotic_In.mesh";
  TCollection_AsciiString cmd = TCollection_AsciiString("ps ux | grep ") + Hexotic_In;
  cmd += TCollection_AsciiString(" | grep -v grep | awk '{print $2}' | xargs kill -9 > /dev/null 2>&1");
  system( cmd.ToCString() );
#endif
}
