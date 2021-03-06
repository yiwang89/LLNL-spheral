# ===========================================================================
# File:   configure.ac
# Author: Martin Casado
#
# Copyright (C) 2000 University of California Regents               
# ===========================================================================

# Process this file with autoconf to produce a configure script.
AC_INIT(Physics/Physics.hh)

AC_PREFIX_DEFAULT("`pwd`/`uname -s`_`uname -p`")

# All mcaro files are stored in macros/
AC_CONFIG_AUX_DIR(m4macros)

# We want the INSTALL script.
AC_SUBST(INSTALL)
#INSTALL=`pwd`/m4macros/install-sh
INSTALL=`which install`

# -------------------------------------------------
# Setup python
# macro definitions are in m4macros/python.m4
# -------------------------------------------------
AC_PROG_PYTHON

# -------------------------------------------------
# setup python binding (m4macros/pythonbinding.m4)
# -------------------------------------------------
SETUP_PYTHONBINDING

# -------------------------------------------------
# Setup the Spheral Environment 
# (m4macros/spheral.m4)
# -------------------------------------------------
SETUP_SPHERAL_ENV

# -------------------------------------------------
# Setup the instantions
# (m4macros/instantiations.m4)
# -------------------------------------------------
SETUP_INSTANTIATIONS

# -------------------------------------------------
# Setup the DistributedMake options
# (m4macros/distributedmake.m4)
# -------------------------------------------------
SETUP_DISTRIBUTEDMAKE

# -------------------------------------------------
# setup geommem (m4macros/geommem.m4)
# -------------------------------------------------
SETUP_GEOMMEM

# -------------------------------------------------
# setup polytope (m4macros/polytope.m4)
# -------------------------------------------------
SETUP_POLYTOPE

# -------------------------------------------------
# setup mpi (m4macros/mpi.m4)
# -------------------------------------------------
SETUP_MPI

# -----------------------------------------------
# allow user to define compiler at the command line, plus look
# for KCC first, defined in m4macros/compilers.m4
# -----------------------------------------------
SETUP_COMPILERS_THE_WAY_I_WANT

# ----------------------------------------------
# Setup up compiler flags for spheral depending
# on the compiler
# ----------------------------------------------
AC_CHECK_OPTIMIZATION
SETUP_DEBUG_PRINT
SETUP_DBC

# -------------------------------------------------
# setup ParMETIS (m4macros/mpi.m4)
# -------------------------------------------------
SETUP_PARMETIS

# -------------------------------------------------
# setup postgres support (m4macros/postgres.m4)
# -------------------------------------------------
SETUP_POSTGRES

# -------------------------------------------------
# setup SPH Gravity stuff (m4macros/SPHGravity.m4)
# -------------------------------------------------
SETUP_SPHGRAVITY

# -------------------------------------------------
# setup MHD stuff (m4macros/mhd.m4)
# -------------------------------------------------
SETUP_MHD

# -------------------------------------------------
# setup FractalGravity (m4macros/FractalGravity.m4)
# -------------------------------------------------
SETUP_FRACTALGRAVITY

# -------------------------------------------------
# setup PETSc (m4macros/petsc.m4)
# -------------------------------------------------
SETUP_PETSC

# -------------------------------------------------
# setup matplotlib (m4macros/matplotlib.m4)
# -------------------------------------------------
SETUP_MATPLOTLIB

# -------------------------------------------------
# setup scipy (m4macros/scipy.m4)
# -------------------------------------------------
SETUP_SCIPY

# -------------------------------------------------
# setup opensubdiv (m4macros/opensubdiv.m4)
# -------------------------------------------------
SETUP_OPENSUBDIV

# -------------------------------------------------
# setup ANEOS (m4macros/aneos.m4)
# -------------------------------------------------
SETUP_ANEOS

# -------------------------------------------------
# setup GEODYN (m4macros/geodyn.m4)
# -------------------------------------------------
SETUP_GEODYN

# -------------------------------------------------
# setup HELMHOLTZ (m4macros/helmholtz.m4)
# -------------------------------------------------
SETUP_HELMHOLTZ

# -----------------------------------------------
# Output the processed AC files
# -----------------------------------------------
AC_OUTPUT(helpers/makefile_master \
          makefile \
          thirdPartyLibs/makefile \
          Geometry/makefile \
          NodeList/makefile \
          Field/makefile \
          FieldOperations/makefile \
          Kernel/makefile \
          Material/makefile \
          Neighbor/makefile \
          DataBase/makefile \
          Boundary/makefile \
          Distributed/makefile \
          Physics/makefile \
          ArtificialViscosity/makefile \
          Hydro/makefile \
          ExternalForce/makefile \
          Gravity/makefile \
          Integrator/makefile \
          FileIO/makefile \
          DataOutput/makefile \
          Utilities/makefile \
          NodeGenerators/makefile \
          SimulationControl/makefile \
          SPH/makefile \
          CRKSPH/makefile \
          SVPH/makefile \
          TaylorSPH/makefile \
          Mesh/makefile \
          Damage/makefile \
          SolidMaterial/makefile \
          Strength/makefile \
          ArtificialConduction/makefile \
          CXXTests/makefile \
          PythonMPIInterfaces/makefile \
          PBGWraps/makefile \
          PBGWraps/SpheralModulesBindings.py \
          Pybind11Wraps/makefile \
          Pybind11Wraps/Spheral.py \
          Pybind11Wraps/CXXTypes/makefile \
          Pybind11Wraps/Geometry/makefile \
          Pybind11Wraps/Silo/makefile \
          Pybind11Wraps/DataOutput/makefile \
          Pybind11Wraps/NodeList/makefile \
          Pybind11Wraps/Field/makefile \
          Pybind11Wraps/Kernel/makefile \
          Pybind11Wraps/Neighbor/makefile \
          Pybind11Wraps/Material/makefile \
          Pybind11Wraps/FileIO/makefile \
          Pybind11Wraps/DataBase/makefile \
          Pybind11Wraps/Boundary/makefile \
          Pybind11Wraps/Physics/makefile \
          Pybind11Wraps/ArtificialViscosity/makefile \
          Pybind11Wraps/Hydro/makefile \
          SimulationControl/spheralDimensions.py)
