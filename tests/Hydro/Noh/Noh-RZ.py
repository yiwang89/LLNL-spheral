#-------------------------------------------------------------------------------
# The Noh test case run in RZ symmetry.
#
# W.F. Noh 1987, JCP, 72, 78-120.
#-------------------------------------------------------------------------------
import os, shutil, mpi
from SolidSpheral2d import *
from SpheralTestUtilities import *

from GenerateNodeDistribution2d import *
if mpi.procs > 1:
    from VoronoiDistributeNodes import distributeNodes2d
else:
    from DistributeNodes import distributeNodes2d

title("RZ hydro test -- Noh problem")

#-------------------------------------------------------------------------------
# Generic problem parameters
#-------------------------------------------------------------------------------
commandLine(problem = "planar",     # one of (planar, cylindrical, spherical)
            KernelConstructor = BSplineKernel,
            order = 5,

            nr = 20,
            nz = 100,

            r0 = 0.0,
            r1 = 0.2,
            z0 = 0.0,
            z1 = 1.0,
            zwall = 0.0,

            nPerh = 1.35,

            gamma = 5.0/3.0,
            mu = 1.0,

            solid = False,    # If true, use the fluid limit of the solid hydro option

            CRKSPH = False,
            PSPH = False,
            SPH = True,       # Choose the H advancement
            evolveTotalEnergy = False,  # Only for SPH variants -- evolve total rather than specific energy
            Qconstructor = MonaghanGingoldViscosityRZ,
            boolReduceViscosity = False,
            HopkinsConductivity = False,     # For PSPH
            nhQ = 5.0,
            nhL = 10.0,
            aMin = 0.1,
            aMax = 2.0,
            boolCullenViscosity = False,
            cullenUseHydroDerivatives = True,  # Reuse the hydro calculation of DvDx.
            alphMax = 2.0,
            alphMin = 0.02,
            betaC = 0.7,
            betaD = 0.05,
            betaE = 1.0,
            fKern = 1.0/3.0,
            boolHopkinsCorrection = True,
            linearConsistent = False,
            fcentroidal = 0.0,
            fcellPressure = 0.0,
            Qhmult = 1.0,
            Cl = 1.0, 
            Cq = 1.0,
            etaCritFrac = 1.0,
            etaFoldFrac = 0.2,
            Qlimiter = False,
            balsaraCorrection = False,
            epsilon2 = 1e-2,
            hmin = 0.0001, 
            hmax = 0.1,
            hminratio = 0.1,
            cfl = 0.5,
            useVelocityMagnitudeForDt = False,
            XSPH = False,
            epsilonTensile = 0.0,
            nTensile = 4.0,
            hourglass = None,
            hourglassOrder = 0,
            hourglassLimiter = 0,
            hourglassFraction = 0.5,
            filter = 0.0,

            IntegratorConstructor = CheapSynchronousRK2Integrator,
            goalTime = 0.6,
            steps = None,
            dt = 0.0001,
            dtMin = 1.0e-5, 
            dtMax = 0.1,
            dtGrowth = 2.0,
            dtverbose = False,
            rigorousBoundaries = False,
            maxSteps = None,
            statsStep = 1,
            vizCycle = None,
            vizTime = 0.1,
            vizDerivs = False,
            HUpdate = IdealH,
            correctionOrder = LinearOrder,
            QcorrectionOrder = LinearOrder,
            volumeType = CRKSumVolume,
            densityUpdate = RigorousSumDensity, # VolumeScaledDensity,
            compatibleEnergy = True,
            gradhCorrection = False,
            correctVelocityGradient = False,
            domainIndependent = False,
            cullGhostNodes = True,
            
            bArtificialConduction = False,
            arCondAlpha = 0.5,

            clearDirectories = True,
            checkError = False,
            checkRestart = False,
            checkEnergy = False,
            restoreCycle = None,
            restartStep = 10000,
            outputFile = "None",
            comparisonFile = "None",
            normOutputFile = "None",
            writeOutputLabel = True,

            graphics = True,
            )

assert not(boolReduceViscosity and boolCullenViscosity)
   
assert problem in ("planar", "cylindrical", "spherical")
rho0 = 1.0
eps0 = 0.0
if problem == "planar":
    vr0, vz0 = 0.0, -1.0
elif problem == "cylindrical":
    vr0, vz0 = -1.0, 0.0
else:
    vr0, vz0 = -1.0, -1.0

if CRKSPH:
   if solid:
      if SPH:
         HydroConstructor = SolidCRKSPHHydroRZ
      else:
         HydroConstructor = SolidACRKSPHHydroRZ
   else:
      if SPH:
         HydroConstructor = CRKSPHHydroRZ
      else:
         HydroConstructor = ACRKSPHHydroRZ
      Qconstructor = CRKSPHMonaghanGingoldViscosityRZ
      gradhCorrection = False
else:
   if solid:
      if SPH:
         HydroConstructor = SolidSPHHydroRZ
      else:
         HydroConstructor = SolidASPHHydroRZ
   else:
      if SPH:
         HydroConstructor = SPHHydroRZ
      else:
         HydroConstructor = ASPHHydroRZ

dataDir = os.path.join("dumps-%s-Noh-RZ" % problem,
                       HydroConstructor.__name__,
                       Qconstructor.__name__,
                       "nPerh=%f" % nPerh,
                       "compatibleEnergy=%s" % compatibleEnergy,
                       "Cullen=%s" % boolCullenViscosity,
                       "filter=%f" % filter)
restartDir = os.path.join(dataDir, "restarts")
restartBaseName = os.path.join(restartDir, "Noh-%s-RZ" % problem)

vizDir = os.path.join(dataDir, "visit")
if vizTime is None and vizCycle is None:
    vizBaseName = None
else:
    vizBaseName = "Noh-planar-RZ"

dr = (r1 - r0)/nr
dz = (z1 - z0)/nz

#-------------------------------------------------------------------------------
# Check if the necessary output directories exist.  If not, create them.
#-------------------------------------------------------------------------------
import os, sys
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
eos = GammaLawGasMKS(gamma, mu)
strength = NullStrength()

#-------------------------------------------------------------------------------
# Interpolation kernels.
#-------------------------------------------------------------------------------
if KernelConstructor==NBSplineKernel:
    Wbase = NBSplineKernel(order)
else:
    Wbase = KernelConstructor()
WT = TableKernel(Wbase, 1000)
kernelExtent = WT.kernelExtent
output("WT")

#-------------------------------------------------------------------------------
# Make the NodeList.
#-------------------------------------------------------------------------------
if solid:
    nodes1 = makeSolidNodeList("nodes1", eos, strength,
                               hmin = hmin,
                               hmax = hmax,
                               hminratio = hminratio,
                               nPerh = nPerh,
                               xmin = (-10.0, -10.0),
                               xmax = ( 10.0,  10.0),
                               kernelExtent = kernelExtent)
else:
    nodes1 = makeFluidNodeList("nodes1", eos, 
                               hmin = hmin,
                               hmax = hmax,
                               hminratio = hminratio,
                               nPerh = nPerh,
                               xmin = (-10.0, -10.0),
                               xmax = ( 10.0,  10.0),
                               kernelExtent = kernelExtent)
    
output("nodes1")
output("nodes1.hmin")
output("nodes1.hmax")
output("nodes1.nodesPerSmoothingScale")

#-------------------------------------------------------------------------------
# Set the node properties.
#-------------------------------------------------------------------------------
generator = GenerateNodeDistributionRZ(nz, nr, rho0, "lattice",
                                       xmin = (z0, r0),
                                       xmax = (z1, r1),
                                       nNodePerh = nPerh,
                                       SPH = SPH)

distributeNodes2d((nodes1, generator))
output("mpi.reduce(nodes1.numInternalNodes, mpi.MIN)")
output("mpi.reduce(nodes1.numInternalNodes, mpi.MAX)")
output("mpi.reduce(nodes1.numInternalNodes, mpi.SUM)")

# Set node specific thermal energies
nodes1.specificThermalEnergy(ScalarField("tmp", nodes1, eps0))
nodes1.massDensity(ScalarField("tmp", nodes1, rho0))

# Set node velocities
pos = nodes1.positions()
vel = nodes1.velocity()
for i in xrange(nodes1.numNodes):
    vel[i].y = vr0
    if pos[i].x > zwall:
        vel[i].x = vz0
    else:
        vel[i].x = -vz0

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
q.balsaraShearCorrection = balsaraCorrection
q.QcorrectionOrder = QcorrectionOrder
output("q")
output("q.Cl")
output("q.Cq")
output("q.epsilon2")
output("q.limiter")
output("q.balsaraShearCorrection")

#-------------------------------------------------------------------------------
# Construct the hydro physics object.
#-------------------------------------------------------------------------------
if CRKSPH:
    hydro = HydroConstructor(W = WT,
                             Q = q,
                             filter = filter,
                             cfl = cfl,
                             useVelocityMagnitudeForDt = useVelocityMagnitudeForDt,
                             compatibleEnergyEvolution = compatibleEnergy,
                             evolveTotalEnergy = evolveTotalEnergy,
                             XSPH = XSPH,
                             correctionOrder = correctionOrder,
                             volumeType = volumeType,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate)
    q.etaCritFrac = etaCritFrac
    q.etaFoldFrac = etaFoldFrac
else:
    hydro = HydroConstructor(W = WT,
                             Q = q,
                             filter = filter,
                             cfl = cfl,
                             useVelocityMagnitudeForDt = useVelocityMagnitudeForDt,
                             compatibleEnergyEvolution = compatibleEnergy,
                             evolveTotalEnergy = evolveTotalEnergy,
                             gradhCorrection = gradhCorrection,
                             correctVelocityGradient = correctVelocityGradient,
                             densityUpdate = densityUpdate,
                             HUpdate = HUpdate,
                             XSPH = XSPH,
                             epsTensile = epsilonTensile,
                             nTensile = nTensile)
output("hydro")
output("hydro.kernel()")
output("hydro.PiKernel()")
output("hydro.cfl")
output("hydro.compatibleEnergyEvolution")
output("hydro.densityUpdate")
output("hydro.HEvolution")

packages = [hydro]

#-------------------------------------------------------------------------------
# Construct the MMRV physics object.
#-------------------------------------------------------------------------------
if boolReduceViscosity:
    evolveReducingViscosityMultiplier = MorrisMonaghanReducingViscosity(q,nhQ,nhL,aMin,aMax)
    packages.append(evolveReducingViscosityMultiplier)
elif boolCullenViscosity:
    evolveCullenViscosityMultiplier = CullenDehnenViscosity(q,WT,alphMax,alphMin,betaC,betaD,betaE,fKern,boolHopkinsCorrection,cullenUseHydroDerivatives)
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
bcs = []
if zwall == z0:
    zPlaneWall = Plane(Vector(zwall, 0.0), Vector(1.0, 0.0))
    bcs.append(ReflectingBoundary(zPlaneWall))

if r0 != 0.0:
    rPlane0 = Plane(Vector(0.0, r0), Vector(0.0, 1.0))
    bcs.append(ReflectingBoundary(rPlane0))

rPlane1 = Plane(Vector(0.0, r1), Vector(0.0, -1.0))
bcs.append(ReflectingBoundary(rPlane1))

for bc in bcs:
    for p in packages:
        p.appendBoundary(bc)

#-------------------------------------------------------------------------------
# Construct an integrator.
#-------------------------------------------------------------------------------
integrator = IntegratorConstructor(db)
for p in packages:
    integrator.appendPhysicsPackage(p)
del p
integrator.lastDt = dt
integrator.dtMin = dtMin
integrator.dtMax = dtMax
integrator.dtGrowth = dtGrowth
integrator.rigorousBoundaries = rigorousBoundaries
integrator.domainDecompositionIndependent = domainIndependent
integrator.verbose = dtverbose
output("integrator")
output("integrator.lastDt")
output("integrator.dtMin")
output("integrator.dtMax")
output("integrator.dtGrowth")
output("integrator.rigorousBoundaries")
output("integrator.domainDecompositionIndependent")
output("integrator.verbose")

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
                            vizDerivs = vizDerivs,
                            SPH = SPH)
output("control")

#-------------------------------------------------------------------------------
# Advance to the end time.
#-------------------------------------------------------------------------------
if not steps is None:
    control.step(steps)

else:
   control.advance(goalTime, maxSteps)

#-------------------------------------------------------------------------------
# Compute the analytic answer.
#-------------------------------------------------------------------------------
import mpi
import NohAnalyticSolution
if problem == "planar":
    xprof = mpi.allreduce([x.x for x in nodes1.positions().internalValues()], mpi.SUM)
    h1 = 1.0/(nPerh*dz)
    answer = NohAnalyticSolution.NohSolution(1,
                                             r = xprof,
                                             v0 = -1.0,
                                             h0 = 1.0/h1)
elif problem == "cylindrical":
    xprof = mpi.allreduce([x.y for x in nodes1.positions().internalValues()], mpi.SUM)
    h1 = 1.0/(nPerh*dr)
    answer = NohAnalyticSolution.NohSolution(2,
                                             r = xprof,
                                             v0 = -1.0,
                                             h0 = 1.0/h1)
else:
    xprof = mpi.allreduce([x.magnitude() for x in nodes1.positions().internalValues()], mpi.SUM)
    h1 = 1.0/(nPerh*dr)
    answer = NohAnalyticSolution.NohSolution(3,
                                             r = xprof,
                                             v0 = -1.0,
                                             h0 = 1.0/h1)

# Compute the simulated specific entropy.
rho = mpi.allreduce(nodes1.massDensity().internalValues(), mpi.SUM)
Pf = ScalarField("pressure", nodes1)
nodes1.pressure(Pf)
P = mpi.allreduce(Pf.internalValues(), mpi.SUM)
A = [Pi/rhoi**gamma for (Pi, rhoi) in zip(P, rho)]

# Solution profiles.
xans, vans, uans, rhoans, Pans, hans = answer.solution(control.time(), xprof)
Aans = [Pi/rhoi**gamma for (Pi, rhoi) in zip(Pans,  rhoans)]
L1 = 0.0
for i in xrange(len(rho)):
    L1 = L1 + abs(rho[i]-rhoans[i])
L1_tot = L1 / len(rho)
if mpi.rank == 0 and outputFile != "None":
    print "L1=",L1_tot,"\n"
    with open("Converge.txt", "a") as myfile:
        myfile.write("%s %s\n" % (nz, L1_tot))

#-------------------------------------------------------------------------------
# Plot the final state.
#-------------------------------------------------------------------------------
if graphics:
    from SpheralGnuPlotUtilities import *
    if problem == "planar":
        rhoPlot, velPlot, epsPlot, PPlot, HPlot = plotState(db, xFunction="%s.x", vecyFunction="%s.x", tenyFunction="1.0/%s.xx")
    elif problem == "cylindrical":
        rhoPlot, velPlot, epsPlot, PPlot, HPlot = plotState(db, xFunction="%s.y", vecyFunction="%s.y", tenyFunction="1.0/%s.yy")
    else:
        rhoPlot, velPlot, epsPlot, PPlot, HPlot = plotRadialState(db)
    plotAnswer(answer, control.time(), rhoPlot, velPlot, epsPlot, PPlot, HPlot)
    EPlot = plotEHistory(control.conserve)
    plots = [(rhoPlot, "Noh-%s-rho-RZ.png" % problem),
             (velPlot, "Noh-%s-vel-RZ.png" % problem),
             (epsPlot, "Noh-%s-eps-RZ.png" % problem),
             (PPlot, "Noh-%s-P-RZ.png" % problem),
             (HPlot, "Noh-%s-h-RZ.png" % problem)]

    # Plot the specific entropy.
    Aplot = generateNewGnuPlot()
    AsimData = Gnuplot.Data(xprof, A,
                            with_ = "points",
                            title = "Simulation",
                            inline = True)
    AansData = Gnuplot.Data(xprof, Aans,
                            with_ = "lines",
                            title = "Solution",
                            inline = True)
    Aplot.plot(AsimData)
    Aplot.replot(AansData)
    Aplot.title("Specific entropy")
    Aplot.refresh()
    plots.append((Aplot, "Noh-planar-A.png"))
    
    if CRKSPH:
        volPlot = plotFieldList(hydro.volume(), 
                                winTitle = "volume",
                                colorNodeLists = False, plotGhosts = False)
        plots.append((volPlot, "Noh-%s-vol.png" % problem))

    if boolCullenViscosity:
        cullAlphaPlot = plotFieldList(q.ClMultiplier(),
                                      winTitle = "Cullen alpha")
        cullDalphaPlot = plotFieldList(evolveCullenViscosityMultiplier.DalphaDt(),
                                       winTitle = "Cullen DalphaDt")
        plots += [(cullAlphaPlot, "Noh-%s-Cullen-alpha.png" % problem),
                  (cullDalphaPlot, "Noh-%s-Cullen-DalphaDt.png" % problem)]

    if boolReduceViscosity:
        alphaPlotQ = plotFieldList(q.reducingViscosityMultiplierQ(),
                                  winTitle = "rvAlphaQ",
                                  colorNodeLists = False, plotGhosts = False)
        alphaPlotL = plotFieldList(q.reducingViscosityMultiplierL(),
                                   winTitle = "rvAlphaL",
                                   colorNodeLists = False, plotGhosts = False)

    # Make hardcopies of the plots.
    for p, filename in plots:
        p.hardcopy(os.path.join(dataDir, filename), terminal="png")


# #-------------------------------------------------------------------------------
# # Measure the difference between the simulation and analytic answer.
# #-------------------------------------------------------------------------------
# rmin, rmax = 0.05, 0.35   # Throw away anything with r < rwall to avoid wall heating.
# rhoprof = mpi.reduce(nodes1.massDensity().internalValues(), mpi.SUM)
# P = ScalarField("pressure", nodes1)
# nodes1.pressure(P)
# Pprof = mpi.reduce(P.internalValues(), mpi.SUM)
# vprof = mpi.reduce([v.x for v in nodes1.velocity().internalValues()], mpi.SUM)
# epsprof = mpi.reduce(nodes1.specificThermalEnergy().internalValues(), mpi.SUM)
# hprof = mpi.reduce([1.0/H.xx for H in nodes1.Hfield().internalValues()], mpi.SUM)
# xprof = mpi.reduce([x.x for x in nodes1.positions().internalValues()], mpi.SUM)

# #-------------------------------------------------------------------------------
# # If requested, write out the state in a global ordering to a file.
# #-------------------------------------------------------------------------------
# if outputFile != "None":
#     outputFile = os.path.join(dataDir, outputFile)
#     from SpheralGnuPlotUtilities import multiSort
#     mof = mortonOrderIndices(db)
#     mo = mpi.reduce(mof[0].internalValues(), mpi.SUM)
#     mprof = mpi.reduce(nodes1.mass().internalValues(), mpi.SUM)
#     rhoprof = mpi.reduce(nodes1.massDensity().internalValues(), mpi.SUM)
#     P = ScalarField("pressure", nodes1)
#     nodes1.pressure(P)
#     Pprof = mpi.reduce(P.internalValues(), mpi.SUM)
#     vprof = mpi.reduce([v.x for v in nodes1.velocity().internalValues()], mpi.SUM)
#     epsprof = mpi.reduce(nodes1.specificThermalEnergy().internalValues(), mpi.SUM)
#     hprof = mpi.reduce([1.0/H.xx for H in nodes1.Hfield().internalValues()], mpi.SUM)
#     if mpi.rank == 0:
#         multiSort(mo, xprof, rhoprof, Pprof, vprof, epsprof, hprof)
#         f = open(outputFile, "w")
#         f.write(("#  " + 20*"'%s' " + "\n") % ("x", "m", "rho", "P", "v", "eps", "h", "mo",
#                                                "rhoans", "Pans", "vans", "epsans", "hans",
#                                                "x_UU", "m_UU", "rho_UU", "P_UU", "v_UU", "eps_UU", "h_UU"))
#         for (xi, mi, rhoi, Pi, vi, epsi, hi, moi,
#              rhoansi, Pansi, vansi, uansi, hansi) in zip(xprof, mprof, rhoprof, Pprof, vprof, epsprof, hprof, mo,
#                                                          rhoans, Pans, vans, uans, hans):
#             f.write((7*"%16.12e " + "%i " + 5*"%16.12e " + 7*"%i " + '\n') % 
#                     (xi, mi, rhoi, Pi, vi, epsi, hi, moi,
#                      rhoansi, Pansi, vansi, uansi, hansi,
#                      unpackElementUL(packElementDouble(xi)),
#                      unpackElementUL(packElementDouble(mi)),
#                      unpackElementUL(packElementDouble(rhoi)),
#                      unpackElementUL(packElementDouble(Pi)),
#                      unpackElementUL(packElementDouble(vi)),
#                      unpackElementUL(packElementDouble(epsi)),
#                      unpackElementUL(packElementDouble(hi))))
#         f.close()

#         # #---------------------------------------------------------------------------
#         # # Also we can optionally compare the current results with another file.
#         # #---------------------------------------------------------------------------
#         # if comparisonFile != "None":
#         #     comparisonFile = os.path.join(dataDir, comparisonFile)
#         #     import filecmp
#         #     assert filecmp.cmp(outputFile, comparisonFile)

# #------------------------------------------------------------------------------
# # Compute the error.
# #------------------------------------------------------------------------------
# if mpi.rank == 0:
#     xans, vans, epsans, rhoans, Pans, hans = answer.solution(control.time(), xprof)
#     import Pnorm
#     print "\tQuantity \t\tL1 \t\t\tL2 \t\t\tLinf"
#     failure = False
#     hD = []

#     if normOutputFile != "None":
#        f = open(normOutputFile, "a")
#        if writeOutputLabel:
#           f.write(("#" + 13*"%17s " + "\n") % ('"nx"',
#                                                '"rho L1"', '"rho L2"', '"rho Linf"',
#                                                '"P L1"',   '"P L2"',   '"P Linf"',
#                                                '"vel L1"', '"vel L2"', '"vel Linf"',
#                                                '"E L1"', '"E L2"', '"E Linf"',
#                                                '"h L1"',   '"h L2"',   '"h Linf"'))
#        f.write("%5i " % nz)
#     for (name, data, ans,
#          L1expect, L2expect, Linfexpect) in [("Mass Density", rhoprof, rhoans, L1rho, L2rho, Linfrho),
#                                              ("Pressure", Pprof, Pans, L1P, L2P, LinfP),
#                                              ("Velocity", vprof, vans, L1v, L2v, Linfv),
#                                              ("Thermal E", epsprof, epsans, L1eps, L2eps, Linfeps),
#                                              ("h       ", hprof, hans, L1h, L2h, Linfh)]:
#         assert len(data) == len(ans)
#         error = [data[i] - ans[i] for i in xrange(len(data))]
#         Pn = Pnorm.Pnorm(error, xprof)
#         L1 = Pn.gridpnorm(1, rmin, rmax)
#         L2 = Pn.gridpnorm(2, rmin, rmax)
#         Linf = Pn.gridpnorm("inf", rmin, rmax)
#         print "\t%s \t\t%g \t\t%g \t\t%g" % (name, L1, L2, Linf)
#         if normOutputFile != "None":
#            f.write((3*"%16.12e ") % (L1, L2, Linf))
#         hD.append([L1,L2,Linf])
        
           

#         if checkError:
#             if not fuzzyEqual(L1, L1expect, tol):
#                 print "L1 error estimate for %s outside expected bounds: %g != %g" % (name,
#                                                                                       L1,
#                                                                                       L1expect)
#                 failure = True
#             if not fuzzyEqual(L2, L2expect, tol):
#                 print "L2 error estimate for %s outside expected bounds: %g != %g" % (name,
#                                                                                       L2,
#                                                                                       L2expect)
#                 failure = True
#             if not fuzzyEqual(Linf, Linfexpect, tol):
#                 print "Linf error estimate for %s outside expected bounds: %g != %g" % (name,
#                                                                                         Linf,
#                                                                                         Linfexpect)
#                 failure = True
#             if failure:
#                 raise ValueError, "Error bounds violated."
#     if normOutputFile != "None":
#        f.write("\n")
                                             
#     # print "%d\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t %g\t" % (nz,hD[0][0],hD[1][0],hD[2][0],hD[3][0],
#     #                                                                             hD[0][1],hD[1][1],hD[2][1],hD[3][1],
#     #                                                                             hD[0][2],hD[1][2],hD[2][2],hD[3][2])

Eerror = (control.conserve.EHistory[-1] - control.conserve.EHistory[0])/control.conserve.EHistory[0]
print "Total energy error: %g" % Eerror
if checkEnergy and abs(Eerror) > 1e-13:
    raise ValueError, "Energy error outside allowed bounds."
