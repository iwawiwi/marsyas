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

#include "Series.h"

using namespace std;
using namespace Marsyas;

Series::Series(string name):MarSystem("Series",name)
{
	isComposite_ = true;
	addControls();
}

Series::~Series()
{
	deleteSlices();
}

MarSystem* 
Series::clone() const 
{
	return new Series(*this);
}

void 
Series::addControls()
{
}

void 
Series::deleteSlices()
{
	vector<realvec *>::const_iterator iter;
	for (iter= slices_.begin(); iter != slices_.end(); ++iter)
	{
		delete *(iter);
	}
	slices_.clear();
}

// STU
mrs_real* 
const Series::recvControls()
{
	if ( marsystemsSize_ != 0 ) {
		if (marsystems_[0]->getType() == "NetworkTCPSource" ) {
			return marsystems_[0]->recvControls();
		}
	}
	return 0;
}

void 
Series::myUpdate(MarControlPtr sender)
{
	if (marsystemsSize_ != 0) 
	{
		//propagate in flow controls to first child
		marsystems_[0]->setctrl("mrs_natural/inObservations", inObservations_);
		marsystems_[0]->setctrl("mrs_natural/inSamples", inSamples_);
		marsystems_[0]->setctrl("mrs_real/israte", israte_);
		marsystems_[0]->setctrl("mrs_string/inObsNames", inObsNames_);
		marsystems_[0]->update();

		// update dataflow component MarSystems in order 
		for (mrs_natural i=1; i < marsystemsSize_; i++)
		{
			marsystems_[i]->setctrl(marsystems_[i]->ctrl_inObsNames_, 
				marsystems_[i-1]->ctrl_onObsNames_);
			marsystems_[i]->setctrl(marsystems_[i]->ctrl_inObservations_, 
				marsystems_[i-1]->ctrl_onObservations_);
			marsystems_[i]->setctrl(marsystems_[i]->ctrl_inSamples_, 
				marsystems_[i-1]->ctrl_onSamples_);
			marsystems_[i]->setctrl(marsystems_[i]->ctrl_israte_, 
				marsystems_[i-1]->ctrl_osrate_);
			marsystems_[i]->update();
		}

		//forward flow propagation
		updctrl(ctrl_onObsNames_, marsystems_[marsystemsSize_-1]->ctrl_onObsNames_, NOUPDATE);
		updctrl(ctrl_onSamples_, marsystems_[marsystemsSize_-1]->ctrl_onSamples_, NOUPDATE);
		updctrl(ctrl_onObservations_, marsystems_[marsystemsSize_-1]->ctrl_onObservations_, NOUPDATE);
		updctrl(ctrl_osrate_, marsystems_[marsystemsSize_-1]->ctrl_osrate_, NOUPDATE);

		// update buffers (aka slices) between components 
		if ((mrs_natural)slices_.size() < marsystemsSize_-1) 
			slices_.resize(marsystemsSize_-1, NULL);

		for (mrs_natural i=0; i< marsystemsSize_-1; i++)
		{
			if (slices_[i] != NULL) 
			{
				if ((slices_[i])->getRows() != marsystems_[i]->ctrl_onObservations_->to<mrs_natural>()  ||
					(slices_[i])->getCols() != marsystems_[i]->ctrl_onSamples_->to<mrs_natural>())
				{
					delete slices_[i];
					slices_[i] = new realvec(marsystems_[i]->ctrl_onObservations_->to<mrs_natural>(), 
						marsystems_[i]->ctrl_onSamples_->to<mrs_natural>());

					marsystems_[i]->ctrl_processedData_->setValue(*(slices_[i])); // [WTF] ?!?!?!?!?!?!?!?!?!?!??!!?!?!?!? [?]

					slPtrs_.push_back(marsystems_[i]->ctrl_processedData_);
					
					(slices_[i])->setval(0.0); // [WTF]?!?!?!?!?!?!?!? [?]
				}
			}
			else 
			{
				slices_[i] = new realvec(marsystems_[i]->ctrl_onObservations_->to<mrs_natural>(), 
					marsystems_[i]->ctrl_onSamples_->to<mrs_natural>());

				marsystems_[i]->ctrl_processedData_->setValue(*(slices_[i]));// [WTF] ?!?!?!?!?!?!?!?!?!?!??!!?!?!?!? [?]
				
				slPtrs_.push_back(marsystems_[i]->ctrl_processedData_);

				(slices_[i])->setval(0.0);// [WTF] ?!?!?!?!?!?!?!?!?!?!??!!?!?!?!? [?]
			}
		}
	}
	else //if composite is empty...
		MarSystem::myUpdate(sender);
}

void
Series::myProcess(realvec& in, realvec& out)
{
	// Add assertions about sizes [!]

	if (marsystemsSize_ == 1)
		marsystems_[0]->process(in,out);
	else if(marsystemsSize_ > 1)
	{
		for (mrs_natural i = 0; i < marsystemsSize_; i++)
		{
			if (i==0)
			{
				marsystems_[i]->process(in, (realvec &) slPtrs_[i]->to<mrs_realvec>()); //breaks encapsulation!!!! [!]
			}
			else if (i == marsystemsSize_-1)
			{
				marsystems_[i]->process((realvec &) slPtrs_[i-1]->to<mrs_realvec>(), out); 
			}
			else
				marsystems_[i]->process((realvec &) slPtrs_[i-1]->to<mrs_realvec>(), //breaks encapsulation!!!! [!]
				(realvec &) slPtrs_[i]->to<mrs_realvec>()); //breaks encapsulation!!!! [!]
		}
	}
	else if(marsystemsSize_ == 0) //composite has no children!
	{
		MRSWARN("Series::process: composite has no children MarSystems - passing input to output without changes.");
		out = in;
	}
}



