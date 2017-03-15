#ifndef DIAGNOSTICTRACK_H
#define DIAGNOSTICTRACK_H

#include "Diagnostic.h"

class Patch;
class Params;
class SmileiMPI;


class DiagnosticTrack : public Diagnostic {

public :
    //! Default constructor
    DiagnosticTrack( Params &params, SmileiMPI* smpi, Patch* patch, unsigned int, OpenPMDparams& );
    //! Default destructor
    ~DiagnosticTrack() override;
    
    void openFile( Params& params, SmileiMPI* smpi, bool newfile ) override;
    
    void closeFile() override;
    
    void init(Params& params, SmileiMPI* smpi, VectorPatch& vecPatches) override;
    
    bool prepare( int itime ) override;
    
    void run( SmileiMPI* smpi, VectorPatch& vecPatches, int itime ) override;
    
    //! Get memory footprint of current diagnostic
    int getMemFootPrint() override {
        return 0;
    }
    
    //! Fills a buffer with the required particle property
    template<typename T> void fill_buffer(VectorPatch& vecPatches, unsigned int iprop, std::vector<T>& buffer);
    
    //! Write a scalar dataset with the given buffer
    template<typename T> void write_scalar( hid_t, std::string, T&, hid_t, hid_t, hid_t, hid_t, unsigned int );
    
    //! Write a vector component dataset with the given buffer
    template<typename T> void write_component( hid_t, std::string, T&, hid_t, hid_t, hid_t, hid_t );
    
    //! Set a given patch's particles with the required IDs
    void setIDs(Patch *);
    
    //! Set a given particles with the required IDs
    void setIDs(Particles&);
    
    //! Index of the species used
    unsigned int speciesId_;
    
    //! Last ID assigned to a particle by this MPI domain
    uint64_t latest_Id;
    
private :
    
    //! Flag to test whether IDs have been set already
    bool IDs_done;
    
    //! HDF5 objects
    hid_t data_group_id, transfer;
     
    //! Number of spatial dimensions
    unsigned int nDim_particle;
    
    //! Current particle partition among the patches own by current MPI
    std::vector<unsigned int> patch_start;
    
    //! Buffer for the output of double array
    std::vector<double> data_double;
    //! Buffer for the output of short array
    std::vector<short> data_short;
    //! Buffer for the output of uint64 array
    std::vector<uint64_t> data_uint64;
};

#endif

