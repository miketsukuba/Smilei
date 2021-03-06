# ---------------------------------------------
# SIMULATION PARAMETERS FOR THE PIC-CODE SMILEI
# ---------------------------------------------

import math
L0 = 2.*math.pi # conversion from normalization length to wavelength


Main(
    geometry = "1d3v",

    number_of_patches = [ 4 ],

    interpolation_order = 2,

    timestep = 0.05 * L0,
    sim_time = 100 * L0,


    time_fields_frozen = 100000000000.,

    cell_length = [20.*L0],
    sim_length = [1600.*L0],

    bc_em_type_x = ["periodic"],


    random_seed = 0,

	referenceAngularFrequency_SI = L0 * 3e8 /1.e-6,
    print_every = 100,
)


Species(
	species_type = "electron1",
	initPosition_type = "regular",
	initMomentum_type = "maxwell-juettner",
	n_part_per_cell= 10,
	mass = 1.0,
	charge = -1.0,
	charge_density = 1.,
	mean_velocity = [0.03, 0., 0.],
	temperature = [0.0000001]*3,
	time_frozen = 100000000.0,
	bc_part_type_xmin = "none",
	bc_part_type_xmax = "none",
	bc_part_type_ymin = "none",
	bc_part_type_ymax = "none",
	c_part_max = 10.
)

Species(
	species_type = "ion1",
	initPosition_type = "regular",
	initMomentum_type = "maxwell-juettner",
	n_part_per_cell= 100,
	mass = 1836.0*13.,
	charge = 3.0,
	charge_density = 1.,
	mean_velocity = [0., 0., 0.],
	temperature = [0.00000001]*3,
	time_frozen = 100000000.0,
	bc_part_type_xmin = "none",
	bc_part_type_xmax = "none",
	bc_part_type_ymin = "none",
	bc_part_type_ymax = "none",
	atomic_number = 13
)


Collisions(
	species1 = ["electron1"],
	species2 = ["ion1"],
	coulomb_log = 3,
	ionizing = True
)




DiagFields(
	every = 1
)


DiagScalar(
	every = 1
)



DiagParticles(
	output = "charge_density",
	every = 20,
	species = ["ion1"],
	axes = [
		 ["x",    0,    Main.sim_length[0],   1]
	]
)

DiagParticles(
	output = "density",
	every = 20,
	species = ["ion1"],
	axes = [
		 ["x",    0,    Main.sim_length[0],   1]
	]
)


DiagParticles(
	output = "density",
	every = 20,
	species = ["electron1"],
	axes = [
		 ["px",    -0.3,    0.3,   1000]
	]
)

