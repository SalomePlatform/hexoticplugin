# Copyright (C) 2015-2016  CEA/DEN, EDF R&D, OPEN CASCADE
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
theStudy = salome.myStudy

from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Create geometry
Box_1 = geompy.MakeBoxDXDYDZ(50, 50, 50)
geompy.addToStudy( Box_1, 'Box_1' )

# Create mesh
Mesh_mghexa_vl = smesh.Mesh(Box_1, "Mesh_mghexa_vl")

Regular_1D = Mesh_mghexa_vl.Segment()
Local_Length_1 = Regular_1D.LocalLength(8.66025,None,1e-07)

MEFISTO_2D = Mesh_mghexa_vl.Triangle(algo=smeshBuilder.MEFISTO)

MG_Hexa = Mesh_mghexa_vl.Hexahedron(algo=smeshBuilder.MG_Hexa)
MG_Hexa_Parameters = MG_Hexa.Parameters()
MG_Hexa.SetViscousLayers(5,5,3,smeshBuilder.Inward,[13,23])
MG_Hexa_Parameters.SetMinSize( 2 )
MG_Hexa_Parameters.SetMaxSize( 4 )
MG_Hexa_Parameters.SetHexesMinLevel( 2 )
MG_Hexa_Parameters.SetHexesMaxLevel( 4 )

# Compute
Mesh_mghexa_vl.Compute()

# End of script
