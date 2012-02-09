/***************************************************************************
 *   Copyright (C) 2010-2012 by Max Thrun                                  *
 *   bear24rw@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef FFT_H
#define FFT_H

extern double *fft_input;
extern fftw_complex *fft_out;
extern fftw_plan fft_plan;

void init_fft(void);
void do_fft(void);
void copy_bins_to_old(void);
void compute_magnitude(void);
void compute_delta_from_last(void);
void add_bins_to_history(void);
void compute_bin_hist(void);
void compute_std_dev(void);

#endif
