
import salome
salome.salome_init()
import GEOM
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
aFilter1 = smesh.GetFilterFromCriteria([smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Sphere_2)])
# aFilter2: elements inside big sphere and not inside small sphere
aFilter2 = smesh.GetFilterFromCriteria([smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Sphere_1, SMESH.FT_LogicalAND),
                                        smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Sphere_2, SMESH.FT_LogicalNOT)])
# aFilter3: elements not inside big sphere
aFilter3 = smesh.GetFilterFromCriteria([smesh.GetCriterion(SMESH.VOLUME,SMESH.FT_BelongToGeom,SMESH.FT_Undefined,Sphere_1, SMESH.FT_LogicalNOT)])

# Create mesh of Cut_2 with sd mode 2
print "Create mesh of Cut_2 with sd mode 2"
Mesh_hexotic_sd2 = smesh.Mesh(Cut_2, "Mesh_hexotic_sd2")

# Create the 2D algo: BlSurf with geometrical mesh
Mesh_hexotic_sd2.Triangle(algo=smeshBuilder.BLSURF).Parameters().SetGeometricMesh( 1 )

# Create the 3D algo: Hexotic with:
# - minl = 4
# - maxl = 8
# - sd = 2
Mesh_hexotic_sd2.Hexahedron(algo=smeshBuilder.Hexotic).SetMinMaxHexes(4, 8).SetHexoticSdMode( 2 )

# Create the groups on filters
g1 = Mesh_hexotic_sd2.GroupOnFilter(SMESH.VOLUME, 'small sphere', aFilter1 )
g1.SetColor( SALOMEDS.Color( 1, 0, 0 ))
g2 = Mesh_hexotic_sd2.GroupOnFilter(SMESH.VOLUME, 'big sphere - small sphere', aFilter2 )
g2.SetColor( SALOMEDS.Color( 0, 1, 0 ))
g3 = Mesh_hexotic_sd2.GroupOnFilter(SMESH.VOLUME, 'box - big sphere', aFilter3 )
g3.SetColor( SALOMEDS.Color( 0, 0, 1 ))

# Compute
Mesh_hexotic_sd2.Compute()

# End of script
