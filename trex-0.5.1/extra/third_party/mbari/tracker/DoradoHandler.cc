/*********************************************************************
 * Software License Agreement (BSD License)
 * 
 *  Copyright (c) 2011, MBARI.
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 * 
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the TREX Project nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "DoradoHandler.hh"
#include "sensor.pb.h"

#include <trex/utils/chrono_helper.hh>

#include <trex/domain/IntegerDomain.hh>
#include <trex/domain/FloatDomain.hh>
#include <trex/domain/BooleanDomain.hh>
#include <trex/domain/EnumDomain.hh>

#include "DrifterTracker.hh"
#include "location.hh"

# define DATA_TL "_trex_data"
# define MBFD_TL "_mbfd"
# define TREX_TL "drifterFollow"

using namespace mbari;
using namespace TREX::transaction;

namespace {
  MessageHandler::factory::declare<DoradoHandler> decl("MBFD");
}

DoradoHandler::DoradoHandler(MessageHandler::xml_arg const &arg) 
  :MessageHandler(arg, ""), m_updated(false),
  m_last_obs(TREX_TL, "undefined"), m_obs_fresh(false) {
  provide(DATA_TL);
  notify(TREX::transaction::Observation(DATA_TL, "undefined"));
  provide(MBFD_TL, true);
  notify(TREX::transaction::Observation(MBFD_TL, "Inactive"));
  provide(TREX_TL);
  notify(m_last_obs);
    

  m_filters.insert(std::make_pair("temperature", kalman_filter_fn(1.0, 1.0, .00024, .05)));
  m_filters.insert(std::make_pair("salinity", kalman_filter_fn(1.0, 1.0, .00012, .05)));
}

bool DoradoHandler::handleRequest(TREX::transaction::goal_id const &g) {
  // I need to process the goal or at least initiate some processing 
  return g->object()==MBFD_TL; 
}


bool DoradoHandler::handleMessage(amqp::queue::message &msg) {
  org::mbari::trex::SensorMessage data;
  static size_t count = 0;
  
  if( data.ParseFromArray(msg.body(), msg.size()) ) {
    int samples = data.sample_size();
    {
      std::ostringstream oss;
      oss<<"dorado."<<(count++)<<".sbd";
      boost::filesystem::path p = owner().get_file(oss.str());
      
      std::fstream of(p.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
      data.SerializeToOstream(&of);
      of.close();
      syslog()<<"New trex proto message logged to "<<oss.str()<<"\n\t"<<p.string();
    }      
    
    for(int i=0; i<samples; ++i) {
      org::mbari::trex::SensorMessage_Sample const &samp = data.sample(i);
      boost::posix_time::ptime 
        date = boost::posix_time::from_time_t(samp.utime());
      
      point<3> pos;
      sensor_data values;
      
      pos[0] = samp.northing();
      pos[1] = samp.easting();
      pos[2] = samp.depth();
      
      values.insert(sensor_data::value_type("northing", pos[0]));
      values.insert(sensor_data::value_type("easting", pos[1]));
      
      if( samp.has_temperature() ) 
	values.insert(sensor_data::value_type("temperature", samp.temperature()));
      if( samp.has_salinity() ) 
	values.insert(sensor_data::value_type("salinity", samp.salinity()));
      if( samp.has_nitrate() ) 
	values.insert(sensor_data::value_type("nitrate", samp.nitrate()));
      if( samp.has_chfl() ) 
	values.insert(sensor_data::value_type("ch_fl", samp.chfl()));
      m_serie.add(date, pos, values);
      m_updated = true;
    }
    if( data.has_gps_fix() ) {
      org::mbari::trex::SensorMessage_GpsError 
        const &fix = data.gps_fix();
      point<2> error_rate;
      error_rate[0] = fix.northing_error_rate();
      error_rate[1] = fix.easting_error_rate();
      m_serie.align(boost::posix_time::from_time_t(fix.from_time()), 
                    boost::posix_time::from_time_t(fix.to_time()), 
                    error_rate);
      gps_fix tmp;
      tmp.from = fix.from_time();
      tmp.to = fix.to_time();
      tmp.north = fix.northing_error_rate();
      tmp.east = fix.easting_error_rate();
      m_fix.push_back(tmp);

      std::ofstream err(owner().get_file("gps_fix.csv").c_str());
      err<<"from_time, to_time, err_north(m/s), err_east(m/s)"<<std::endl;
      err.precision(2);
      for(std::list<gps_fix>::const_iterator j = m_fix.begin(); m_fix.end()!=j; ++j) {
        err<<j->from<<", "<<j->to<<", "<<j->north<<", "<<j->east<<std::endl;
      }
      err.close();
      syslog()<<"Updated gps_fix.csv with new fix update";
    }
    
    // For now I will produce the file globally asuming that this a whole single mission
    std::ofstream csv(owner().get_file("sensor.csv").c_str());
    csv<<"utime, corrected_lat, corrected_lon, corrected_northing, corrected_easting, depth, "
    ", travel_dist, northing, easting, temperature, salinity, nitrate, chfl"<<std::endl;
    
    double travel = 0.0;
    earth_point pred;
    rhumb_lines dist_c;
    
    // Generate the csv file
    for(serie<sensor_data>::iterator i=m_serie.begin(); m_serie.end()!=i; ++i) {
      csv<<i->first<<", ";
      earth_point loc(i->second.first[0], i->second.first[1], 10, 'S'); // hard coded zone for mbari
      if( i!=m_serie.begin() ) {
        travel += pred.distance(loc, dist_c);
      }
      pred = loc;
      
      csv.precision(6);
      csv<<loc.latitude()<<", "<<loc.longitude()<<", ";
      csv.precision(2);
      csv<<i->second.first[0]<<", "<<i->second.first[1]<<", "<<i->second.first[2]<<", "<<travel;
      sensor_data::const_iterator j = i->second.second.find("northing");
      if( j!=i->second.second.end() )
        csv<<j->second<<", ";
      else
        csv<<"NaN, ";
      j = i->second.second.find("easting");
      if( j!=i->second.second.end() )
        csv<<j->second<<", ";
      else
        csv<<"NaN, ";
      csv.precision(6);

      j = i->second.second.find("temperature");
      if( j!=i->second.second.end() )
        csv<<j->second<<", ";
      else
        csv<<"NaN, ";
      j = i->second.second.find("salinity");
      if( j!=i->second.second.end() ) {
        csv.precision(4);
        csv<<j->second<<", ";
      } else
        csv<<"NaN, ";
      csv.precision(6);
      j = i->second.second.find("nitrate");
      if( j!=i->second.second.end() ) {
        csv<<j->second;
      } else
        csv<<"NaN";
      j = i->second.second.find("ch_fl");
      if( j!=i->second.second.end() ) {
        csv<<j->second<<", ";
      } else
        csv<<"NaN, ";
      csv<<std::endl;
    }
    csv.close();
    syslog()<<"Wrote "<<m_serie.size()<<" samples other "<<travel<<" meters travelled";
    
    for(size_t i=0; i<data.observations_size(); ++i) {      
      org::mbari::trex::Predicate const &pred(data.observations(i));
      if( TREX_TL==pred.object() ) {
        date_type date = boost::posix_time::from_time_t(pred.start());
        
        if( m_obs_fresh && m_since>date )
          continue; // ignore old observations
	syslog(info)<<"Creating new observation "<<pred.object()<<"."<<pred.predicate();
        m_last_obs = Observation(pred.object(), pred.predicate());
        m_obs_fresh = true;
        m_since = date;
        // Add token attributes :
        for(int v=0; v<pred.attributes_size(); ++v) {
          org::mbari::trex::Predicate_Variable const &var = pred.attributes(v);
          std::string name = var.name();
          if( var.has_int_d() ) {
            org::mbari::trex::Predicate_FloatDomain const &dom = var.int_d();
            FloatDomain::bound lo(FloatDomain::minus_inf), 
                                hi(FloatDomain::plus_inf);
            
            if( dom.has_min_v() )
              lo = dom.min_v();
            if( dom.has_max_v() )
              hi = dom.max_v();
	    if( lo>hi ) {
	      syslog(warn)<<" domain ["<<lo<<", "<<hi<<"] is empty ignoing attribute "<<name;
	      continue;
	    }
            m_last_obs.restrictAttribute(name, FloatDomain(lo, hi));
          } else if( var.has_bool_d() ) {
            m_last_obs.restrictAttribute(name, BooleanDomain(var.bool_d()));
          } else {
            size_t enums = var.enum_d_size();
            if( enums>0 ) {
              EnumDomain values;
              for(size_t e=0; e<enums; ++e)
                values.add(var.enum_d(e));
              m_last_obs.restrictAttribute(name, values);
            }
          }
        }
      }
    }
    
    return true;
  }
  return false;
}

bool DoradoHandler::synchronize() {
  double delta_t;
  typedef TREX::utils::chrono_posix_convert<duration_type> cvt;
  
  if( m_updated ) {
    TREX::transaction::Observation obs(DATA_TL, "Received");
    
    duration_type 
       delta = cvt::to_chrono(tickToTime(now())-m_serie.newest());
    delta_t= delta.count();
    delta_t /= tickDuration().count();
    obs.restrictAttribute("freshness", 
                         IntegerDomain(std::floor(delta_t+0.5)));    
    obs.restrictAttribute("nsamples", IntegerDomain(m_serie.size()));

    TREX::utils::display(syslog(info)<<"New trex samples freshness is ", delta)
      <<"\n\total samples "<<m_serie.size();
    notify(obs);
    m_updated = false;
  }
    
  if( m_obs_fresh ) {
    // duration_type delta = cvt::to_chrono(tickToTime(now())-m_since);
    // delta_t = delta.count();
    // delta_t /= tickDuration().count();
    // // indicate how aold this observation is in term of ticks
    // m_last_obs.restrictAttribute("freshness", IntegerDomain(std::floor(delta_t+0.5)));
    notify(m_last_obs);
    m_obs_fresh = false;
  }
  return true;
}
