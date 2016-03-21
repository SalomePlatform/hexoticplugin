# Copyright (C) 2013-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)


# create a sphere
sphere = geompy.MakeSphereR(100.)
geompy.addToStudy(sphere, "sphere")

# create a mesh on the sphere
mghexaMesh = smesh.Mesh(sphere,"sphere: MG-CADSurf and MG-Hexa mesh")

# create a MG-CADSurf algorithm for faces
MG_CADSurf = mghexaMesh.Triangle(algo=smeshBuilder.MG_CADSurf)
MG_CADSurf.SetGeometricMesh( 1 )

# create a MG-Hexa algorithm for volumes
MG_Hexa = mghexaMesh.Hexahedron(algo=smeshBuilder.MG_Hexa)

## compute the mesh
#mghexaMesh.Compute()

# Change the level of subdivision
MG_Hexa.SetMinMaxHexes(4, 8)

## compute the mesh
#mghexaMesh.Compute()

# Local size

# Get the sphere skin
faces = geompy.SubShapeAll(sphere, geompy.ShapeType["FACE"])

# Set a local size on the face
MG_Hexa.SetMinMaxSize(10, 20)
MG_Hexa.SetSizeMap(faces[0], 10)

# compute the mesh
mghexaMesh.Compute()

# End of script
