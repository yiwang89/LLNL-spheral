#-------------------------------------------------------------------------------
# A 1-D acoustic wave test.  In this version we establish a standing wave
# in a reflecting box.
#-------------------------------------------------------------------------------
from math import *
from Spheral1d import *
from SpheralTestUtilities import *
import mpi
import numpy as np
#import matplotlib.pyplot as plt

title("Standing wave propagation test.")

#-------------------------------------------------------------------------------
# Generic problem parameters
#-------------------------------------------------------------------------------
commandLine(nx1 = 100,
            x0 = 0.0,
            x1 = 1.0,

            rho1 = 1.0,
            eps1 = 1.0,
            A = 0.005,
            kfreq = 1.0,

            cs2 = 1.0,
            mu = 1.0,

            nPerh = 2.01,

            Qconstructor = MonaghanGingoldViscosity,
            #Qconstructor = TensorMonaghanGingoldViscosity,
            Cl = 0.0,
            Cq = 0.0,
            Qlimiter = False,
            epsilon2 = 1e-2,
            hmin = 0.0001, 
            hmax = 0.1,
            cfl = 0.5,
            XSPH = False,
            epsilonTensile = 0.0,
            nTensile = 4,
            filter = 0.0,

            SVPH = False,
            CRKSPH = False,
            TSPH = False,
            IntegratorConstructor = CheapSynchronousRK2Integrator,
            steps = None,
            goalTime = 5.0,
            dt = 0.0001,
            dtMin = 1.0e-5, 
            dtMax = 0.1,
            dtGrowth = 2.0,
            dtverbose = False,
            rigorousBoundaries = False,
            maxSteps = None,
            statsStep = 1,
            smoothIters = 0,
            HEvolution = IdealH,
            densityUpdate = RigorousSumDensity,
            compatibleEnergy = True,
            gradhCorrection = True,
            linearConsistent = False,
            ComputeL1Norm = False,

            restoreCycle = None,
            restartStep = 10000,
            restartBaseName = "dumps-AcousticWave-1d",

            graphics = "gnu",

            checkReversibility = False,
            )

if SVPH:
    HydroConstructor = SVPHFacetedHydro
elif CRKSPH:
    HydroConstructor = CRKSPHHydro
    Qconstructor = CRKSPHMonaghanGingoldViscosity
elif TSPH:
    HydroConstructor = TaylorSPHHydro
else:
    HydroConstructor = SPHHydro

#-------------------------------------------------------------------------------
# Material properties.
#-------------------------------------------------------------------------------
eos = IsothermalEquationOfStateMKS(cs2, mu)

#-------------------------------------------------------------------------------
# Interpolation kernels.
#-------------------------------------------------------------------------------
WT = TableKernel(BSplineKernel(), 1000)
WTPi = TableKernel(BSplineKernel(), 1000)
output("WT")
output("WTPi")

#-------------------------------------------------------------------------------
# Make the NodeList.
#-------------------------------------------------------------------------------
nodes1 = makeFluidNodeList("nodes1", eos,
                           hmin = hmin,
                           hmax = hmax,
                           nPerh = nPerh)
output("nodes1")
output("nodes1.hmin")
output("nodes1.hmax")
output("nodes1.nodesPerSmoothingScale")

#-------------------------------------------------------------------------------
# Set the node properties.
#-------------------------------------------------------------------------------
from DistributeNodes import distributeNodesInRange1d
distributeNodesInRange1d([(nodes1, nx1, rho1, (x0, x1))],
                         nPerh = nPerh)

L = x1 - x0
def Minterval(xi0, xi1):
    return rho1*((xi1 - xi0) +
                 A*L/(pi*kfreq)*(sin(pi*kfreq/L*(xi1 - x0)) -
                                 sin(pi*kfreq/L*(xi0 - x0))))

# Set the node positions, velocities, and densities.
cs = sqrt(cs2)
pos = nodes1.positions()
vel = nodes1.velocity()
rho = nodes1.massDensity()
mass = nodes1.mass()
dx = (x1 - x0)/nx1
imin = int((pos[0].x - x0)/dx + 0.5)
imax = int((pos[-1].x - x0)/dx + 0.5) + 1
for i in xrange(nodes1.numInternalNodes):
    xi0 = pos[i].x - 0.5*dx
    xi1 = pos[i].x + 0.5*dx
    mass[i] = Minterval(xi0, xi1)
    rho[i] = mass[i]/dx

#-------------------------------------------------------------------------------
# Construct a DataBase to hold our node list
#-------------------------------------------------------------------------------
db = DataBase()
output("db")
output("db.appendNodeList(nodes1)")
output("db.numNodeLists")
output("db.numFluidNodeLists")

#-------------------------------------------------------------------------------
# Construct the artificial viscosity.
#-------------------------------------------------------------------------------
q = Qconstructor(Cl, Cq)
q.epsilon2 = epsilon2
q.limiter = Qlimiter
output("q")
output("q.Cl")
output("q.Cq")
output("q.epsilon2")
output("q.limiter")

#-------------------------------------------------------------------------------
# Construct the hydro physics object.
#-------------------------------------------------------------------------------
if SVPH:
    hydro = HydroConstructor(WT, q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             XSVPH = XSPH,
                             linearConsistent = linearConsistent,
                             densityUpdate = densityUpdate,
                             HUpdate = HEvolution,
                             xmin = Vector(-100.0),
                             xmax = Vector( 100.0))
elif CRKSPH:
    hydro = HydroConstructor(WT, WTPi, q,
                             filter = filter,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             XSPH = XSPH,
                             densityUpdate = densityUpdate,
                             HUpdate = HEvolution)

elif TSPH:
    hydro = HydroConstructor(WT, q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             XSPH = XSPH,
                             HUpdate = HEvolution)
else:
    hydro = HydroConstructor(WT, WTPi, q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             gradhCorrection = gradhCorrection,
                             XSPH = XSPH,
                             densityUpdate = densityUpdate,
                             HUpdate = HEvolution,
                             epsTensile = epsilonTensile,
                             nTensile = nTensile)
output("hydro")

#-------------------------------------------------------------------------------
# Create boundary conditions.
#-------------------------------------------------------------------------------
xPlane0 = Plane(Vector(x0), Vector( 1.0))
xPlane1 = Plane(Vector(x1), Vector(-1.0))
xbc0 = ReflectingBoundary(xPlane0)
xbc1 = ReflectingBoundary(xPlane1)
for bc in (xbc0, xbc1):
    hydro.appendBoundary(bc)

#-------------------------------------------------------------------------------
# Construct a time integrator.
#-------------------------------------------------------------------------------
integrator = IntegratorConstructor(db)
integrator.appendPhysicsPackage(hydro)
integrator.lastDt = dt
integrator.dtMin = dtMin
integrator.dtMax = dtMax
integrator.dtGrowth = dtGrowth
integrator.rigorousBoundaries = rigorousBoundaries
integrator.verbose = dtverbose
output("integrator")
output("integrator.lastDt")
output("integrator.dtMin")
output("integrator.dtMax")
output("integrator.dtGrowth")
output("integrator.rigorousBoundaries")

#-------------------------------------------------------------------------------
# Make the problem controller.
#-------------------------------------------------------------------------------
print "Making controller."
control = SpheralController(integrator, WT,
                            statsStep = statsStep,
                            restartStep = restartStep,
                            restartBaseName = restartBaseName,
                            restoreCycle = restoreCycle)
output("control")

#-------------------------------------------------------------------------------
# Advance to the end time.
#-------------------------------------------------------------------------------
if steps is None:
    if control.time() < goalTime:
        control.advance(goalTime, maxSteps)
    if checkReversibility:
        for i in xrange(nodes1.numNodes):
            vel[i] = -vel[i]
        control.advance(2*goalTime, maxSteps)
else:
    control.step(steps)

#-------------------------------------------------------------------------------
# Compute the analytic answer.
#-------------------------------------------------------------------------------
import StandingWaveSolution
xlocal = [pos.x for pos in nodes1.positions().internalValues()]
xglobal = mpi.reduce(xlocal, mpi.SUM)
dx = (x1 - x0)/nx1
h1 = 1.0/(nPerh*dx)
answer = StandingWaveSolution.StandingWaveSolution(eos, cs, rho1, x0, x1, A, kfreq, h1)

#-------------------------------------------------------------------------------
# Plot the final state.
#-------------------------------------------------------------------------------
if graphics == "gnu":
    from SpheralGnuPlotUtilities import *
    state = State(db, integrator.physicsPackages())
    rhoPlot, velPlot, epsPlot, PPlot, HPlot = plotState(state)
    if mpi.rank == 0:
        plotAnswer(answer, control.time(), rhoPlot, velPlot, epsPlot, PPlot, HPlot, xglobal)
    cs = state.scalarFields(HydroFieldNames.soundSpeed)
    csPlot = plotFieldList(cs, winTitle="Sound speed", colorNodeLists=False)
    EPlot = plotEHistory(control.conserve)

    # Plot the correction terms.

    # Plot the grad h correction term (omega)

    if SVPH:
        volPlot = plotFieldList(hydro.volume(),
                                winTitle = "volume",
                                colorNodeLists = False)
    elif CRKSPH:
        A0=hydro.A0()
	print("ARRAY LENGTH:")
        print(A0[0].__len__())
        tmp=[]
        for i in range(A0[0].__len__()):
		tmp.append(A0[0][i])
        A=np.array(tmp)
        #ret=smooth(A,11,'hamming') 
        CoeffBx = Gnuplot.Data(A,
                               with_ = "points",
                               #with_ = "lines",
                               title = "Bx",
                               inline = True)
        p0 = generateNewGnuPlot()
        p0.plot(CoeffBx)
        p0.title("COEFF")
        p0.refresh()
        #print(A0.size())
        #A=np.array(A0)
        #ret=smooth(A,11,'hamming')
        volPlot = plotFieldList(hydro.volume(),
                                winTitle = "volume",
                                colorNodeLists = False)
        A0Plot = plotFieldList(hydro.A0(),
                               winTitle = "A0",
                               colorNodeLists = False)
        APlot = plotFieldList(hydro.A(),
                              winTitle = "A",
                              colorNodeLists = False)
        BPlot = plotFieldList(hydro.B(),
                              yFunction = "%s.x",
                              winTitle = "B",
                              colorNodeLists = False)

    else:
        omegaPlot = plotFieldList(hydro.omegaGradh(),
                                  winTitle = "grad h correction",
                                  colorNodeLists = False)
    if ComputeL1Norm:
       xans, vans, uans, rhoans, Pans, hans = answer.solution(control.time(), xglobal)
       #rho = hydro.massDensity() 
       fieldList = state.scalarFields(HydroFieldNames.massDensity)
       #rho = field.internalValues()
       for field in fieldList:
          rho = [eval("%s" % "y") for y in field.internalValues()]
          #plt.figure()
          #plt.plot(xans,rhoans)
          #plt.scatter(xans,rho)
          #plt.xlim([np.min(xans),np.max(xans)])
          #plt.ylim([np.min(rhoans),np.max(rhoans)])
          #plt.show()
          diff=np.array(rho)-np.array(rhoans)
          L1Norm=(1.0/len(diff))*np.sum(np.abs(diff))
          print "\n\nL1Norm=",L1Norm, "\n\n"

Eerror = (control.conserve.EHistory[-1] - control.conserve.EHistory[0])/control.conserve.EHistory[0]
print "Total energy error: %g" % Eerror
if abs(Eerror) > 1e-13:
    raise ValueError, "Energy error outside allowed bounds."
