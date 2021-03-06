#ifndef _InterFacePublic_Defined_
#define _InterFacePublic_Defined_
namespace FractalSpace
{
//! Headers for regular function calls
  Fractal_Memory* FractalGravityFirstTime(int NumberParticles,
					  int balance,
					  int FractalNodes0,
					  int FractalNodes1,
					  int FractalNodes2,
					  int FFTNodes,
					  bool Periodic,
					  bool Debug,
					  int GridLength,
					  int Padding,
					  int LevelMax,
					  int MinimumNumber,
					  int MaxHypreIterations,
					  double HypreTolerance,
					  string BaseDirectory,
					  string RunIdentifier,
					  bool TimeTrial,
					  MPI_Comm& TalkToMe);
  void FractalGravity(Fractal_Memory* PFM,int NumberParticles,vector <double>& xmin,vector <double>& xmax, double G);
  void FractalGravityFinal(Fractal_Memory* PFM);
  //! One-Stop-Routine for setting up the Fractal_Memory object

  void do_fractal_force(Fractal_Memory* PFM);
  //! Wrapper for actually running the Poisson Solver

  Fractal_Memory* fractal_memory_create();
  //! Create a Fractal_Memory object

  void fractal_memory_setup(Fractal_Memory* PFM);
  //! Setup Fractal_Memory with everything

  void fractal_memory_delete(Fractal_Memory* PFM);
  //! Delete a Fractal_Memory object

  void fractal_memory_content_delete(Fractal_Memory* PFM);
  //! Delete, gracefully, all contents of a Fractal_Memory object

  void fractal_create(Fractal_Memory* PFM);
  //! Create a Fractal object

  void fractal_delete(Fractal_Memory* PFM);
  //! Delete a Fractal object

  void add_particles(Fractal_Memory* PFM,int first,int total,
		    vector <double>& xmin,vector <double>& xmax,
		    vector <double>& posx,vector <double>& posy,
		    vector <double>& posz,vector <double>& masses);
  //! Add positions and masses to the Fractal object. This can be done in multiple steps.
  //! xmin,xmax are the positions of the lower left and upper right corners
  //! of the User's computational cube. Really BAAAAD things can happen if not all particles
  //! are addded.

  void get_field(Fractal_Memory* PFM,int first,int total,double G,
		vector <double>& xmin,vector <double>& xmax,
		vector <double>& pot,vector <double>& fx,
		vector <double>& fy,vector <double>& fz);
//! Receive potential and forces from Fractal object. G is the User's gravitational constant

  void get_potential(Fractal_Memory* PFM,int first,int total,double G,
		    vector <double>& xmin,vector <double>& xmax,
		    vector <double>& pot);
  bool I_am_a_real_particle(Fractal_Memory*PFM,int ni);

//!
//! Headers for Fractal_Memory class function calls.
//! void Fractal_Memory::setBalance(int Balance)
//! void Fractal_Memory::setNumberParticles(int number_particles)
//! void Fractal_Memory::setFractalNodes(int FractalNodes0,int FractalNodes1,int FractalNodes2)
//! void Fractal_Memory::setPeriodic(bool periodic)
//! void Fractal_Memory::setDebug(bool FractalDebug)
//! void Fractal_Memory::setGridLength(int grid_length)
//! void Fractal_Memory::setPadding(int padding)
//! void Fractal_Memory::setLevelMax(int level_max)
//! void Fractal_Memory::setMinimumNumber(int minimum_number)
//! void Fractal_Memory::setHypreIterations(int maxitsHypre)
//! void Fractal_Memory::setHypreTolerance(double tolHypre)
//! void Fractal_Memory::setBaseDirectory(string BaseDirectory)
//! void Fractal_Memory::setRunIdentifier(string RunIdentifier)
//!
//! Parameters for fractal_interface_setup, MUST BE SET.
//! balance         (0) => equal volume domains (easy but slow)
//!                 (1) => equal # particles + level 0 points for each domain (significant speedup)
//! periodic        ( true if periodic BC, false if isolated BC)
//! grid_length     ( length of fundamental grid) 
//!                 ( must be even for periodic simulation)
//! level_max       ( number of levels of refinement (usually 8))
//! minimum_number  ( minimum number of particles belonging to a high point (usually 8))
//! padding         (-1) => resolution jumps by factors of 2 (cheap)
//!                 (1)  => each high point fully padded (expensive)
//!                 (0)  => no padding (fast, but not recommended)
//! 
//! tolHypre        ( accuracy of hypre solver (1.0e-7 recommended))
//! maxitsHypre     ( max number of iterations in Hypre (20 recommended))
//! fractalDebug    ( debug run (true) extra IO)
//! FractalNodes0   ( Number of processors in x-direction)
//! FractalNodes1   ( Number of processors in y-direction)
//! FractalNodes2   ( Number of processors in z-direction)
//! BaseDirectory   ( (string) Base directory for Fractal output, absolute path, ends with /)
//! RunIdentifier   ( (string) Unique Identifier, no spaces)
}
#endif
