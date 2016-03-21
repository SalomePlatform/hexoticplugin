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

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Create geometry: a box cut by a holed sphere
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Sphere_1 = geompy.MakeSphereR(75)
Sphere_2 = geompy.MakeSphereR(25)
geompy.TranslateDXDYDZ(Box_1, -100, -100, -100)
Cut_1 = geompy.MakeCut(Sphere_1, Sphere_2)
Cut_2 = geompy.MakeCut(Box_1, Cut_1)
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Sphere_1, 'Sphere_1' )
geompy.addToStudy( Sphere_2, 'Sphere_2' )
geompy.addToStudy( Cut_1, 'Cut_1' )
geompy.addToStudy( Cut_2, 'Cut_2' )

# Create filters
# aFilter1: elements inside small sphere
aFilter1 = smesh.GetFilter(SMESH.VOLUME,SMESH.FT_BelongToGeom,'=',Sphere_2)
# aFilter2: elements inside big sphere and not inside small sphere
aFilter2 = smesh.GetFilterFromCriteria([smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,'=',Sphere_1, SMESH.FT_LogicalAND),
                                        smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,'=',Sphere_2, SMESH.FT_LogicalNOT)])
# aFilter3: elements not inside big sphere
aFilter3 = smesh.GetFilter(SMESH.VOLUME,SMESH.FT_BelongToGeom,'=',Sphere_1, SMESH.FT_LogicalNOT)

# Create mesh of Cut_2 with sd mode 2
print "Create mesh of Cut_2 with sd mode 2"
Mesh_mghexa_sd2 = smesh.Mesh(Cut_2, "Mesh_mghexa_sd2")

# Create the 2D algo: MG-CADSurf with geometrical mesh
Mesh_mghexa_sd2.Triangle(algo=smeshBuilder.MG_CADSurf).SetGeometricMesh( 1 )

# Create the 3D algo: MG-Hexa with:
# - minl = 4
# - maxl = 8
# - sd = 2
Mesh_mghexa_sd2.Hexahedron(smeshBuilder.MG_Hexa).SetMinMaxHexes(4, 8).SetHexoticSdMode( 2 )

# Create the groups on filters
g1 = Mesh_mghexa_sd2.GroupOnFilter(SMESH.VOLUME, 'small sphere', aFilter1 )
g1.SetColor( SALOMEDS.Color( 1, 0, 0 ))
g2 = Mesh_mghexa_sd2.GroupOnFilter(SMESH.VOLUME, 'big sphere - small sphere', aFilter2 )
g2.SetColor( SALOMEDS.Color( 0, 1, 0 ))
g3 = Mesh_mghexa_sd2.GroupOnFilter(SMESH.VOLUME, 'box - big sphere', aFilter3 )
g3.SetColor( SALOMEDS.Color( 0, 0, 1 ))

# Compute
Mesh_mghexa_sd2.Compute()

# End of script

