# Copyright (C) 2007-2012  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

##
# @package HexoticPLUGINDC
# Python API for the Hexotic meshing plug-in module.

from smesh import Mesh_Algorithm, AssureGeomPublished

# import HexoticPlugin module if possible
noHexoticPlugin = 0
try:
    import HexoticPlugin
except ImportError:
    noHexoticPlugin = 1
    pass

#----------------------------
# Mesh algo type identifiers
#----------------------------

## Algorithm type: Hexotic hexahedron 3D algorithm, see Hexotic_Algorithm
Hexotic = "Hexotic_3D"

#----------------------------
# Algorithms
#----------------------------

## Defines a hexahedron 3D algorithm
#
#  It is created by calling smesh.Mesh.Hexahedron( smesh.Hexotic, geom=0 )
class Hexotic_Algorithm(Mesh_Algorithm):

    ## name of the dynamic method in smesh.Mesh class
    #  @internal
    meshMethod = "Hexahedron"
    ## type of algorithm used with helper function in smesh.Mesh class
    #  @internal
    algoType   = Hexotic
    ## doc string of the method in smesh.Mesh class
    #  @internal
    docHelper  = "Creates hexahedron 3D algorithm for volumes"

    ## Private constructor.
    #  @param mesh parent mesh object algorithm is assigned to
    #  @param geom geometry (shape/sub-shape) algorithm is assigned to;
    #              if it is @c 0 (default), the algorithm is assigned to the main shape
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        if noHexoticPlugin: print "Warning: HexoticPlugin module unavailable"
        self.Create(mesh, geom, Hexotic, "libHexoticEngine.so")
        pass

    ## Defines "MinMaxQuad" hypothesis to give three hexotic parameters
    #  @param min minimal level of recursive partitioning on the initial octree cube
    #  @param max maximal level of recursive partitioning on the initial octree cube
    #  @param quad not documented
    #  @return hypothesis object
    def MinMaxQuad(self, min=3, max=8, quad=True):
        self.params = self.Hypothesis("Hexotic_Parameters", [], "libHexoticEngine.so",
                                      UseExisting=0)
        self.params.SetHexesMinLevel(min)
        self.params.SetHexesMaxLevel(max)
        self.params.SetHexoticQuadrangles(quad)
        return self.params

    pass # end of Hexotic_Algorithm class
