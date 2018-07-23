#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_mymalloc.h"
#include "core_io_tree.h"

#include "io/read_tree_binary.h"
#ifdef HDF5
#include "io/read_tree_hdf5.h"
#include "io/read_tree_genesis_standard_hdf5.h"
#endif

void load_forest_table(const int ThisTask, const int filenr, const enum Valid_TreeTypes my_TreeType,
                       int *nforests, int **forestnhalos)
{
    switch (my_TreeType)
        {
#ifdef HDF5
        case genesis_lhalo_hdf5:
            load_forest_table_hdf5(ThisTask, filenr, nforests, forestnhalos);
            break;
            
        case genesis_standard_hdf5:
            load_forest_table_genesis_hdf5(ThisTask, filenr, nforests, forestnhalos);
            break;
#endif

        case lhalo_binary:
            (void) ThisTask;
            load_forest_table_binary(filenr, nforests, forestnhalos);
            break;

        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
        }

}

void free_forest_table(enum Valid_TreeTypes my_TreeType)
{
    /* Don't forget to free the open file handle */
    switch (my_TreeType) {
#ifdef HDF5
    case genesis_lhalo_hdf5:
        close_hdf5_file();
        break;
        
    case genesis_standard_hdf5:
        close_genesis_hdf5_file();
        break;
            
#endif
            
    case lhalo_binary:
        close_binary_file();
        break;
        
    default:
        fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
        fprintf(stderr, "Please add it there.\n");
        ABORT(EXIT_FAILURE);
        
    }
}


int load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
                struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal)
{

#ifndef HDF5
    (void) forestnr; /* forestnr is currently only used for the hdf5 files */
#endif    

    int32_t *orig_index=NULL;/* the original file index order */
    
    switch (my_TreeType)
        {
            
#ifdef HDF5
        case genesis_lhalo_hdf5:
            load_forest_hdf5(forestnr, nhalos, halos);
            break;
            
        case genesis_standard_hdf5:
            load_forest_genesis_hdf5(forestnr, nhalos, halos);
            break;
#endif            
            
        case lhalo_binary:
            load_forest_binary(nhalos, halos, &orig_index);
            break;
            
        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for ``%s`` in ``%s``.\n",
                    __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
            
        }

    int maxgals = (int)(MAXGALFAC * nhalos);
    if(maxgals < 10000) maxgals = 10000;

    *haloaux = mymalloc(sizeof(struct halo_aux_data) * nhalos);
    *halogal = mymalloc(sizeof(struct GALAXY) * maxgals);
    *galaxies = mymalloc(sizeof(struct GALAXY) * maxgals);/* used to be fof_maxgals instead of maxgals*/

    struct halo_aux_data *tmp_halo_aux = *haloaux;
    for(int i = 0; i < nhalos; i++) {
        tmp_halo_aux->DoneFlag = 0;
        tmp_halo_aux->HaloFlag = 0;
        tmp_halo_aux->NGalaxies = 0;
        tmp_halo_aux->orig_index = orig_index[i];
        tmp_halo_aux++;
    }
    /* orig_index was allocated within the corresponding
       load_forest_* function. The values have now been copied into
       `haloaux` -- so we can free the memory
    */
    free(orig_index);
    
    return maxgals;
}

