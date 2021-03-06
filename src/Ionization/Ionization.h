#ifndef IONIZATION_H
#define IONIZATION_H

#include <map>

#include "Tools.h"
#include "Params.h"
#include "Field.h"
#include "Particles.h"
#include "Projector.h"


//! Class Ionization: generic class allowing to define Ionization physics
class Ionization
{

public:
    //! Constructor for Ionization
    Ionization(Params& params, Species * species);
    virtual ~Ionization();
    
    //! Overloading of () operator
    virtual void operator() (Particles*, unsigned int, unsigned int, std::vector<LocalFields>*, ElectroMagn*, Projector*) {};
    
    Particles new_electrons;

protected:
    std::vector<double> Potential;
    std::vector<double> Azimuthal_quantum_number;
    
    double eV_to_au;
    double au_to_mec2;
    double EC_to_au;
    double au_to_w0;
    
    double referenceAngularFrequency_SI;
    double dt;
    double invdt;
    unsigned int nDim_field;
    unsigned int nDim_particle;
    unsigned int atomic_number_;
    unsigned int ionized_species_invmass;

private:


};

#endif
