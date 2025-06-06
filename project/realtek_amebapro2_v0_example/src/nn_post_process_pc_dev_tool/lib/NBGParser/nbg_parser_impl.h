/******************************************************************************\
|* Copyright (c) 2017-2023 by Vivante Corporation.  All Rights Reserved.      *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of Vivante Corporation.  This is proprietary information owned by Vivante  *|
|* Corporation.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of Vivante Corporation.                                 *|
|*                                                                            *|
\******************************************************************************/

#ifndef _NBG_PARSER_IMPL_H
#define _NBG_PARSER_IMPL_H

#include "gc_vip_nbg_format.h"


enum nbg_nn_command_size_e
{
    NBG_NN_COMMAND_SIZE_128 = 0,
    NBG_NN_COMMAND_SIZE_192 = 1,
};

typedef struct _nbg_reader
{
    vip_uint32_t    offset;
    vip_uint32_t    total_size;
    vip_uint8_t     *data;
    vip_uint8_t     *current_data;
} nbg_reader_t;

typedef struct _nbg_parser_data
{
    /* Fixed part of the bin. */
    gcvip_bin_fixed_t              fixed;

    /* Dynamic data part of the bin. */
    gcvip_bin_inout_entry_t        *inputs;
    gcvip_bin_inout_entry_t        *outputs;
    gcvip_bin_layer_t              *orig_layers; /* original layers info, loading from binary graph */
    gcvip_bin_operation_t          *operations;
    gcvip_bin_entry_t              *LCDT;
    gcvip_bin_sh_operation_t       *sh_ops;
    void                           *nn_ops;
    gcvip_bin_tp_operation_t       *tp_ops;
    gcvip_bin_patch_data_entry_t   *pd_entries;
    gcvip_bin_layer_parameter_t    *lp_entries;
    gcvip_bin_hw_init_operation_info_entry_t *hw_init_ops;
    gcvip_bin_entry_t              *ICDT;
    void                           *LCD;

    vip_uint32_t                    n_inputs;
    vip_uint32_t                    n_outputs;
    vip_uint32_t                    n_orig_layers; /* the number of original layers */
    vip_uint32_t                    n_operations;
    vip_uint32_t                    n_LCDT;
    vip_uint32_t                    n_nn_ops;
    vip_uint32_t                    n_tp_ops;
    vip_uint32_t                    n_sh_ops;
    vip_uint32_t                    n_pd_entries;
    vip_uint32_t                    n_lp_entries;

    vip_uint32_t                    n_hw_init_ops;
    vip_uint32_t                    n_ICDT;

    nbg_reader_t                    reader;
} nbg_parser_data_t;


#endif
