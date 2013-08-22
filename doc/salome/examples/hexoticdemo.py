
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# create a sphere
sphere = geompy.MakeSphereR(100.)
geompy.addToStudy(sphere, "sphere")

# create a mesh on the sphere
hexoticMesh = smesh.Mesh(sphere,"sphere: BLSurf and Hexotic mesh")

# create a BLSurf algorithm for faces
BLSURF = hexoticMesh.Triangle(algo=smeshBuilder.BLSURF)
BLSURF.SetGeometricMesh( 1 )

# create a Hexotic algorithm for volumes
HEXOTIC = hexoticMesh.Hexahedron(algo=smeshBuilder.Hexotic)

## compute the mesh
#hexoticMesh.Compute()

# Change the level of subdivision
HEXOTIC.SetMinMaxHexes(4, 8)

## compute the mesh
#hexoticMesh.Compute()

# Local size

# Get the sphere skin
faces = geompy.SubShapeAll(sphere, geompy.ShapeType["FACE"])

# Set a local size on the face
HEXOTIC.SetMinMaxSize(10, 20)
HEXOTIC.SetSizeMap(faces[0], 10)

# compute the mesh
hexoticMesh.Compute()

# End of script
