import os, sys
import shutil
from SolidSpheral3d import *
from SpheralTestUtilities import *
from SodAnalyticSolution import *

title("1-D integrated hydro test -- planar Sod problem")

#-------------------------------------------------------------------------------
# Generic problem parameters
#-------------------------------------------------------------------------------
commandLine(nx1 = 160,
            ny1 = 32,
            nz1 = 32,

            nx2 = 100,
            ny2 = 20,
            nz2 = 20,

            rho1 = 1.0,
            rho2 = 0.25,
            P1 = 1.0,
            P2 = 0.1795,

            numNodeLists = 1,

            x0 = -0.5,
            x1 = 0.0,
            x2 = 0.5,

            y0 = 0.0,
            y1 = 0.1,

            z0 = 0.0,
            z1 = 0.1,

            hsmooth = 0.0,             # Optionally smooth initial discontinuity
            sumInitialDensity = False, # Optionally sum the initial density before setting the pressure and such

            nPerh = 1.25,

            gammaGas = 5.0/3.0,
            mu = 1.0,
            
            SVPH = False,
            CRKSPH = False,
            PSPH = False,
            evolveTotalEnergy = False,  # Only for SPH variants -- evolve total rather than specific energy
            solid = False,    # If true, use the fluid limit of the solid hydro option
            Qconstructor = MonaghanGingoldViscosity,
            boolReduceViscosity = False,
            nh = 5.0,
            aMin = 0.1,
            aMax = 2.0,
            Cl = 1.0,
            Cq = 1.5,
            etaCritFrac = 1.0,
            etaFoldFrac = 0.2,
            boolCullenViscosity = False,
            alphMax = 2.0,
            alphMin = 0.02,
            betaC = 0.7,
            betaD = 0.05,
            betaE = 1.0,
            fKern = 1.0/3.0,
            boolHopkinsCorrection = True,
            HopkinsConductivity = False,
            linearInExpansion = False,
            quadraticInExpansion = False,
            Qlimiter = False,
            epsilon2 = 1e-4,
            hmin = 1e-10,
            hmax = 1.0,
            cfl = 0.5,
            XSPH = False,
            epsilonTensile = 0.0,
            nTensile = 8,
            rhoMin = 0.01,
            filter = 0.00,
            KernelConstructor = BSplineKernel,
            order = 5,
            
            bArtificialConduction = False,
            arCondAlpha = 0.5,

            IntegratorConstructor = CheapSynchronousRK2Integrator,
            dtverbose = False,
            steps = None,
            goalTime = 0.15,
            dt = 1e-6,
            dtMin = 1.0e-6,
            dtMax = 0.1,
            dtGrowth = 2.0,
            rigorousBoundaries = False,
            maxSteps = None,
            statsStep = 10,
            HUpdate = IdealH,
            correctionOrder = LinearOrder,
            volumeType = CRKSumVolume,
            densityUpdate = RigorousSumDensity,
            compatibleEnergy = True,
            correctVelocityGradient = True,
            gradhCorrection = True,
            linearConsistent = False,

            clearDirectories = False,
            restoreCycle = -1,
            restartStep = 100,
            dataDirBase = "dumps-Sod-planar-3d",
            restartBaseName = "Sod-planar-3d-restart",
            outputFile = "None",
            checkRestart = False,

            vizCycle = None,
            vizTime = 0.01,
            graphics = True,
            )

assert not(boolReduceViscosity and boolCullenViscosity)
if SVPH:
    HydroConstructor = SVPHFacetedHydro
elif CRKSPH:
    if solid:
        HydroConstructor = SolidCRKSPHHydro
    else:
        HydroConstructor = CRKSPHHydro
    Qconstructor = CRKSPHMonaghanGingoldViscosity
elif PSPH:
    HydroConstructor = PSPHHydro
else:
    if solid:
        HydroConstructor = SolidSPHHydro
    else:
        HydroConstructor = SPHHydro

dataDir = os.path.join(dataDirBase, 
                       HydroConstructor.__name__,
                       Qconstructor.__name__,
                       "nPerh=%f" % nPerh,
                       "compatibleEnergy=%s" % compatibleEnergy,
                       "correctionOrder=%s" % correctionOrder,
                       "Cullen=%s" % boolCullenViscosity,
                       "Condc=%s" % HopkinsConductivity,
                       "filter=%f" % filter,
                       "%i" % (nx1 + nx2))
restartDir = os.path.join(dataDir, "restarts")
restartBaseName = os.path.join(restartDir, "Sod-planar-3d-%i" % (nx1 + nx2))

vizDir = os.path.join(dataDir, "visit")
if vizTime is None and vizCycle is None:
    vizBaseName = None
else:
    vizBaseName = "Sod-planar-3d-%i" % (nx1 + nx2)

assert numNodeLists in (1, 2)

#-------------------------------------------------------------------------------
# Check if the necessary output directories exist.  If not, create them.
#-------------------------------------------------------------------------------
if mpi.rank == 0:
    if clearDirectories and os.path.exists(dataDir):
        shutil.rmtree(dataDir)
    if not os.path.exists(restartDir):
        os.makedirs(restartDir)
    if not os.path.exists(vizDir):
        os.makedirs(vizDir)
mpi.barrier()

#-------------------------------------------------------------------------------
# Material properties.
#-------------------------------------------------------------------------------
eos = GammaLawGasMKS(gammaGas, mu)
strength = NullStrength()

#-------------------------------------------------------------------------------
# Interpolation kernels.
#-------------------------------------------------------------------------------
if KernelConstructor==NBSplineKernel:
  WT = TableKernel(NBSplineKernel(order), 1000)
else:
  WT = TableKernel(KernelConstructor(), 1000)
kernelExtent = WT.kernelExtent
output("WT")

#-------------------------------------------------------------------------------
# Make the NodeLists.
#-------------------------------------------------------------------------------
if solid:
    makeNL = makeSolidNodeList
else:
    makeNL = makeFluidNodeList
nodes1 = makeNL("nodes1", eos, 
                hmin = hmin,
                hmax = hmax,
                nPerh = nPerh,
                kernelExtent = kernelExtent,
                rhoMin = rhoMin)
nodes2 = makeNL("nodes2", eos, 
                hmin = hmin,
                hmax = hmax,
                nPerh = nPerh,
                kernelExtent = kernelExtent,
                rhoMin = rhoMin)
nodeSet = [nodes1, nodes2]

#-------------------------------------------------------------------------------
# A function to specify the density profile.
#-------------------------------------------------------------------------------
dx1 = (x1 - x0)/nx1
dx2 = (x2 - x1)/nx2
hfold = hsmooth*max(dx1, dx2)
def rho_initial(posi):
    xi = posi.x
    if xi <= x1 - hfold:
        return rho1
    elif xi > x1 + hfold:
        return rho2
    else:
        f = 0.5*(sin(0.5*pi*(xi - x1)/hfold) + 1.0)
        return (1.0 - f)*rho1 + f*rho2

#-------------------------------------------------------------------------------
# Set the node properties.
#-------------------------------------------------------------------------------
from GenerateNodeDistribution3d import GenerateNodeDistribution3d
gen1 = GenerateNodeDistribution3d(nx1, ny1, nz1, rho_initial, "lattice",
                                  xmin = (x0, y0, z0),
                                  xmax = (x1, y1, z1),
                                  nNodePerh = nPerh,
                                  SPH = True)
gen2 = GenerateNodeDistribution3d(nx2, ny2, nz2, rho_initial, "lattice",
                                  xmin = (x1, y0, z0),
                                  xmax = (x2, y1, z1),
                                  nNodePerh = nPerh,
                                  SPH = True)

if mpi.procs > 1:
    from VoronoiDistributeNodes import distributeNodes3d
else:
    from DistributeNodes import distributeNodes3d

if numNodeLists == 1:
    from CompositeNodeDistribution import CompositeNodeDistribution
    gen = CompositeNodeDistribution(gen1, gen2)
    distributeNodes3d((nodes1, gen))
else:
    distributeNodes3d((nodes1, gen1),
                      (nodes2, gen2))

for n in nodeSet:
    output("n.name")
    output("   mpi.reduce(n.numInternalNodes, mpi.MIN)")
    output("   mpi.reduce(n.numInternalNodes, mpi.MAX)")
    output("   mpi.reduce(n.numInternalNodes, mpi.SUM)")
del n

# Set node specific thermal energies
def specificEnergy(xi, rhoi):
    if xi <= x1 - hfold:
        Pi = P1
    elif xi > x1 + hfold:
        Pi = P2
    else:
        f = 0.5*(sin(0.5*pi*(xi - x1)/hfold) + 1.0)
        Pi = (1.0 - f)*P1 + f*P2
    return Pi/((gammaGas - 1.0)*rhoi)

for nodes in nodeSet:
    pos = nodes.positions()
    eps = nodes.specificThermalEnergy()
    rho = nodes.massDensity()
    for i in xrange(nodes.numInternalNodes):
        eps[i] = specificEnergy(pos[i].x, rho[i])

#-------------------------------------------------------------------------------
# Construct a DataBase to hold our node list
#-------------------------------------------------------------------------------
db = DataBase()
output("db")
for nodes in nodeSet:
    output("db.appendNodeList(nodes)")
output("db.numNodeLists")
output("db.numFluidNodeLists")

#-------------------------------------------------------------------------------
# Construct the artificial viscosity.
#-------------------------------------------------------------------------------
try:
    q = Qconstructor(Cl, Cq, linearInExpansion, quadraticInExpansion)
except:
    q = Qconstructor(Cl, Cq)
q.limiter = Qlimiter
q.epsilon2 = epsilon2
output("q")
output("q.Cl")
output("q.Cq")
output("q.limiter")
output("q.epsilon2")
try:
    output("q.linearInExpansion")
    output("q.quadraticInExpansion")
except:
    pass

#-------------------------------------------------------------------------------
# Construct the hydro physics object.
#-------------------------------------------------------------------------------
if SVPH:
    hydro = HydroConstructor(W = WT,
                             Q = q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             XSVPH = XSPH,
                             linearConsistent = linearConsistent,
                             generateVoid = False,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate,
                             xmin = Vector(-100.0),
                             xmax = Vector( 100.0))
elif CRKSPH:
    hydro = HydroConstructor(W = WT, 
                             Q = q,
                             filter = filter,
                             cfl = cfl,
                             correctionOrder = correctionOrder,
                             volumeType = volumeType,
                             compatibleEnergyEvolution = compatibleEnergy,
                             evolveTotalEnergy = evolveTotalEnergy,
                             XSPH = XSPH,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate)
    q.etaCritFrac = etaCritFrac
    q.etaFoldFrac = etaFoldFrac
elif PSPH:
    hydro = HydroConstructor(W = WT,
                             Q = q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             evolveTotalEnergy = evolveTotalEnergy,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate,
                             XSPH = XSPH,
                             correctVelocityGradient = correctVelocityGradient,
                             HopkinsConductivity = HopkinsConductivity)
else:
    hydro = HydroConstructor(W = WT,
                             Q = q,
                             cfl = cfl,
                             compatibleEnergyEvolution = compatibleEnergy,
                             evolveTotalEnergy = evolveTotalEnergy,
                             gradhCorrection = gradhCorrection,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate,
                             XSPH = XSPH,
                             correctVelocityGradient = correctVelocityGradient,
                             epsTensile = epsilonTensile,
                             nTensile = nTensile)
output("hydro")

packages = [hydro]

#-------------------------------------------------------------------------------
# Construct the MMRV physics object.
#-------------------------------------------------------------------------------
if boolReduceViscosity:
    evolveReducingViscosityMultiplier = MorrisMonaghanReducingViscosity(q,nh,nh,aMin,aMax)
    packages.append(evolveReducingViscosityMultiplier)
elif boolCullenViscosity:
    evolveCullenViscosityMultiplier = CullenDehnenViscosity(q,WT,alphMax,alphMin,betaC,betaD,betaE,fKern,boolHopkinsCorrection)
    packages.append(evolveCullenViscosityMultiplier)

#-------------------------------------------------------------------------------
# Construct the Artificial Conduction physics object.
#-------------------------------------------------------------------------------
if bArtificialConduction:
    #q.reducingViscosityCorrection = True
    ArtyCond = ArtificialConduction(WT,arCondAlpha)
    
    packages.append(ArtyCond)

#-------------------------------------------------------------------------------
# Create boundary conditions.
#-------------------------------------------------------------------------------
xPlane0 = Plane(Vector(x0, y0, z0), Vector( 1.0,  0.0,  0.0))
yPlane0 = Plane(Vector(x0, y0, z0), Vector( 0.0,  1.0,  0.0))
zPlane0 = Plane(Vector(x0, y0, z0), Vector( 0.0,  0.0,  1.0))
xPlane1 = Plane(Vector(x2, y1, z1), Vector(-1.0,  0.0,  0.0))
yPlane1 = Plane(Vector(x2, y1, z1), Vector( 0.0, -1.0,  0.0))
zPlane1 = Plane(Vector(x2, y1, z1), Vector( 0.0,  0.0, -1.0))

xbc0 = ReflectingBoundary(xPlane0)
ybc0 = ReflectingBoundary(yPlane0)
zbc0 = ReflectingBoundary(zPlane0)
xbc1 = ReflectingBoundary(xPlane1)
ybc1 = ReflectingBoundary(yPlane1)
zbc1 = ReflectingBoundary(zPlane1)

for p in packages:
    for bc in [xbc0, ybc0, zbc0, xbc1, ybc1, zbc1]:
        p.appendBoundary(bc)
del p, bc

#-------------------------------------------------------------------------------
# Construct an integrator, and add the one physics package.
#-------------------------------------------------------------------------------
integrator = IntegratorConstructor(db)
for p in packages:
    integrator.appendPhysicsPackage(p)
integrator.lastDt = dt
integrator.dtGrowth = dtGrowth
if dtMin:
    integrator.dtMin = dtMin
if dtMax:
    integrator.dtMax = dtMax
integrator.rigorousBoundaries = rigorousBoundaries
integrator.verbose = dtverbose
output("integrator")
output("integrator.havePhysicsPackage(hydro)")
output("integrator.lastDt")
output("integrator.dtMin")
output("integrator.dtMax")
output("integrator.rigorousBoundaries")

#-------------------------------------------------------------------------------
# Make the problem controller.
#-------------------------------------------------------------------------------
control = SpheralController(integrator, WT,
                            statsStep = statsStep,
                            restartStep = restartStep,
                            restartBaseName = restartBaseName,
                            restoreCycle = restoreCycle,
                            vizBaseName = vizBaseName,
                            vizDir = vizDir,
                            vizStep = vizCycle,
                            vizTime = vizTime,
                            SPH = True)
output("control")

#-------------------------------------------------------------------------------
# If requested, reset the intial energies and densities.
#-------------------------------------------------------------------------------
if sumInitialDensity and control.totalSteps == 0:
    packages = integrator.physicsPackages()
    state = State(db, packages)
    derivs = StateDerivatives(db, packages)
    integrator.setGhostNodes()
    integrator.preStepInitialize(state, derivs)
    cm = db.connectivityMap()
    pos = db.fluidPosition
    mass = db.fluidMass
    H = db.fluidHfield
    rho = db.fluidMassDensity
    computeSPHSumMassDensity(cm, WT, True, pos, mass, H, rho)
    for nodes in nodeSet:
        pos = nodes.positions()
        eps = nodes.specificThermalEnergy()
        rho = nodes.massDensity()
        for i in xrange(nodes.numInternalNodes):
            eps[i] = specificEnergy(pos[i].x, rho[i])

#-------------------------------------------------------------------------------
# Advance to the end time.
#-------------------------------------------------------------------------------
if not steps is None:
    control.step(steps)

    # Are we doing the restart test?
    if checkRestart:
        state0 = State(db, integrator.physicsPackages())
        state0.copyState()
        control.loadRestartFile(control.totalSteps)
        state1 = State(db, integrator.physicsPackages())
        if not state1 == state0:
            raise ValueError, "The restarted state does not match!"
        else:
            print "Restart check PASSED."

else:
    control.advance(goalTime, maxSteps)
    control.dropRestartFile()

#-------------------------------------------------------------------------------
# Plot the final state.
#-------------------------------------------------------------------------------
dx1 = (x1 - x0)/nx1
dx2 = (x2 - x1)/nx2
h1 = 1.0/(nPerh*dx1)
h2 = 1.0/(nPerh*dx2)
answer = SodSolution(nPoints=nx1 + nx2,
                     gamma = gammaGas,
                     rho1 = rho1,
                     P1 = P1,
                     rho2 = rho2,
                     P2 = P2,
                     x0 = x0,
                     x1 = x1,
                     x2 = x2,
                     h1 = 1.0/h1,
                     h2 = 1.0/h2)

#cs = db.newFluidScalarFieldList(0.0, "sound speed")
#db.fluidSoundSpeed(cs)
cs = hydro.soundSpeed()

# Make a flat list from a FieldList
def createList(x):
    result = []
    for i in xrange(len(x)):
        for j in xrange(x[i].numInternalElements):
            result.append(x(i,j))
    return mpi.allreduce(result)

# Compute the simulated specific entropy.
rho = createList(db.fluidMassDensity)
P = createList(hydro.pressure())
A = [Pi/rhoi**gammaGas for (Pi, rhoi) in zip(P, rho)]

# The analytic solution for the simulated entropy.
xprof = [x.x for x in createList(db.fluidPosition)]
multiSort(xprof, rho, P, A)
xans, vans, uans, rhoans, Pans, hans = answer.solution(control.time(), xprof)
Aans = [Pi/rhoi**gammaGas for (Pi, rhoi) in zip(Pans,  rhoans)]
csAns = [sqrt(gammaGas*Pi/rhoi) for (Pi, rhoi) in zip(Pans,  rhoans)]

if graphics:
    import Gnuplot
    from SpheralGnuPlotUtilities import *

    rhoPlot, velPlot, epsPlot, PPlot, HPlot = plotState(db, plotStyle="points")
    plotAnswer(answer, control.time(),
               rhoPlot, velPlot, epsPlot, PPlot, HPlot)
    pE = plotEHistory(control.conserve)

    csPlot = plotFieldList(cs, winTitle="Sound speed", plotStyle="points")
    csAnsData = Gnuplot.Data(xans, csAns, 
                             with_ = "lines",
                             title = "Analytic")
    csPlot.replot(csAnsData)

    Aplot = generateNewGnuPlot()
    Adata = Gnuplot.Data(xprof, A,
                         with_ = "points",
                         title = "P/rho^\gamma",
                         inline = True)
    AansData = Gnuplot.Data(xprof, Aans,
                         with_ = "points",
                         title = "Solution",
                         inline = True)
    Aplot.replot(Adata)
    Aplot.replot(AansData)

    plots = [(rhoPlot, "Sod-planar-rho.png"),
             (velPlot, "Sod-planar-vel.png"),
             (epsPlot, "Sod-planar-eps.png"),
             (PPlot, "Sod-planar-P.png"),
             (HPlot, "Sod-planar-h.png"),
             (csPlot, "Sod-planar-cs.png"),
             (Aplot, "Sod-planar-entropy.png")]
    
    if CRKSPH:
        APlot = plotFieldList(hydro.A(),
                              winTitle = "A",
                              plotStyle = "points",
                              colorNodeLists = False)
        BPlot = plotFieldList(hydro.B(),
                              yFunction = "%s.x",
                              winTitle = "B",
                              plotStyle = "points",
                              colorNodeLists = False)
        plots += [(APlot, "Sod-planar-A.png"),
                  (BPlot, "Sod-planar-B.png")]
        derivs = StateDerivatives(db, integrator.physicsPackages())
        drhodt = derivs.scalarFields("delta mass density")
        pdrhodt = plotFieldList(drhodt, winTitle = "DrhoDt", plotStyle = "points", colorNodeLists=False)
    
    viscPlot = plotFieldList(hydro.maxViscousPressure(),
                             winTitle = "max(rho^2 Piij)",
                             plotStyle = "points",
                             colorNodeLists = False)
    plots.append((viscPlot, "Sod-planar-viscosity.png"))
    
    if boolCullenViscosity:
        cullAlphaPlot = plotFieldList(q.ClMultiplier(),
                                      plotStyle = "points",
                                      winTitle = "Cullen alpha")
        cullDalphaPlot = plotFieldList(evolveCullenViscosityMultiplier.DalphaDt(),
                                       plotStyle = "points",
                                       winTitle = "Cullen DalphaDt")
        plots += [(cullAlphaPlot, "Sod-planar-Cullen-alpha.png"),
                  (cullDalphaPlot, "Sod-planar-Cullen-DalphaDt.png")]

    if boolReduceViscosity:
        alphaPlot = plotFieldList(q.ClMultiplier(),
                                  winTitle = "rvAlpha",
                                  plotStyle = "points",
                                  colorNodeLists = False)

    # Make hardcopies of the plots.
    for p, filename in plots:
        p.hardcopy(os.path.join(dataDir, filename), terminal="png")

print "Energy conservation: original=%g, final=%g, error=%g" % (control.conserve.EHistory[0],
                                                                control.conserve.EHistory[-1],
                                                                (control.conserve.EHistory[-1] - control.conserve.EHistory[0])/control.conserve.EHistory[0])
