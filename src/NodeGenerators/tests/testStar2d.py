#-------------------------------------------------------------------------------
# Unit test of the InteriorGenerator2d and relaxation for a polygonl boundary.
#-------------------------------------------------------------------------------
from math import *
from Spheral2d import *
from InteriorGenerator import InteriorGenerator2d
from VoronoiDistributeNodes import distributeNodes2d
from SpheralVisitDump import *

# Create our star polygonal geometry.  Adapted from one of our 2D polytope test boundaries.
points = vector_of_Vector()
facets = vector_of_vector_of_unsigned()
theta0 = 2*pi/5
outerRadius = 2.0
innerRadius = outerRadius*( sin(theta0/4.0) / sin(3*theta0/4.0) )
center = Vector(1,1)
for p in xrange(5):
    # For the pointy bits of the star
    theta = pi/2 + p*theta0
    points.push_back(center + Vector(outerRadius*cos(theta),
                                     outerRadius*sin(theta)))
    # For the concave bits of the star
    theta = pi/2 + p*theta0 + theta0/2.0;
    points.push_back(center + Vector(innerRadius*cos(theta),
                                     innerRadius*sin(theta)))

for f in xrange(10):
    facets.push_back(vector_of_unsigned(2))
    facets[-1][0] = f
    facets[-1][1] = (f + 1) % 10
starBoundary = Polygon(points, facets)

# Generate a NodeList for us to fill in.
eos = GammaLawGasMKS(1.4, 1.0)
WT = TableKernel(BSplineKernel(), 1000)
nodes = makeFluidNodeList("nodes", eos,
                          hmin = 1e-5,
                          hmax = 100.0,
                          hminratio = 0.1,
                          nPerh = 2.01,
                          xmin = Vector(-100, -100),
                          xmax = Vector(100, 100))

# Make those nodes!
generator = InteriorGenerator2d(boundary = starBoundary, 
                                dx = 0.05,
                                rho = 10.0,
                                nNodePerh = 2.01,
                                jitter = 0.4,
                                SPH = False)

distributeNodes2d((nodes, generator))

# Write out a Visit file to see what happened.
dumper = SpheralVisitDump("star_generator_test",
                          listOfFields = [nodes.massDensity(),
                                          nodes.Hfield()])
dumper.dump(0.0, 0)

# Do some relaxation of the points.
db = DataBase()
db.appendNodeList(nodes)
relaxer = relaxNodeDistribution(db,
                                starBoundary,
                                vector_of_Boundary(),
                                WT,
                                SPHSmoothingScale(),
                                WeightingFunctor(),
                                10,
                                1.0e-3)

# Dump the new result.
dumper.dump(1.0, 1)

