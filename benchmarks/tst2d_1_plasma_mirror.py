# ----------------------------------------------------------------------------------------
# 					SIMULATION PARAMETERS FOR THE PIC-CODE SMILEI
# ----------------------------------------------------------------------------------------

import math

l0 = 2.*math.pi			# laser wavelength
t0 = l0					# optical cycle
Lsim = [20.*l0,50.*l0]	# length of the simulation
Tsim = 50.*t0			# duration of the simulation
resx = 20.				# nb of cells in on laser wavelength
rest = 30.				# time of timestep in one optical cycle 


Main(
    geometry = "2d3v",
    
    interpolation_order = 2 ,
    
    cell_length = [l0/resx,l0/resx],
    sim_length  = Lsim,
    
    number_of_patches = [ 16, 8 ],
    
    timestep = t0/rest,
    sim_time = Tsim,
     
    bc_em_type_x = ['silver-muller'],
    bc_em_type_y = ['silver-muller'],
    
    random_seed = smilei_mpi_rank
)

LaserGaussian2D(
    boxSide         = "xmin",
    a0              = 0.1,
	focus           = [10.*l0, 25.0*l0],
    waist           = 5.0*l0,
    incidence_angle = 20./180.*math.pi,
    time_envelope   = tgaussian(fwhm=5.*t0, center=10.*t0)
)

Species(
	species_type = 'ion',
	initPosition_type = 'random',
	initMomentum_type = 'cold',
	ionization_model = 'none',
	n_part_per_cell = 5,
	c_part_max = 1.0,
	mass = 1836.0,
	charge = 1.0,
	nb_density = trapezoidal(2.0,xvacuum=10.*l0,xplateau=4.*l0),
	time_frozen = Tsim,
	bc_part_type_xmin  = 'refl',
	bc_part_type_xmax  = 'refl',
	bc_part_type_ymin = 'none',
	bc_part_type_ymax = 'none'
)

Species(
	species_type = 'eon',
	initPosition_type = 'random',
	initMomentum_type = 'cold',
	ionization_model = 'none',
	n_part_per_cell = 5,
	c_part_max = 1.0,
	mass = 1.0,
	charge = -1.0,
	nb_density = trapezoidal(2.0,xvacuum=10.*l0,xplateau=4.*l0),
	time_frozen = 0.,
	bc_part_type_xmin  = 'refl',
	bc_part_type_xmax  = 'refl',
	bc_part_type_ymin = 'none',
	bc_part_type_ymax = 'none'
)


DiagScalar(every=5)


DiagFields(
    every = 15,
    fields = ['Ex','Ey','Rho_eon']
)

