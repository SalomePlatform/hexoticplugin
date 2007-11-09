//  HexoticPlugin : C++ implementation
//
//  Copyright (C) 2006  OPEN CASCADE, CEA/DEN, EDF R&D
// 
//  This library is free software; you can redistribute it and/or 
//  modify it under the terms of the GNU Lesser General Public 
//  License as published by the Free Software Foundation; either 
//  version 2.1 of the License. 
// 
//  This library is distributed in the hope that it will be useful, 
//  but WITHOUT ANY WARRANTY; without even the implied warranty of 
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
//  Lesser General Public License for more details. 
// 
//  You should have received a copy of the GNU Lesser General Public 
//  License along with this library; if not, write to the Free Software 
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
// 
//  See http://www.opencascade.org/SALOME/ or email : webmaster.salome@opencascade.org 
//
//
// File    : HexoticPlugin_Hexotic.cxx
// Author  : Lioka RAZAFINDRAZAKA (CEA)
// Date    : 2006/06/30
// Project : SALOME
//=============================================================================
using namespace std;

#include "HexoticPlugin_Hexotic.hxx"
#include "HexoticPlugin_Hypothesis.hxx"
// #include "HexoticPlugin_Mesher.hxx"

#include "SMDS_MeshElement.hxx"
#include "SMDS_MeshNode.hxx"

#include <TopExp_Explorer.hxx>
#include <OSD_File.hxx>

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

#include <SMESH_Gen.hxx>
#include <SMESHDS_Mesh.hxx>
#include <SMESH_ControlsDef.hxx>

#include <list>
#include <cstdlib>
#include <iostream>

#include <BRepClass3d_SolidClassifier.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Precision.hxx>

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
  _iShape=0;
  _nbShape=0;
  _nodeRefNumber=0;
  _compatibleHypothesis.push_back("Hexotic_Parameters");
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

  list<const SMESHDS_Hypothesis*>::const_iterator itl;
  const SMESHDS_Hypothesis* theHyp;

  const list<const SMESHDS_Hypothesis*>& hyps = GetUsedHypothesis(aMesh, aShape);
  int nbHyp = hyps.size();
  if (!nbHyp)
  {
    aStatus = SMESH_Hypothesis::HYP_OK;
    return true;  // can work with no hypothesis
  }

  itl = hyps.begin();
  theHyp = (*itl); // use only the first hypothesis

  string hypName = theHyp->GetName();
  if (hypName == "Hexotic_Parameters")
  {
    _hypothesis = static_cast<const HexoticPlugin_Hypothesis*> (theHyp);
    ASSERT(_hypothesis);
    aStatus = SMESH_Hypothesis::HYP_OK;
  }
  else
    aStatus = SMESH_Hypothesis::HYP_INCOMPATIBLE;

  return aStatus == SMESH_Hypothesis::HYP_OK;
}

//=======================================================================
//function : writeHexoticFile
//purpose  : 
//=======================================================================

static bool writeHexoticFile (ofstream &                      theFile,
                             SMESHDS_Mesh *                   theMesh,
                             map <int,int> &                  theSmdsToHexoticIdMap,
                             map <int,const SMDS_MeshNode*> & theHexoticIdToNodeMap,
                             const TCollection_AsciiString &  Hexotic_In) {
  cout << endl;
  cout << "Creating Hexotic processed mesh file : " << Hexotic_In << endl;

  int nbShape = 0;

  TopExp_Explorer expface(theMesh->ShapeToMesh(), TopAbs_FACE);
  for ( ; expface.More(); expface.Next() )
    nbShape++;

  int tabID[nbShape];
  TopoDS_Shape tabShape[nbShape], aShape;
  int shapeID;
  bool idFound;

  int nbVertices    = 0;
  int nbTriangles   = 0;
  const char* space = "  ";
  int dummy_1D      = 0;
  int dummy_2D;

  int aSmdsNodeID = 1;
  const SMDS_MeshNode* aNode;
  SMDS_NodeIteratorPtr itOnNode;

  list< const SMDS_MeshElement* > faces;
  list< const SMDS_MeshElement* >::iterator itListFace;
  const SMDS_MeshElement* aFace;
  SMESHDS_SubMesh* theSubMesh;
  map<int,int>::const_iterator itOnSmdsNode;
  SMDS_ElemIteratorPtr itOnSubNode, itOnSubFace;

// Writing SMESH points into Hexotic File

  nbVertices = theMesh->NbNodes();

  theFile << "MeshVersionFormatted 1" << endl;
  theFile << endl;
  theFile << "Dimension" << endl;
  theFile << 3 << endl;
  theFile << "# Set of mesh vertices" << endl;
  theFile << "Vertices" << endl;
  theFile << nbVertices << endl;

  itOnNode = theMesh->nodesIterator();
  while ( itOnNode->more() ) {
      aNode = itOnNode->next();
      theSmdsToHexoticIdMap.insert( map <int,int>::value_type( aNode->GetID(), aSmdsNodeID ));
      theHexoticIdToNodeMap.insert (map <int,const SMDS_MeshNode*>::value_type( aSmdsNodeID, aNode ));
      aSmdsNodeID++;
      theFile << aNode->X() << space << aNode->Y() << space << aNode->Z() << space << dummy_1D << endl;
      }

// Writing SMESH faces into Hexotic File

  nbTriangles = theMesh->NbFaces();

  theFile << endl;
  theFile << "# Set of mesh triangles (v1,v2,v3,tag)" << endl;
  theFile << "Triangles" << endl;
  theFile << nbTriangles << endl;

  expface.ReInit();
  for ( int i = 0; expface.More(); expface.Next(), i++ ) {
    tabID[i] = 0;
    aShape   = expface.Current();
    shapeID  = theMesh->ShapeToIndex( aShape );
    idFound  = false;
    for ( int j=0; j<=i; j++) {
      if ( shapeID == tabID[j] ) {
        idFound = true;
        break;
      }
    }
    if ( not idFound ) {
      tabID[i]    = shapeID;
      tabShape[i] = aShape;
    }
  }
  for ( int i=0; i<nbShape; i++ ) {
    if ( not (tabID[i] == 0) ) {
      aShape      = tabShape[i];
      shapeID     = tabID[i];
      theSubMesh  = theMesh->MeshElements( aShape );
      itOnSubFace = theSubMesh->GetElements();
      while ( itOnSubFace->more() ) {
        aFace    = itOnSubFace->next();
        dummy_2D = shapeID;
        itOnSubNode = aFace->nodesIterator();
        while ( itOnSubNode->more() ) {
          aSmdsNodeID  = itOnSubNode->next()->GetID();
          itOnSmdsNode = theSmdsToHexoticIdMap.find( aSmdsNodeID );
          ASSERT( itOnSmdsNode != theSmdsToHexoticIdMap.end() );
          theFile << (*itOnSmdsNode).second << space;
        }
        theFile << dummy_2D << endl;
      }
    }
  }

  theFile << endl;
  theFile << "End" << endl;

  cout << "Processed mesh file created, it contains :" << endl;
  cout << "    " << nbVertices  << " vertices"  << endl;
  cout << "    " << nbTriangles << " triangles" << endl;
  cout << endl;

  return true;
}

//=======================================================================
//function : findShape
//purpose  :
//=======================================================================

static TopoDS_Shape findShape(const SMDS_MeshNode *aNode[],
                              TopoDS_Shape        aShape,
                              const TopoDS_Shape  shape[],
                              const double        box[][6],
                              const int           nShape) {

  Standard_Real PX, PY, PZ;
  int iShape;

  PX = ( aNode[0]->X() + aNode[1]->X() + aNode[2]->X() + aNode[3]->X() ) / 4.0;
  PY = ( aNode[0]->Y() + aNode[1]->Y() + aNode[2]->Y() + aNode[3]->Y() ) / 4.0;
  PZ = ( aNode[0]->Z() + aNode[1]->Z() + aNode[2]->Z() + aNode[3]->Z() ) / 4.0;
  gp_Pnt aPnt(PX, PY, PZ);

  BRepClass3d_SolidClassifier SC (aShape, aPnt, Precision::Confusion());
  if ( not(SC.State() == TopAbs_IN) ) {
    for (iShape = 0; iShape < nShape; iShape++) {
      aShape = shape[iShape];
      if ( not( PX < box[iShape][0] || box[iShape][1] < PX ||
                PY < box[iShape][2] || box[iShape][3] < PY ||
                PZ < box[iShape][4] || box[iShape][5] < PZ) ) {
        BRepClass3d_SolidClassifier SC (aShape, aPnt, Precision::Confusion());
        if (SC.State() == TopAbs_IN)
          break;
      }
    }
  }
  return aShape;
}

//=======================================================================
//function : readResult
//purpose  : 
//=======================================================================

static bool readResult(string                           theFile,
                       SMESHDS_Mesh *                   theMesh,
                       map <int,const SMDS_MeshNode*> & theHexoticIdToNodeMap,
                       int &                            nodeRefNumber,
                       const int                        nbShape,
                       TopoDS_Shape                     tabShape[],
                       double                           tabBox[][6])
{
  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  MESSAGE("Read " << theFile << " file");
  ifstream fileRes(theFile.c_str());
  ASSERT(fileRes);

  string token, theField;
  int EndOfFile = 0, nbElem = 0, nField = 10, nbRef = 0;
  int compoundID = theMesh->ShapeToIndex( theMesh->ShapeToMesh() );
  int aHexoticNodeID = 0, shapeID, hexoticShapeID;
  int IdShapeRef = 2;
  int tabID[nbShape];
  int tabRef [nField];
  map <string,int> mapField;
  TopoDS_Shape aShape;

  for (int i=0; i<nbShape; i++)
    tabID[i] = 0;

  mapField["MeshVersionFormatted"] = 0; tabRef[0] = 0;
  mapField["Dimension"]            = 1; tabRef[1] = 0;
  mapField["Vertices"]             = 2; tabRef[2] = 3;
  mapField["Edges"]                = 3; tabRef[3] = 2;
  mapField["Triangles"]            = 4; tabRef[4] = 3;
  mapField["Quadrilaterals"]       = 5; tabRef[5] = 4;
  mapField["Hexahedra"]            = 6; tabRef[6] = 8;
  mapField["Corners"]              = 7; tabRef[7] = 1;
  mapField["Ridges"]               = 8; tabRef[8] = 1;
  mapField["End"]                  = 9; tabRef[9] = 0;

  nodeRefNumber += theMesh->NbNodes();

  SMDS_NodeIteratorPtr itOnHexoticInputNode = theMesh->nodesIterator();
  while ( itOnHexoticInputNode->more() )
    theMesh->RemoveNode( itOnHexoticInputNode->next() );

  while ( EndOfFile == 0  ) {
    int dummy;
    fileRes >> token;

    if (mapField.count(token)) {
      nField = mapField[token];
      nbRef  = tabRef[nField];
    }
    else {
      nField = -1;
      nbRef = 0;
    }

    nbElem = 0;
    if ((nField < 9) && (nField >=0))
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
        MESSAGE("Read " << nbElem << " vertices");
        int aHexoticID;
        double coord[nbRef];
        SMDS_MeshNode * aHexoticNode;

        for ( int iElem = 0; iElem < nbElem; iElem++ ) {
          aHexoticID = iElem + 1 + nodeRefNumber;
          for ( int iCoord = 0; iCoord < 3; iCoord++ )
            fileRes >> coord[ iCoord ];
          fileRes >> dummy;
          aHexoticNode = theMesh->AddNode(coord[0], coord[1], coord[2]);
          theMesh->SetNodeInVolume( aHexoticNode, compoundID );
          theHexoticIdToNodeMap[ aHexoticID ] = aHexoticNode;
        }
        break;
      }
      case 3: // "Edges"
      case 5: // "Quadrilaterals"
      case 6: { // "Hexahedra"
        MESSAGE("Read " << nbElem << " " << token);
        const SMDS_MeshNode * node[nbRef];
        SMDS_MeshElement* aHexoticElement;
        map <int,const SMDS_MeshNode*>::iterator itOnHexoticNode;
            
        for ( int iElem = 0; iElem < nbElem; iElem++ ) {
          for ( int iRef = 0; iRef < nbRef; iRef++ ) {
            fileRes >> aHexoticNodeID;                          // read nbRef aHexoticNodeID
            aHexoticNodeID += nodeRefNumber;
            itOnHexoticNode = theHexoticIdToNodeMap.find( aHexoticNodeID );
            node[ iRef ] = itOnHexoticNode->second;
          }
          fileRes >> dummy;
          shapeID = compoundID;
          switch (nField) {
            case 3: { // "Edges"
              aHexoticElement = theMesh->AddEdge( node[0], node[1] );
              break;
            }
            case 5: { // "Quadrilaterals"
              aHexoticElement = theMesh->AddFace( node[0], node[1], node[2], node[3] );
              shapeID = dummy;
              break;
            }
            case 6: { // "Hexahedra"
              aHexoticElement = theMesh->AddVolume( node[0], node[3], node[2], node[1], node[4], node[7], node[6], node[5] );
              if ( nbShape > 1 ) {
                hexoticShapeID = dummy - IdShapeRef;
                if ( tabID[ hexoticShapeID ] == 0 ) {
                  if (iElem == 0)
                    aShape = tabShape[0];
                  aShape = findShape(node, aShape, tabShape, tabBox, nbShape);
                  shapeID = theMesh->ShapeToIndex( aShape );
                  tabID[ hexoticShapeID ] = shapeID;
                }
                else
                  shapeID = tabID[ hexoticShapeID ];
              }
              break;
            }
          }
          theMesh->SetMeshElementOnShape( aHexoticElement, shapeID );
        }
        break;
      }
      case 4: { // "Triangles"
        // MESSAGE("No action on token " << token << ", " << nbElem << " to read ") ;
        break;
      }
      case 7: { // "Corners"
        // MESSAGE("No action on token " << token << ", " << nbElem << " to read ") ;
        for ( int iElem = 0; iElem < nbElem; iElem++ )
          fileRes >> dummy;
        break;
      }
      case 8: { // "Ridges"
        // MESSAGE("No action on token " << token << ", " << nbElem << " to read ") ;
        for ( int iElem = 0; iElem < nbElem; iElem++ )
          fileRes >> dummy;
        break;
      }
      case 9: { // "End"
        EndOfFile = 1;
        MESSAGE("End of " << theFile << " file");
        break;
      }
      default: {
        MESSAGE("Unknown Token: " << token);
        // ASSERT(0);
      }
    }
  }
  cout << endl;
  return true;
}

//=============================================================================
/*!
 * Pass parameters to Hexotic
 */
//=============================================================================

void HexoticPlugin_Hexotic::SetParameters(const HexoticPlugin_Hypothesis* hyp) {
  if (hyp) {
    MESSAGE("HexoticPlugin_Hexotic::SetParameters");
    _hexesMinLevel = hyp->GetHexesMinLevel();
    _hexesMaxLevel = hyp->GetHexesMaxLevel();
    _hexoticQuadrangles = hyp->GetHexoticQuadrangles();
    _hexoticIgnoreRidges = hyp->GetHexoticIgnoreRidges();
    _hexoticInvalidElements = hyp->GetHexoticInvalidElements();
    _hexoticSharpAngleThreshold = hyp->GetHexoticSharpAngleThreshold();
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

//=============================================================================
/*!
 * Here we are going to use the Hexotic mesher
 */
//=============================================================================

bool HexoticPlugin_Hexotic::Compute(SMESH_Mesh&          theMesh,
                                     const TopoDS_Shape& theShape)
{
  bool Ok = true;
  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();
  TCollection_AsciiString hexahedraMessage;

  if (_iShape == 0 && _nbShape == 0) {
    TopExp_Explorer expf(meshDS->ShapeToMesh(), TopAbs_SOLID);
    for ( ; expf.More(); expf.Next() )
      _nbShape++;                             // we count the number of shapes
    _tabNode = new SMDS_MeshNode*[_nbShape];  // we declare the size of the node array
  }

  // to prevent from displaying error message after computing,
  // we need to create one node for each shape theShape.

  _tabNode[_iShape] = meshDS->AddNode(0, 0, 0);
  meshDS->SetNodeInVolume( _tabNode[_iShape], meshDS->ShapeToIndex(theShape) );

  _iShape++;

  if (_iShape == _nbShape ) {

    for (int i=0; i<_nbShape; i++)
      meshDS->RemoveNode( _tabNode[i] );  // we destroy the _nbShape nodes
    delete _tabNode;

    // create bounding box for each shape of the compound

    int iShape = 0;
    TopoDS_Shape tabShape[_nbShape];
    double tabBox[_nbShape][6];
    Standard_Real Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;

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

    cout << endl;
    cout << "Hexotic execution..." << endl;
    cout << _name << " parameters :" << endl;
    cout << "    " << _name << " Segments Min Level = " << _hexesMinLevel << endl;
    cout << "    " << _name << " Segments Max Level = " << _hexesMaxLevel << endl;
    cout << "    " << "Salome Quadrangles : " << (_hexoticQuadrangles ? "yes":"no") << endl;
    cout << "    " << "Hexotic can ignore ridges : " << (_hexoticIgnoreRidges ? "yes":"no") << endl;
    cout << "    " << "Hexotic authorize invalide elements : " << ( _hexoticInvalidElements ? "yes":"no") << endl;
    cout << "    " << _name << " Sharp angle threshold = " << _hexoticSharpAngleThreshold << " degrees" << endl;

    TCollection_AsciiString aTmpDir = getTmpDir();
    TCollection_AsciiString Hexotic_In, Hexotic_Out;
    TCollection_AsciiString run_Hexotic( "hexotic" );

    TCollection_AsciiString minl = " -minl ", maxl = " -maxl ", angle = " -ra ";
    TCollection_AsciiString in   = " -in ",   out  = " -out ";
    TCollection_AsciiString ignoreRidges = " -nr ", invalideElements = " -inv ";
    TCollection_AsciiString subdom = " -sd ";

    TCollection_AsciiString minLevel, maxLevel, sharpAngle, mode;
    minLevel = _hexesMinLevel;
    maxLevel = _hexesMaxLevel;
    sharpAngle = _hexoticSharpAngleThreshold;
    mode = 4;

    map <int,int> aSmdsToHexoticIdMap;
    map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

    Hexotic_In  = aTmpDir + "Hexotic_In.mesh";
    Hexotic_Out = aTmpDir + "Hexotic_Out.mesh";

    if (_hexoticIgnoreRidges)
      run_Hexotic +=  ignoreRidges;

    if (_hexoticInvalidElements)
      run_Hexotic +=  invalideElements;

    run_Hexotic += angle + sharpAngle + minl + minLevel + maxl + maxLevel + in + Hexotic_In + out + Hexotic_Out;
    run_Hexotic += subdom + mode;

    cout << endl;
    cout << "Hexotic command : " << run_Hexotic << endl;

    OSD_File( Hexotic_In  ).Remove();
    OSD_File( Hexotic_Out ).Remove();

    ofstream HexoticFile (Hexotic_In.ToCString() , ios::out);

    Ok = ( writeHexoticFile(HexoticFile, meshDS, aSmdsToHexoticIdMap, aHexoticIdToNodeMap, Hexotic_In) );
    HexoticFile.close();

    MESSAGE("HexoticPlugin_Hexotic::Compute");

    system( run_Hexotic.ToCString() );

    // --------------
    // read a result
    // --------------

    ifstream fileRes( Hexotic_Out.ToCString() );
    if ( ! fileRes.fail() ) {
      Ok = readResult( Hexotic_Out.ToCString(), meshDS, aHexoticIdToNodeMap, _nodeRefNumber,
                       _nbShape, tabShape, tabBox );
      hexahedraMessage = "complete";
    }
    else {
      hexahedraMessage = "failed";
      cout << endl;
      cout << "Problem with Hexotic output file " << Hexotic_Out.ToCString() << endl;
      Ok = false;
    }
    cout << "Hexahedra meshing " << hexahedraMessage << endl;
    cout << endl;
    _iShape=0;
  }
  return Ok;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & HexoticPlugin_Hexotic::SaveTo(ostream & save)
{
  return save;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & HexoticPlugin_Hexotic::LoadFrom(istream & load)
{
  return load;
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

ostream & operator << (ostream & save, HexoticPlugin_Hexotic & hyp)
{
  return hyp.SaveTo( save );
}

//=============================================================================
/*!
 *  
 */
//=============================================================================

istream & operator >> (istream & load, HexoticPlugin_Hexotic & hyp)
{
  return hyp.LoadFrom( load );
}
