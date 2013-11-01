#-------------------------------------------------------------------------------
# Master import file to import the Spheral packages and a standard set of
# helper extensions, including the optional solid material and strength
# extensions.
#-------------------------------------------------------------------------------
from Spheral import *
from SpheralModules.Spheral.SolidMaterial import *
from SpheralModules.Spheral.SolidSPHSpace import *
from SolidNodeLists import *
from GradyKippTensorDamage import *
from SolidSPHHydros import *

# ------------------------------------------------------------------------------
# Import the SolidMaterial python extensions.
# ------------------------------------------------------------------------------
from SolidMaterialUnits import *
from SolidMaterialEquationsOfState import *

# ------------------------------------------------------------------------------
# Import our shadow layers for augmenting C++ types.
# ------------------------------------------------------------------------------
for shadowedthing in ("TillotsonEquationOfState",
                      "ConstantStrength"):
    exec("from Shadow%(thing)s import %(thing)s1d, %(thing)s2d, %(thing)s3d" % {"thing" : shadowedthing})
