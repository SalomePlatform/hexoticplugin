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

# Create geometry: a box cut by a plane
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Translation_1 = geompy.MakeTranslation(Box_1, 0, 200, 0)
Partition_1 = geompy.MakePartition([Box_1, Translation_1])
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Partition_1, 'Partition_1' )

# Create mesh of Partition_1 with sd mode 4 (default sd mode in SALOME)
Mesh_mghexa_sd4 = smesh.Mesh(Partition_1, "Mesh_mghexa_sd4")
Mesh_mghexa_sd4.Triangle(smeshBuilder.MG_CADSurf)
Mesh_mghexa_sd4.Hexahedron(smeshBuilder.MG_Hexa).SetMinMaxHexes(4, 8).SetHexoticSdMode( 4 )

# Compute
Mesh_mghexa_sd4.Compute()

# End of script
