#ifndef _plant_parameters_h
#define _plant_parameters_h

#include <math.h>

#include "mymath.h"

/* Mechanic transmission parameters */
/* Motor pulley radius m */
#define r_pm 7e-3

/* encoder pulley largest (external) radius */
#define r_ee 35e-3

/* encoder pulley smallest (internal) radius */
#define r_ei 8e-3
/* rack pulley largest (external) radius */
#define r_ce 40e-3
/* rack pulley smallest (internal) radius, in contact with rack */
#define r_ci 10e-3

/* material poliacetilene density kg/m3 */
#define d_PA 1410

/* motor pulley thickness */
#define e_pm 5e-3

/* largest encoder pulley thickness */
#define e_ee 5e-3

/* smallest encoder pulley thickness */
#define e_ei 5e-3

/* largest rack pulley thickness */
#define e_ce 5e-3

/* smallest rack pulley thickness */
#define e_ci 5e-3

/* Number of hollow sections in encoder pulley */
#define N_ef 4

/* Number of hollow sections in rack pulley */
#define N_cf 4

/* frame thickness for encoder pulley */
#define e_ep 3e-3

/* frame thickness for rack pulley */
#define e_cp 3e-3

/* ray of encoder pulley hollow section */
#define r_ef ((r_ee - 2 * e_ep - r_ei) / 2)

/* ray of rack pulley hollow section */
#define r_cf ((r_ce - 2 * e_cp - r_ci) / 2)

/* encoder pulley hollow section distance from center */
#define r_ep r_ei + e_ep + r_ef

/* rack pulley hollow section distance from center */
#define r_cp r_ci + e_cp + r_cf

/* cilinder friction constant */
#define b_e 0.2

/* motor axis angular friction constant */
#define B_m 8.3e-6

/* motor pulley angular friction constant */
#define B_pm 10e-6

/* combined encoder pulley angular friction constant  */
#define B_pe 10e-6

/* encoder axis angular friction constant */
#define B_e 10e-6

/* combined rack pulley angular friction constant */
#define B_pc 10e-6

/* mass of piston kg */
#define m_e 0.37

/* inertia momentum of motor rotor */
#define J_m 20e-6

/* inertia momentum of motor pulley */
#define J_pm (d_PA * M_PI * e_pm * sqr4(r_pm) / 2) 

/* inertia momentum of encoder */
#define J_e 10e-6

/* inertia momentum of encoder pulley */
#define J_pe (d_PA * M_PI * (e_ee * sqr4(r_ee)/2 + e_ei * sqr4(r_ei) - N_ef * (e_ee * sqr4(r_ef) + sqr(r_ef) * e_ee * sqr(r_ep))))

/* inertia momentum of rack pulley */
#define J_pc (d_PA * M_PI * (e_ce * sqr4(r_ce)/2 + e_ci * sqr4(r_ci) - N_cf * (e_ce * sqr4(r_cf) + sqr(r_cf) * e_ce * sqr(r_cp))))

/* equivalent angular friction at motor axis reference */
#define B_eq_motor (B_m + B_pm + (B_e + B_pe) * sqr(r_pm / r_ee) + B_pc * sqr((r_ei * r_pm) / (r_ce * r_ee)) + b_e * r_ci * sqr((r_ei * r_pm) / (r_ce * r_ee)))

/* equivalent inercia momentum at motor axis reference */
#define J_eq_motor (J_m + J_pm + (J_e + J_pe) * sqr(r_pm / r_ee) + J_pc * sqr((r_ei * r_pm) / (r_ce * r_ee)) + m_e * r_ci * sqr((r_ei * r_pm) / (r_ce * r_ee)))

/* piston internal diameter */
#define Demb 69e-3

/* piston transversal internal area  */
#define A_e (M_PI * Demb * Demb / 4)

/* input pressure for O2 in Pa */
#define rho_O2 200e3

/* air pressure in Pa */
#define rho_air 101000

/* motor maximum speed */
#define OMEGA_M_MAX (12777 / 60 * 2 * M_PI)

/* encoder maximun speed rad/s */
#define OMEGA_E_MAX (OMEGA_M_MAX * r_pm / r_ee)

#endif
