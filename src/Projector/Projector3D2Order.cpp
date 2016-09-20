#include "Projector3D2Order.h"

#include <cmath>
#include <iostream>

#include "ElectroMagn.h"
#include "Field3D.h"
#include "Particles.h"
#include "Tools.h"
#include "Patch.h"

using namespace std;


// ---------------------------------------------------------------------------------------------------------------------
// Constructor for Projector3D2Order
// ---------------------------------------------------------------------------------------------------------------------
Projector3D2Order::Projector3D2Order (Params& params, Patch* patch) : Projector3D(params, patch)
{
    dx_inv_   = 1.0/params.cell_length[0];
    dx_ov_dt  = params.cell_length[0] / params.timestep;
    dy_inv_   = 1.0/params.cell_length[1];
    dy_ov_dt  = params.cell_length[1] / params.timestep;
    dz_inv_   = 1.0/params.cell_length[2];
    dz_ov_dt  = params.cell_length[2] / params.timestep;
    
    one_third = 1.0/3.0;

    i_domain_begin = patch->getCellStartingGlobalIndex(0);
    j_domain_begin = patch->getCellStartingGlobalIndex(1);
    k_domain_begin = patch->getCellStartingGlobalIndex(2);

    DEBUG("cell_length "<< params.cell_length[0]);

}


// ---------------------------------------------------------------------------------------------------------------------
// Destructor for Projector3D2Order
// ---------------------------------------------------------------------------------------------------------------------
Projector3D2Order::~Projector3D2Order()
{
}


// ---------------------------------------------------------------------------------------------------------------------
//! Project local currents (sort)
// ---------------------------------------------------------------------------------------------------------------------
void Projector3D2Order::operator() (double* Jx, double* Jy, double* Jz, Particles &particles, unsigned int ipart, double gf, unsigned int bin, std::vector<unsigned int> &b_dim, int* iold, double* deltaold)
{

    // -------------------------------------
    // Variable declaration & initialization
    // -------------------------------------
    
    // (x,y,z) components of the current density for the macro-particle
    double charge_weight = (double)(particles.charge(ipart))*particles.weight(ipart);
    double crx_p = charge_weight*dx_ov_dt;
    double cry_p = charge_weight*dy_ov_dt;
    double crz_p = charge_weight*dz_ov_dt;
#ifdef _PATCH3D_TODO
#endif
    
    // variable declaration
    double xpn, ypn, zpn;
    double delta, delta2;
    // arrays used for the Esirkepov projection method
    double Sx0[5], Sx1[5], Sy0[5], Sy1[5], Sz0[5], Sz1[5], DSx[5], DSy[5], DSz[5];
    double Wx[5][5][5], Wy[5][5][5], Wz[5][5][5], Jx_p[5][5][5], Jy_p[5][5][5], Jz_p[5][5][5];
    
    for (unsigned int i=0; i<5; i++) {
        Sx1[i] = 0.;
        Sy1[i] = 0.;
        Sz1[i] = 0.;
    }
    for (unsigned int i=0; i<5; i++) {
        for (unsigned int j=0; j<5; j++) {
            for (unsigned int k=0; k<5; k++) {
                Jx_p[i][j][k] = 0.;
                Jy_p[i][j][k] = 0.;
                Jz_p[i][j][k] = 0.;
            }
        }
    }
    
    // --------------------------------------------------------
    // Locate particles & Calculate Esirkepov coef. S, DS and W
    // --------------------------------------------------------
    
    // locate the particle on the primal grid at former time-step & calculate coeff. S0
    delta = *deltaold;
    delta2 = delta*delta;
    Sx0[0] = 0.;
    Sx0[1] = 0.5 * (delta2-delta+0.25);
    Sx0[2] = 0.75-delta2;
    Sx0[3] = 0.5 * (delta2+delta+0.25);
    Sx0[4] = 0.;
    
    delta = *(deltaold+1);
    delta2 = delta*delta;
    Sy0[0] = 0.;
    Sy0[1] = 0.5 * (delta2-delta+0.25);
    Sy0[2] = 0.75-delta2;
    Sy0[3] = 0.5 * (delta2+delta+0.25);
    Sy0[4] = 0.;
    
    delta = *(deltaold+2);
    delta2 = delta*delta;
    Sz0[0] = 0.;
    Sz0[1] = 0.5 * (delta2-delta+0.25);
    Sz0[2] = 0.75-delta2;
    Sz0[3] = 0.5 * (delta2+delta+0.25);
    Sz0[4] = 0.;
    
    // locate the particle on the primal grid at current time-step & calculate coeff. S1
    xpn = particles.position(0, ipart) * dx_inv_;
    int ip = round(xpn);
    int ipo = *iold;
    int ip_m_ipo = ip-ipo-i_domain_begin;
    delta  = xpn - (double)ip;
    delta2 = delta*delta;
    Sx1[ip_m_ipo+1] = 0.5 * (delta2-delta+0.25);
    Sx1[ip_m_ipo+2] = 0.75-delta2;
    Sx1[ip_m_ipo+3] = 0.5 * (delta2+delta+0.25);
    
    ypn = particles.position(1, ipart) * dy_inv_;
    int jp = round(ypn);
    int jpo = *(iold+1);
    int jp_m_jpo = jp-jpo-j_domain_begin;
    delta  = ypn - (double)jp;
    delta2 = delta*delta;
    Sy1[jp_m_jpo+1] = 0.5 * (delta2-delta+0.25);
    Sy1[jp_m_jpo+2] = 0.75-delta2;
    Sy1[jp_m_jpo+3] = 0.5 * (delta2+delta+0.25);
    
    zpn = particles.position(2, ipart) * dz_inv_;
    int kp = round(zpn);
    int kpo = *(iold+2);
    int kp_m_kpo = kp-kpo-k_domain_begin;
    delta  = zpn - (double)kp;
    delta2 = delta*delta;
    Sz1[kp_m_kpo+1] = 0.5 * (delta2-delta+0.25);
    Sz1[kp_m_kpo+2] = 0.75-delta2;
    Sz1[kp_m_kpo+3] = 0.5 * (delta2+delta+0.25);
    
    // computes Esirkepov coefficients
    for (unsigned int i=0; i < 5; i++) {
        DSx[i] = Sx1[i] - Sx0[i];
        DSy[i] = Sy1[i] - Sy0[i];
        DSz[i] = Sz1[i] - Sz0[i];
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Wx[i][j][k] = DSx[i] * (Sy0[j]*Sz0[k] + 0.5*DSy[j]*Sz0[k] + 0.5*DSz[k]*Sy0[j] + one_third*DSy[j]*DSz[k]);
                Wy[i][j][k] = DSy[j] * (Sz0[k]*Sx0[i] + 0.5*DSz[k]*Sx0[i] + 0.5*DSx[i]*Sz0[k] + one_third*DSz[k]*DSx[i]);
                Wz[i][j][k] = DSz[k] * (Sx0[i]*Sy0[j] + 0.5*DSx[i]*Sy0[j] + 0.5*DSy[j]*Sx0[i] + one_third*DSx[i]*DSy[j]);
            }
        }
    }
    
    // ------------------------------------------------
    // Local current created by the particle
    // calculate using the charge conservation equation
    // ------------------------------------------------
    for (unsigned int i=1 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Jx_p[i][j][k] = Jx_p[i-1][j][k] - crx_p * Wx[i-1][j][k];
            }
        }
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=1 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Jy_p[i][j][k] = Jy_p[i][j-1][k] - cry_p * Wy[i][j-1][k];
            }
        }
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=1 ; k<5 ; k++) {
                Jz_p[i][j][k] = Jz_p[i][j][k-1] - crz_p * Wz[i][j][k-1];
            }
        }
    }
    
    // ---------------------------
    // Calculate the total current
    // ---------------------------
    
    ipo -= bin+2;   //This minus 2 come from the order 2 scheme, based on a 5 points stencil from -2 to +2.
    // i/j/kpo stored with - i/j/k_domain_begin in Interpolator
    jpo -= 2;
    kpo -= 2;
    
    int iloc, jloc, kloc, linindex;
    
    // Jx^(d,p,p)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*b_dim[2]*b_dim[1]+jloc*b_dim[2]+kloc;
                Jx [linindex] += Jx_p[i][j][k]; // iloc = (i+ipo)*b_dim[1];
            }
        }
    }//i
    
    // Jy^(p,d,p)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*b_dim[2]*(b_dim[1]+1)+jloc*b_dim[2]+kloc;
                Jy [linindex] += Jy_p[i][j][k]; //
            }
        }
    }//i
    
    // Jz^(p,p,d)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*(b_dim[2]+1)*b_dim[1]+jloc*(b_dim[2]+1)+kloc;
                Jz [linindex] += Jz_p[i][j][k]; //
            }
        }
    }//i
    
#ifdef _PATCH3D_TODO
#endif
    
} // END Project local current densities (Jx, Jy, Jz, sort)


// ---------------------------------------------------------------------------------------------------------------------
//! Project local current densities (sort)
// ---------------------------------------------------------------------------------------------------------------------
void Projector3D2Order::operator() (double* Jx, double* Jy, double* Jz, double* rho, Particles &particles, unsigned int ipart, double gf, unsigned int bin, std::vector<unsigned int> &b_dim, int* iold, double* deltaold)
{

    // -------------------------------------
    // Variable declaration & initialization
    // -------------------------------------
    
    // (x,y,z) components of the current density for the macro-particle
    double charge_weight = (double)(particles.charge(ipart))*particles.weight(ipart);
    double crx_p = charge_weight*dx_ov_dt;
    double cry_p = charge_weight*dy_ov_dt;
    double crz_p = charge_weight*dz_ov_dt;
#ifdef _PATCH3D_TODO
#endif
    
    // variable declaration
    double xpn, ypn, zpn;
    double delta, delta2;
    // arrays used for the Esirkepov projection method
    double Sx0[5], Sx1[5], Sy0[5], Sy1[5], Sz0[5], Sz1[5], DSx[5], DSy[5], DSz[5];
    double Wx[5][5][5], Wy[5][5][5], Wz[5][5][5], Jx_p[5][5][5], Jy_p[5][5][5], Jz_p[5][5][5];
    
    for (unsigned int i=0; i<5; i++) {
        Sx1[i] = 0.;
        Sy1[i] = 0.;
        Sz1[i] = 0.;
    }
    for (unsigned int i=0; i<5; i++) {
        for (unsigned int j=0; j<5; j++) {
            for (unsigned int k=0; k<5; k++) {
                Jx_p[i][j][k] = 0.;
                Jy_p[i][j][k] = 0.;
                Jz_p[i][j][k] = 0.;
            }
        }
    }
    
    // --------------------------------------------------------
    // Locate particles & Calculate Esirkepov coef. S, DS and W
    // --------------------------------------------------------
    
    // locate the particle on the primal grid at former time-step & calculate coeff. S0
    delta = *deltaold;
    delta2 = delta*delta;
    Sx0[0] = 0.;
    Sx0[1] = 0.5 * (delta2-delta+0.25);
    Sx0[2] = 0.75-delta2;
    Sx0[3] = 0.5 * (delta2+delta+0.25);
    Sx0[4] = 0.;
    
    delta = *(deltaold+1);
    delta2 = delta*delta;
    Sy0[0] = 0.;
    Sy0[1] = 0.5 * (delta2-delta+0.25);
    Sy0[2] = 0.75-delta2;
    Sy0[3] = 0.5 * (delta2+delta+0.25);
    Sy0[4] = 0.;
    
    delta = *(deltaold+2);
    delta2 = delta*delta;
    Sz0[0] = 0.;
    Sz0[1] = 0.5 * (delta2-delta+0.25);
    Sz0[2] = 0.75-delta2;
    Sz0[3] = 0.5 * (delta2+delta+0.25);
    Sz0[4] = 0.;
    
    // locate the particle on the primal grid at current time-step & calculate coeff. S1
    xpn = particles.position(0, ipart) * dx_inv_;
    int ip = round(xpn);
    int ipo = *iold;
    int ip_m_ipo = ip-ipo-i_domain_begin;
    delta  = xpn - (double)ip;
    delta2 = delta*delta;
    Sx1[ip_m_ipo+1] = 0.5 * (delta2-delta+0.25);
    Sx1[ip_m_ipo+2] = 0.75-delta2;
    Sx1[ip_m_ipo+3] = 0.5 * (delta2+delta+0.25);
    
    ypn = particles.position(1, ipart) * dy_inv_;
    int jp = round(ypn);
    int jpo = *(iold+1);
    int jp_m_jpo = jp-jpo-j_domain_begin;
    delta  = ypn - (double)jp;
    delta2 = delta*delta;
    Sy1[jp_m_jpo+1] = 0.5 * (delta2-delta+0.25);
    Sy1[jp_m_jpo+2] = 0.75-delta2;
    Sy1[jp_m_jpo+3] = 0.5 * (delta2+delta+0.25);
    
    zpn = particles.position(2, ipart) * dz_inv_;
    int kp = round(zpn);
    int kpo = *(iold+2);
    int kp_m_kpo = kp-kpo-k_domain_begin;
    delta  = zpn - (double)kp;
    delta2 = delta*delta;
    Sz1[kp_m_kpo+1] = 0.5 * (delta2-delta+0.25);
    Sz1[kp_m_kpo+2] = 0.75-delta2;
    Sz1[kp_m_kpo+3] = 0.5 * (delta2+delta+0.25);
    
    // computes Esirkepov coefficients
    for (unsigned int i=0; i < 5; i++) {
        DSx[i] = Sx1[i] - Sx0[i];
        DSy[i] = Sy1[i] - Sy0[i];
        DSz[i] = Sz1[i] - Sz0[i];
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Wx[i][j][k] = DSx[i] * (Sy0[j]*Sz0[k] + 0.5*DSy[j]*Sz0[k] + 0.5*DSz[k]*Sy0[j] + one_third*DSy[j]*DSz[k]);
                Wy[i][j][k] = DSy[j] * (Sz0[k]*Sx0[i] + 0.5*DSz[k]*Sx0[i] + 0.5*DSx[i]*Sz0[k] + one_third*DSz[k]*DSx[i]);
                Wz[i][j][k] = DSz[k] * (Sx0[i]*Sy0[j] + 0.5*DSx[i]*Sy0[j] + 0.5*DSy[j]*Sx0[i] + one_third*DSx[i]*DSy[j]);
            }
        }
    }
    
    // ------------------------------------------------
    // Local current created by the particle
    // calculate using the charge conservation equation
    // ------------------------------------------------
    for (unsigned int i=1 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Jx_p[i][j][k] = Jx_p[i-1][j][k] - crx_p * Wx[i-1][j][k];
            }
        }
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=1 ; j<5 ; j++) {
            for (unsigned int k=0 ; k<5 ; k++) {
                Jy_p[i][j][k] = Jy_p[i][j-1][k] - cry_p * Wy[i][j-1][k];
            }
        }
    }
    for (unsigned int i=0 ; i<5 ; i++) {
        for (unsigned int j=0 ; j<5 ; j++) {
            for (unsigned int k=1 ; k<5 ; k++) {
                Jz_p[i][j][k] = Jz_p[i][j][k-1] - crz_p * Wz[i][j][k-1];
            }
        }
    }
    
    // ---------------------------
    // Calculate the total current
    // ---------------------------

    ipo -= bin+2;   //This minus 2 come from the order 2 scheme, based on a 5 points stencil from -2 to +2.
                    // i/j/kpo stored with - i/j/k_domain_begin in Interpolator
    jpo -= 2;
    kpo -= 2;
    
    int iloc, jloc, kloc, linindex;
    
    // Jx^(d,p,p)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*b_dim[2]*b_dim[1]+jloc*b_dim[2]+kloc;
                Jx [linindex] += Jx_p[i][j][k]; // iloc = (i+ipo)*b_dim[1];
            }
        }
    }//i

    // Jy^(p,d,p)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*b_dim[2]*(b_dim[1]+1)+jloc*b_dim[2]+kloc;
                Jy [linindex] += Jy_p[i][j][k]; //
            }
        }
    }//i
    
    // Jz^(p,p,d)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*(b_dim[2]+1)*b_dim[1]+jloc*(b_dim[2]+1)+kloc;
                Jz [linindex] += Jz_p[i][j][k]; //
            }
        }
    }//i

    // Rho^(p,p,p)
    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = i+ipo;
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = j+jpo;
            for (unsigned int k=0 ; k<5 ; k++) {
                kloc = k+kpo;
                linindex = iloc*b_dim[2]*b_dim[1]+jloc*b_dim[2]+kloc;
                rho[linindex] += charge_weight * Sx1[i]*Sy1[j]*Sz1[k];
            }
        }
    }//i

#ifdef _PATCH3D_TODO
#endif


} // END Project local densities (Jx, Jy, Jz, rho, sort)


// ---------------------------------------------------------------------------------------------------------------------
//! Project local densities only (Frozen species)
// ---------------------------------------------------------------------------------------------------------------------
void Projector3D2Order::operator() (double* rho, Particles &particles, unsigned int ipart, unsigned int bin, std::vector<unsigned int> &b_dim)
{
    //Warning : this function is used for frozen species only. It is assumed that position = position_old !!!

    // -------------------------------------
    // Variable declaration & initialization
    // -------------------------------------

    int iloc,jloc,kloc;
    // (x,y,z) components of the current density for the macro-particle
    double charge_weight = (double)(particles.charge(ipart))*particles.weight(ipart);

    // variable declaration
    double xpn, ypn, zpn;
    double delta, delta2;
    double Sx1[5], Sy1[5], Sz1[5]; // arrays used for the Esirkepov projection method

// Initialize all current-related arrays to zero
    for (unsigned int i=0; i<5; i++) {
        Sx1[i] = 0.;
        Sy1[i] = 0.;
        Sz1[i] = 0.;
    }

    // --------------------------------------------------------
    // Locate particles & Calculate Esirkepov coef. S, DS and W
    // --------------------------------------------------------

    // locate the particle on the primal grid at current time-step & calculate coeff. S1
    xpn = particles.position(0, ipart) * dx_inv_;
    int ip = round(xpn);
    delta  = xpn - (double)ip;
    delta2 = delta*delta;
    Sx1[1] = 0.5 * (delta2-delta+0.25);
    Sx1[2] = 0.75-delta2;
    Sx1[3] = 0.5 * (delta2+delta+0.25);

    ypn = particles.position(1, ipart) * dy_inv_;
    int jp = round(ypn);
    delta  = ypn - (double)jp;
    delta2 = delta*delta;
    Sy1[1] = 0.5 * (delta2-delta+0.25);
    Sy1[2] = 0.75-delta2;
    Sy1[3] = 0.5 * (delta2+delta+0.25);

    zpn = particles.position(2, ipart) * dz_inv_;
    int kp = round(zpn);
    delta  = zpn - (double)kp;
    delta2 = delta*delta;
    Sz1[1] = 0.5 * (delta2-delta+0.25);
    Sz1[2] = 0.75-delta2;
    Sz1[3] = 0.5 * (delta2+delta+0.25);

    // ---------------------------
    // Calculate the total charge
    // ---------------------------
    ip -= i_domain_begin + bin +2;
    jp -= j_domain_begin + 2;
    kp -= k_domain_begin + 2;

    for (unsigned int i=0 ; i<5 ; i++) {
        iloc = (i+ip)*b_dim[2]*b_dim[1];
        for (unsigned int j=0 ; j<5 ; j++) {
            jloc = (jp+j)*b_dim[2];
            for (unsigned int k=0 ; k<5 ; k++) {
                rho[iloc+jloc+kp+k] += charge_weight * Sx1[i]*Sy1[j]*Sz1[k];
            }
        }
    }//i

} // END Project local current densities (Frozen species)

// ---------------------------------------------------------------------------------------------------------------------
//! Project global current densities (ionize)
// ---------------------------------------------------------------------------------------------------------------------
void Projector3D2Order::operator() (Field* Jx, Field* Jy, Field* Jz, Particles &particles, int ipart, LocalFields Jion)
{
    ERROR("Projection of ionization current not yet defined for 3D 2nd order");

} // END Project global current densities (ionize)

//Wrapper for projection
void Projector3D2Order::operator() (ElectroMagn* EMfields, Particles &particles, SmileiMPI* smpi, int istart, int iend, int ithread, int ibin, int clrw, int diag_flag, std::vector<unsigned int> &b_dim, int ispec)
{
    std::vector<int> *iold = &(smpi->dynamics_iold[ithread]);
    std::vector<double> *delta = &(smpi->dynamics_deltaold[ithread]);
    std::vector<double> *gf = &(smpi->dynamics_gf[ithread]);
    
    int dim1 = EMfields->dimPrim[1];
    int dim2 = EMfields->dimPrim[2];

    if (diag_flag == 0){ 
	double* b_Jx =  &(*EMfields->Jx_ )(ibin*clrw* dim1   * dim2   );
	double* b_Jy =  &(*EMfields->Jy_ )(ibin*clrw*(dim1+1)* dim2   );
	double* b_Jz =  &(*EMfields->Jz_ )(ibin*clrw* dim1   *(dim2+1));
        for (unsigned int ipart=istart ; ipart<iend; ipart++ )
    	    (*this)(b_Jx , b_Jy , b_Jz , particles,  ipart, (*gf)[ipart], ibin*clrw, b_dim, &(*iold)[3*ipart], &(*delta)[3*ipart]);
    } else {
	double* b_Jx =  &(*EMfields->Jx_s[ispec] )(ibin*clrw*dim1*dim2);
	double* b_Jy =  &(*EMfields->Jy_s[ispec] )(ibin*clrw*(dim1+1)*dim2);
	double* b_Jz =  &(*EMfields->Jz_s[ispec] )(ibin*clrw*dim1*(dim2+1));
	double* b_rho = &(*EMfields->rho_s[ispec])(ibin*clrw*dim1*dim2);
        for (unsigned int ipart=istart ; ipart<iend; ipart++ )
	    (*this)(b_Jx , b_Jy , b_Jz ,b_rho, particles,  ipart, (*gf)[ipart], ibin*clrw, b_dim, &(*iold)[3*ipart], &(*delta)[3*ipart]);
    }

}
