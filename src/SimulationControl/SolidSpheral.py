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

