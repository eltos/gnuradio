/* -*- c++ -*- */
/*
 * Copyright 2009,2011,2012,2014 Free Software Foundation, Inc.
 * Copyright 2025 Daniel Estevez <daniel@destevez.net>
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef INCLUDED_DIGITAL_FLL_BAND_EDGE_CC_IMPL_H
#define INCLUDED_DIGITAL_FLL_BAND_EDGE_CC_IMPL_H

#include <gnuradio/blocks/control_loop.h>
#include <gnuradio/digital/fll_band_edge_cc.h>
#include <gnuradio/filter/fir_filter_with_buffer.h>

namespace gr {
namespace digital {

class fll_band_edge_cc_impl : public fll_band_edge_cc
{
private:
    float d_sps;
    float d_rolloff;
    size_t d_filter_size;
    float d_bandwidth;

    std::vector<gr_complex> d_taps_lower;
    std::vector<gr_complex> d_taps_upper;
    std::unique_ptr<gr::filter::kernel::fir_filter_with_buffer_ccc> d_filter_lower;
    std::unique_ptr<gr::filter::kernel::fir_filter_with_buffer_ccc> d_filter_upper;

    /*!
     * Design the band-edge filter based on the number of samples
     * per symbol, filter rolloff factor, and the filter size
     */
    void design_filter();

public:
    fll_band_edge_cc_impl(float samps_per_sym,
                          float rolloff,
                          int filter_size,
                          float bandwidth);

    void set_samples_per_symbol(float sps) override;
    void set_rolloff(float rolloff) override;
    void set_filter_size(int filter_size) override;

    float samples_per_symbol() const override;
    float rolloff() const override;
    int filter_size() const override;

    void print_taps() override;

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items) override;
};

} /* namespace digital */
} /* namespace gr */

#endif /* INCLUDED_DIGITAL_FLL_BAND_EDGE_CC_IMPL_H */
