# Copyright (C) 2013-2014  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

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
