# Copyright (C) 2007-2011  CEA/DEN, EDF R&D, OPEN CASCADE
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
#

from smesh import Mesh_Algorithm, AssureGeomPublished

# import HexoticPlugin module if possible
noHexoticPlugin = 0
try:
    import HexoticPlugin
except ImportError:
    noHexoticPlugin = 1
    pass

Hexotic = "Hexotic_3D"


## Defines a hexahedron 3D algorithm
#
class Hexotic_Algorithm(Mesh_Algorithm):

    meshMethod = "Hexahedron"
    algoType   = Hexotic

    ## Private constructor.
    def __init__(self, mesh, geom=0):
        Mesh_Algorithm.__init__(self)
        if noHexoticPlugin: print "Warning: HexoticPlugin module unavailable"
        self.Create(mesh, geom, Hexotic, "libHexoticEngine.so")
        pass

    ## Defines "MinMaxQuad" hypothesis to give three hexotic parameters
    def MinMaxQuad(self, min=3, max=8, quad=True):
        self.params = self.Hypothesis("Hexotic_Parameters", [], "libHexoticEngine.so",
                                      UseExisting=0)
        self.params.SetHexesMinLevel(min)
        self.params.SetHexesMaxLevel(max)
        self.params.SetHexoticQuadrangles(quad)
        return self.params
