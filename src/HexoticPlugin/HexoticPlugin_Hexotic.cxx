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
//function : countFaces
//purpose  : 
//=======================================================================

static int countFaces( SMESHDS_Mesh*                    the2DMesh,
                       list< const SMDS_MeshElement* >& theListOfFaces,
                       bool                             externalFaces )
{
  int nbFaces = 0;
  TopExp_Explorer fExp( (the2DMesh->ShapeToMesh()), TopAbs_FACE );
  SMESHDS_SubMesh* the2DSubMesh;
  SMDS_ElemIteratorPtr itOnSmdsElement;

  if ( externalFaces )
    nbFaces = the2DMesh->NbFaces();
  else {
    for ( ; fExp.More(); fExp.Next() ) {
      the2DSubMesh = the2DMesh->MeshElements( fExp.Current() );
      if ( the2DSubMesh ) {
        itOnSmdsElement = the2DSubMesh->GetElements();
        while ( itOnSmdsElement->more() ) {
          theListOfFaces.push_back( itOnSmdsElement->next() );
          nbFaces++;
        }
      }
    }
  }
  return nbFaces;
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

  bool onlyExternalFaces = true;
  int nbVertices         = 0;
  int nbTriangles        = 0;
  const char* space      = "  ";
  const int   dummyint   = 0;

  int aSmdsNodeID = 1;
  const SMDS_MeshNode* aNode;
  SMDS_NodeIteratorPtr itOnNode;

  list< const SMDS_MeshElement* > faces;
  list< const SMDS_MeshElement* >::iterator itListFace;
  const SMDS_MeshElement* aFace;
  map<int,int>::const_iterator itOnSmdsNode;
  SMDS_ElemIteratorPtr itOnSmdsElement;
  SMDS_ElemIteratorPtr itOnFaceNode;
  SMDS_FaceIteratorPtr itOnSmdsFace;

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
      theFile << aNode->X() << space << aNode->Y() << space << aNode->Z() << space << dummyint << endl;
      }

// Writing SMESH faces into Hexotic File

  nbTriangles = countFaces(theMesh, faces, onlyExternalFaces);

  theFile << endl;
  theFile << "# Set of mesh triangles (v1,v2,v3,tag)" << endl;
  theFile << "Triangles" << endl;
  theFile << nbTriangles << endl;

  if ( onlyExternalFaces ) {
    itOnSmdsFace = theMesh->facesIterator();
    while ( itOnSmdsFace->more() ) {
      aFace = itOnSmdsFace->next();
      itOnFaceNode = aFace->nodesIterator();
      while ( itOnFaceNode->more() ) {
        aSmdsNodeID = itOnFaceNode->next()->GetID();
        itOnSmdsNode = theSmdsToHexoticIdMap.find( aSmdsNodeID );
        ASSERT( itOnSmdsNode != theSmdsToHexoticIdMap.end() );
        theFile << (*itOnSmdsNode).second << space;
      }
      theFile << dummyint << endl;
    }
  }
  else {
    itListFace = faces.begin();
    for ( ; itListFace != faces.end(); ++itListFace ) {
      aFace = *itListFace;
      itOnFaceNode = aFace->nodesIterator();
      while ( itOnFaceNode->more() ) {
        aSmdsNodeID = itOnFaceNode->next()->GetID();
        itOnSmdsNode = theSmdsToHexoticIdMap.find( aSmdsNodeID );
        ASSERT( itOnSmdsNode != theSmdsToHexoticIdMap.end() );
        theFile << (*itOnSmdsNode).second << space;
      }
      theFile << dummyint << endl;
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
//function : getInt
//purpose  : 
//=======================================================================

static bool getInt( int & theValue, char * & theLine )
{
  char *ptr;
  theValue = strtol( theLine, &ptr, 10 );
  if ( ptr == theLine ||
      // there must not be neither '.' nor ',' nor 'E' ...
      (*ptr != ' ' && *ptr != '\n' && *ptr != '\0'))
    return false;

  DUMP( "  " << theValue );
  theLine = ptr;
  return true;
}

//=======================================================================
//function : getDouble
//purpose  : 
//=======================================================================

static bool getDouble( double & theValue, char * & theLine )
{
  char *ptr;
  theValue = strtod( theLine, &ptr );
  if ( ptr == theLine )
    return false;

  DUMP( "   " << theValue );
  theLine = ptr;
  return true;
}
  
//=======================================================================
//function : readLine
//purpose  : 
//=======================================================================

#define HexoticPlugin_BUFLENGTH 256
#define HexoticPlugin_ReadLine(aPtr,aBuf,aFile,aLineNb) \
{  aPtr = fgets( aBuf, HexoticPlugin_BUFLENGTH - 2, aFile ); aLineNb++; DUMP(endl); }

//=======================================================================
//function : readResult
//purpose  : 
//=======================================================================

static bool readResult(FILE *                           theFile,
                       SMESHDS_Mesh *                   theMesh,
                       const TopoDS_Shape &             theShape,
                       map <int,const SMDS_MeshNode*> & theHexoticIdToNodeMap,
                       const TCollection_AsciiString &  Hexotic_Out,
                       int &                            nodeRefNumber)
{
  // ---------------------------------
  // Read generated elements and nodes
  // ---------------------------------

  cout << "Reading Hexotic output file : " << Hexotic_Out << endl;
  cout << endl;

  char aBuffer[ HexoticPlugin_BUFLENGTH ];
  char * aPtr;
  int aLineNb = 0;
  int shapeID = theMesh->ShapeToIndex( theShape );

  int line = 1, EndOfFile = 0, nbElem = 0, nField = 10, nbRef = 0, aHexoticNodeID = 0;
  char * theField;

  char * tabField [nField];
  int    tabRef [nField];

  tabField[0] = "MeshVersionFormatted";    tabRef[0] = 0;
  tabField[1] = "Dimension";               tabRef[1] = 0;
  tabField[2] = "Vertices";                tabRef[2] = 3;
  tabField[3] = "Edges";                   tabRef[3] = 2;
  tabField[4] = "Triangles";               tabRef[4] = 3;
  tabField[5] = "Quadrilaterals";          tabRef[5] = 4;
  tabField[6] = "Hexahedra";               tabRef[6] = 8;
  tabField[7] = "Corners";                 tabRef[7] = 1;
  tabField[8] = "Ridges";                  tabRef[0] = 1;
  tabField[9] = "End";                     tabRef[0] = 0;

  nodeRefNumber += theMesh->NbNodes();

  SMDS_NodeIteratorPtr itOnHexoticInputNode = theMesh->nodesIterator();
  while ( itOnHexoticInputNode->more() )
    theMesh->RemoveNode( itOnHexoticInputNode->next() );

  while ( EndOfFile == 0  ) {
    HexoticPlugin_ReadLine( aPtr, aBuffer, theFile, aLineNb );
    for ( int iField = 0; iField < nField; iField++ ) {
      stringstream theMessage;
      theField = tabField[iField];
      if ( strncmp(aPtr, theField, strlen(theField)) == 0 ) {
        if ( strcmp(theField, "End") == 0 ) {
          EndOfFile = 1;
          theMessage << "End of Hexotic output file has been reached";
        }
        else {
          HexoticPlugin_ReadLine( aPtr, aBuffer, theFile, aLineNb );
          line++;
          getInt( nbElem, aPtr );

          if ( strcmp(theField, "MeshVersionFormatted") == 0 )
            theMessage << "Hexotic mesh descriptor : " << theField << " " << nbElem;
          else if ( strcmp(theField, "Dimension") == 0 )
            theMessage << "Hexotic mesh of " << nbElem << "D dimension";
          else if ( strcmp(theField, "Vertices")       == 0 ||
                    strcmp(theField, "Edges")          == 0 ||
                    strcmp(theField, "Quadrilaterals") == 0 ||
                    strcmp(theField, "Hexahedra")      == 0 ) {
            nbRef = tabRef[iField];
            HexoticPlugin_ReadLine( aPtr, aBuffer, theFile, aLineNb );           // read blank line

            if ( strcmp(theField, "Vertices") == 0 ) {
              int aHexoticID;
              double coord[nbRef];
              SMDS_MeshNode * aHexoticNode;

              for ( int iElem = 0; iElem < nbElem; iElem++ ) {
                aHexoticID = iElem + 1 + nodeRefNumber;
                HexoticPlugin_ReadLine( aPtr, aBuffer, theFile, aLineNb );   // read file lines
                for ( int iCoord = 0; iCoord < 3; iCoord++ )
                  getDouble ( coord[ iCoord ], aPtr );
                aHexoticNode = theMesh->AddNode(coord[0], coord[1], coord[2]);
                theMesh->SetNodeInVolume( aHexoticNode, shapeID );
                theHexoticIdToNodeMap[ aHexoticID ] = aHexoticNode;
              }
            }
            else {
              const SMDS_MeshNode * node[nbRef];
              SMDS_MeshElement* aHexoticElement;
              map <int,const SMDS_MeshNode*>::iterator itOnHexoticNode;

              for ( int iElem = 0; iElem < nbElem; iElem++ ) {
                HexoticPlugin_ReadLine( aPtr, aBuffer, theFile, aLineNb );   // read file lines
                for ( int iRef = 0; iRef < nbRef; iRef++ ) {
                  getInt ( aHexoticNodeID, aPtr );                         // read nbRef aHexoticNodeID
                  aHexoticNodeID += nodeRefNumber;
                  itOnHexoticNode = theHexoticIdToNodeMap.find( aHexoticNodeID );
                  node[ iRef ] = itOnHexoticNode->second;
                }

                if ( strcmp(theField, "Edges") == 0 )                        // create an element
                  aHexoticElement = theMesh->AddEdge( node[0], node[1] );
                else if ( strcmp(theField, "Quadrilaterals") == 0 )
                  aHexoticElement = theMesh->AddFace( node[0], node[1], node[2], node[3] );
                else if ( strcmp(theField, "Hexahedra") == 0 )
                  aHexoticElement = theMesh->AddVolume( node[0], node[1], node[2], node[3], node[4], node[5], node[6], node[7] );

                theMesh->SetMeshElementOnShape( aHexoticElement, shapeID );
              }
            }
            theMessage << nbElem << " " << theField << " created";
          }
        }
        if ( theMessage.str().size() != 0 ) {
          cout << theMessage.str() << endl;
          break;
        }
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
  bool Ok;
  SMESHDS_Mesh* meshDS = theMesh.GetMeshDS();

  if (_iShape == 0 && _nbShape == 0) {
    cout << endl;
    cout << "Hexotic execution..." << endl;
    cout << endl;

    TopExp_Explorer expf(meshDS->ShapeToMesh(), TopAbs_SOLID);
    for ( ; expf.More(); expf.Next() )
        _nbShape++;
  }

  _iShape++;

  if (_iShape == _nbShape ) {
    SetParameters(_hypothesis);

    cout << endl;
    cout << _name << " parameters :" << endl;
    cout << "    " << _name << " Segments Min Level = " << _hexesMinLevel << endl;
    cout << "    " << _name << " Segments Max Level = " << _hexesMaxLevel << endl;
    cout << "    " << "Salome Quadrangles : " << (_hexoticQuadrangles ? "yes":"no") << endl;

    TCollection_AsciiString aTmpDir = getTmpDir();
    TCollection_AsciiString Hexotic_In, Hexotic_Out;
    TCollection_AsciiString run_Hexotic( "hexotic" );

    TCollection_AsciiString minl = " -minl ", maxl = " -maxl ";
    TCollection_AsciiString in   = " -in ",   out  = " -out ";

    TCollection_AsciiString minLevel, maxLevel;
    minLevel = _hexesMinLevel;
    maxLevel = _hexesMaxLevel;

    map <int,int> aSmdsToHexoticIdMap;
    map <int,const SMDS_MeshNode*> aHexoticIdToNodeMap;

    Hexotic_In  = aTmpDir + "Hexotic_In.mesh";
    Hexotic_Out = aTmpDir + "Hexotic_Out.mesh";
    run_Hexotic += minl + minLevel + maxl + maxLevel + in + Hexotic_In + out + Hexotic_Out;

    // cout << "Hexotic command : " << run_Hexotic << endl;

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

    FILE * aResultFile = fopen( Hexotic_Out.ToCString(), "r" );
    if (aResultFile) {
      Ok = readResult( aResultFile, meshDS, theShape, aHexoticIdToNodeMap, Hexotic_Out, _nodeRefNumber );
      fclose(aResultFile);
      cout << "Hexotic output file read !" << endl;
      cout << endl;
    }
    else
      Ok = false;
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
