#ifndef _plant_parameters_h
#define _plant_parameters_h

#include <math.h>

#include "mymath.h"

/* Mechanic transmission parameters */
/* Motor pulley radius */
#define r_1 (15e-3 / 2) 
/* pulley larger radius */
#define r_2 (90e-3 / 2)
/* pulley smaller radius, in contact with rack */
#define r_3 (15e-3 / 2)

/* transmission ratio */
#define kPOL (r_1 / r_2)
  
/* cilinder friction constant */
#define b_e 0.2

/* motor pulley angular friction constant */
#define B_1 10e-6

/* rack axis angular friction constant  */
#define B_2 10e-6

/* motor axis angular friction constant */
#define B_m 10e-6

/* mass of piston kg */
#define m_e 0.37

/* inertia momentum of pulley */
#define J_2 100e-6

/* inertia momentum of motor pulley */
#define J_1 10e-6

/* inertia momentum of motor rotor */
#define J_m 20e-6

/* equivalent angular friction */
#define B_eq (b_e * (r_1 * r_3) / r_2 + B_2 * sqr(r_1/r_2) + B_1 + B_m)

/* equivalent inercia momentum */
#define J_eq (m_e * (r_1 * r_3) / r_2 + J_2 * sqr(r_1/r_2) + J_1 + J_m)

/* piston internal diameter */
#define Demb 69e-3

/* piston transversal internal  area  */
#define A_e (M_PI * Demb * Demb / 4)

/* input pressure for O2 in Pa */
#define rho_O2 200e3

/* air pressure in Pa */
#define rho_air 101000

#endif
