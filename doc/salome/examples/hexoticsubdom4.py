
import salome
salome.salome_init()
import GEOM
from salome.geom import geomBuilder
geompy = geomBuilder.New(salome.myStudy)

import SMESH, SALOMEDS
from salome.smesh import smeshBuilder
smesh =  smeshBuilder.New(salome.myStudy)

# Create geometry: a box cut by a plane
Box_1 = geompy.MakeBoxDXDYDZ(200, 200, 200)
Translation_1 = geompy.MakeTranslation(Box_1, 0, 200, 0)
Partition_1 = geompy.MakePartition([Box_1, Translation_1], [], [], [], geompy.ShapeType["SOLID"], 0, [], 0)
geompy.addToStudy( Box_1, 'Box_1' )
geompy.addToStudy( Translation_1, 'Translation_1' )
geompy.addToStudy( Partition_1, 'Partition_1' )

# Create mesh of Partition_1 with sd mode 4 (default sd mode in SALOME)
Mesh_hexotic_sd4 = smesh.Mesh(Partition_1, "Mesh_hexotic_sd4")
Mesh_hexotic_sd4.Triangle(algo=smeshBuilder.BLSURF)
Mesh_hexotic_sd4.Hexahedron(algo=smeshBuilder.Hexotic).SetMinMaxHexes(4, 8).SetHexoticSdMode( 4 )

# Compute
Mesh_hexotic_sd4.Compute()

# End of script
