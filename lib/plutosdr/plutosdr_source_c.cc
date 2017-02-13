/* -*- c++ -*- */
/*
 * Copyright 2012 Dimitri Stolnikov <horiz0n@gmx.net>
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include "arg_helpers.h"
#include "osmosdr/source.h"
#include "plutosdr_source_c.h"

using namespace boost::assign;

plutosdr_source_c_sptr make_plutosdr_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new plutosdr_source_c(args));
}

plutosdr_source_c::plutosdr_source_c(const std::string &args) :
    gr::hier_block2("plutosdr_source_c",
                   gr::io_signature::make(0, 0, 0),
                   gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
  uri = "ip:192.168.2.1";
  //uri = "usb:6.29.5";
  frequency = 434000000;
  samplerate = 5000000;
  decimation = 1;
  bandwidth = 4000000;
  ch1_en = ch2_en = true;
  ch3_en = ch4_en = false;
  buffer_size = 0x4000;
  quadrature = true;
  rfdc = bbdc = true;
  gain1_mode = "Fast";
  gain1_value = 64;
  gain2_mode = "Fast";
  gain2_value = 64;
  rf_port_select = "A_BALANCED";
  filter = "";

  _src = gr::iio::fmcomms2_source::make(uri.c_str(), frequency, samplerate,
                                        decimation, bandwidth,
                                        ch1_en, ch2_en, ch3_en, ch4_en,
                                        buffer_size, quadrature, rfdc, bbdc,
                                        gain1_mode.c_str(), gain1_value,
                                        gain2_mode.c_str(), gain2_value,
		                                rf_port_select.c_str(), filter.c_str());

  _stofi = gr::blocks::short_to_float::make(1, 32767.f);
  _stofq = gr::blocks::short_to_float::make(1, 32767.f);
  _ftc = gr::blocks::float_to_complex::make();

  connect( _src, 0, _stofi, 0);
  connect( _src, 1, _stofq, 0);
  connect( _stofi, 0, _ftc, 0);
  connect( _stofq, 0, _ftc, 1);
  connect( _ftc, 0, self(), 0 );
}

plutosdr_source_c::~plutosdr_source_c()
{
}

std::vector< std::string > plutosdr_source_c::get_devices()
{
  std::vector< std::string > devices;

  std::string args = "plutosdr,label='PlutoSDR'";

  devices.push_back( args );

  return devices;
}

std::string plutosdr_source_c::name()
{
  return "PlutoSDR";
}

size_t plutosdr_source_c::get_num_channels()
{
  return output_signature()->max_streams();
}

osmosdr::meta_range_t plutosdr_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t rates;

  rates += osmosdr::range_t( 2500000 );
  rates += osmosdr::range_t( 5000000 );
  rates += osmosdr::range_t( 10000000 );
  rates += osmosdr::range_t( 20000000 );

  return rates;
}

double plutosdr_source_c::set_sample_rate( double rate )
{
  samplerate = (unsigned long) rate;
  set_params();

  return samplerate;
}

double plutosdr_source_c::get_sample_rate( void )
{
  return samplerate;
}

osmosdr::freq_range_t plutosdr_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  range += osmosdr::range_t( 325.0e6, 3800.0e6, 1.0 );

  return range;
}

double plutosdr_source_c::set_center_freq( double freq, size_t chan )
{
  frequency = (unsigned long long) freq;
  set_params();

  return freq;
}

double plutosdr_source_c::get_center_freq( size_t chan )
{
  return frequency;
}

double plutosdr_source_c::set_freq_corr( double ppm, size_t chan)
{
  _freq_corr = ppm;
  return ppm;
}

double plutosdr_source_c::get_freq_corr( size_t chan)
{
  return _freq_corr;
}

std::vector<std::string> plutosdr_source_c::get_gain_names( size_t chan )
{
  std::vector< std::string > gains;

  gains.push_back( "RX1" );
  gains.push_back( "RX2" );

  return gains;
}

osmosdr::gain_range_t plutosdr_source_c::get_gain_range( size_t chan)
{
  osmosdr::gain_range_t range;

  range += osmosdr::range_t( 0, 100, 1 );     // FIXME: I have no idea...

  return range;
}

osmosdr::gain_range_t plutosdr_source_c::get_gain_range( const std::string & name,
                                                         size_t chan)
{
  osmosdr::gain_range_t range;

  range += osmosdr::range_t( 0, 100, 1 );     // FIXME: I have no idea...

  return range;
}


double plutosdr_source_c::set_gain( double gain, size_t chan )
{
  gain1_value = gain;
  gain2_value = gain;
  set_params();

  return gain;
}

double plutosdr_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  if ( name == "RX1" )
    gain1_value = gain;
  else if ( name == "RX2" )
    gain2_value = gain;

  set_params();
  return gain;
}

double plutosdr_source_c::get_gain( size_t chan )
{
  return gain1_value;
}

double plutosdr_source_c::get_gain( const std::string & name, size_t chan )
{
  if ( name == "RX1" )
    return gain1_value;
  else if ( name == "RX2" )
    return gain2_value;

  return 0.0;
}

std::vector< std::string > plutosdr_source_c::get_antennas( size_t chan )
{
  // FIXME: Other IIO source can have many?
  std::vector< std::string > antennas;

  antennas += get_antenna( chan );

  return antennas;
}

std::string plutosdr_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna( chan );
}

std::string plutosdr_source_c::get_antenna( size_t chan )
{
  return "A_BALANCED";
}


void plutosdr_source_c::set_params( void )
{
  _src->set_params( frequency, samplerate, bandwidth, quadrature, rfdc, bbdc,
                    gain1_mode.c_str(), gain1_value,
                    gain2_mode.c_str(), gain2_value,
                    rf_port_select.c_str() );
}
