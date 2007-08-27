/*
** Copyright (C) 1998-2006 George Tzanetakis <gtzan@cs.uvic.ca>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include "Kurtosis.h"

using namespace std;
using namespace Marsyas;


Kurtosis::Kurtosis(string name):MarSystem("Kurtosis",name)
{
  z_ = 0.0;
  b_ = 0.0;
  q_ = 0.0;
}


Kurtosis::~Kurtosis()
{
}


MarSystem* 
Kurtosis::clone() const 
{
  return new Kurtosis(*this);
}

void
Kurtosis::myUpdate(MarControlPtr sender)
{
  MRSDIAG("Kurtosis.cpp - Kurtosis:myUpdate");
  
  setctrl("mrs_natural/onSamples", getctrl("mrs_natural/inSamples"));
  setctrl("mrs_natural/onObservations", (mrs_natural)1);
  setctrl("mrs_real/osrate", getctrl("mrs_real/israte"));
  setctrl("mrs_string/onObsNames", "Kurtosis,");

  //defaultUpdate(); [!]
	inObservations_ = getctrl("mrs_natural/inObservations")->to<mrs_natural>();

	obsrow_.create(inObservations_);
}


void 
Kurtosis::myProcess(realvec& in, realvec& out)
{
  //checkFlow(in,out);

  for (t = 0; t < inSamples_; t++)
    {
      
      for (o=0; o < inObservations_; o++)
		{
		  obsrow_(o) = in(o,t);
		}
      z_ = 0.0;
      mrs_real mean = obsrow_.mean();
      for (o=0; o < inObservations_; o++)
		{
		  b_ =  obsrow_(o) - mean ;
		  
		  // take x - u to the fourth power into the sum
		  z_ += (b_ * b_ * b_ * b_); 
		}
      
      // standard deviation to the fourth power
      q_ = obsrow_.var() * obsrow_.var();


      if ((z_ < 1.0e-45)||(q_ < 1.0e-45))
		out(0,t) = 0.5;
      else 
		{
		  // out(0,t) = (mrs_real)((z_ /  q_ * inObservations_) - 3.0);
		  out(0,t) = (mrs_real)((z_ /  q_ ) - 3.0);
		}
    }   
}

