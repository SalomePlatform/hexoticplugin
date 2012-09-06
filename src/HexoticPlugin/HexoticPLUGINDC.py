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

from smesh_algorithm import Mesh_Algorithm
from smesh import AssureGeomPublished

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
        self.params = None
        pass

    ## Defines "SetMinMaxHexes" hypothesis to give two hexotic parameters
    #  @param min minimal level of recursive partitioning on the initial octree cube
    #  @param max maximal level of recursive partitioning on the initial octree cube
    #  @return hypothesis object
    def SetMinMaxHexes(self, min=3, max=8):
        self.Parameters().SetHexesMinLevel(min)
        self.Parameters().SetHexesMaxLevel(max)
        return self.Parameters()

    ## Defines "SetMinMaxSize" hypothesis to give two hexotic parameters
    #  @param min minimal element's size
    #  @param max maximal element's size
    #  @return hypothesis object
    def SetMinMaxSize(self, min, max):
        self.Parameters().SetMinSize(min)
        self.Parameters().SetMaxSize(max)
        return self.Parameters()

    ## (OBSOLETE) Defines "MinMaxQuad" hypothesis to give three hexotic parameters
    #  @param min minimal level of recursive partitioning on the initial octree cube
    #  @param max maximal level of recursive partitioning on the initial octree cube
    #  @param quad not documented
    #  @return hypothesis object
    def MinMaxQuad(self, min=3, max=8, quad=True):
        print "WARNING: Function MinMaxQuad is deprecated, use SetMinMaxHexes instead"
        return self.SetMinMaxHexes(min, max)

    ## Defines hypothesis having several parameters
    #  @return hypothesis object
    def Parameters(self):
        if not self.params:
            self.params = self.Hypothesis("Hexotic_Parameters", [],
                                          "libHexoticEngine.so", UseExisting=0)
            pass
        return self.params


    pass # end of Hexotic_Algorithm class
