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

#include "FlowThru.h"

using namespace std;
using namespace Marsyas;

FlowThru::FlowThru(string name):MarSystem("FlowThru",name)
{
	isComposite_ = true;
	addControls();
}

FlowThru::FlowThru(const FlowThru& a):MarSystem(a)
{
	ctrl_innerOut_ = getctrl("mrs_realvec/innerOut");
}

FlowThru::~FlowThru()
{
	deleteSlices();
}

MarSystem* 
FlowThru::clone() const 
{
	return new FlowThru(*this);
}

void 
FlowThru::addControls()
{
	addctrl("mrs_realvec/innerOut", realvec(), ctrl_innerOut_);
}

void 
FlowThru::deleteSlices()
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
const FlowThru::recvControls()
{
	if ( marsystemsSize_ != 0 ) {
		if (marsystems_[0]->getType() == "NetworkTCPSource" ) {
			return marsystems_[0]->recvControls();
		}
	}
	return 0;
}

void 
FlowThru::myUpdate(MarControlPtr sender)
{
	//forward flow propagation
	//just pass input flow to the output
	MarSystem::myUpdate(sender);
	
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
		
		//THIS DOES NOT WORK DUE TO BREAKS IN ENCAPSULATION!!!! [!]
		//link mrs_realvec/innerOut control to the output control of the last child
		//ctrl_innerOut_->clearLinks();
		//ctrl_innerOut_->linkTo(marsystems_[marsystemsSize_-1]->getctrl("mrs_realvec/processedData"));

		// update buffers (aka slices) between components 
		if ((mrs_natural)slices_.size() < marsystemsSize_) 
			slices_.resize(marsystemsSize_, NULL);

		for (mrs_natural i=0; i< marsystemsSize_; i++)
		{
			if (slices_[i] != NULL) 
			{
				if ((slices_[i])->getRows() != marsystems_[i]->ctrl_onObservations_->to<mrs_natural>()  ||
					(slices_[i])->getCols() != marsystems_[i]->ctrl_onSamples_->to<mrs_natural>())
				{
					delete slices_[i];
					slices_[i] = new realvec(marsystems_[i]->ctrl_onObservations_->to<mrs_natural>(), 
						marsystems_[i]->ctrl_onSamples_->to<mrs_natural>());

					(marsystems_[i])->ctrl_processedData_->setValue(*(slices_[i]));// [WTF] ?!?!?!?!?!?!?!?!?!?!??!!?!?!?!? [?]

					slPtrs_.push_back(marsystems_[i]->ctrl_processedData_);
					
					(slices_[i])->setval(0.0);// [WTF] ?!?!?!?!?!?!?!?!?!?!??!!?!?!?!? [?]
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

			if(i==marsystemsSize_-1)
			{
				MarControlAccessor acc(ctrl_innerOut_, NOUPDATE);
				realvec& innerOut = acc.to<mrs_realvec>();
				innerOut.create(marsystems_[i]->ctrl_onObservations_->to<mrs_natural>(),marsystems_[i]->ctrl_onSamples_->to<mrs_natural>());
			}
		}
	}
}

void
FlowThru::myProcess(realvec& in, realvec& out)
{
	// Add assertions about sizes [!]

	//input should be passed thru the output untouched!
	out = in;

  if(marsystemsSize_ >= 1)
	{
		for (mrs_natural i = 0; i < marsystemsSize_; i++)
		{
			if (i==0)
			{
				marsystems_[i]->process(in, (realvec &) slPtrs_[i]->to<mrs_realvec>());
			}
			else if (i == marsystemsSize_-1)//!!!!!!!!!!!!!!!!!!!!!!!! [!]
			{
				mrs_realvec outVec;
				outVec.allocate(marsystems_[i]->ctrl_onObservations_->to<mrs_natural>(),
												marsystems_[i]->ctrl_onSamples_->to<mrs_natural>());
				marsystems_[i]->process((realvec &) slPtrs_[i-1]->to<mrs_realvec>(),outVec); 
				ctrl_innerOut_->setValue(outVec); //COPYING INVOLVED! ONLY WAY TO GET AROUND BREAK IN INCAPSULATION (so we can use links to this control)
			}
			else
				marsystems_[i]->process((realvec &) slPtrs_[i-1]->to<mrs_realvec>(), 
																(realvec &) slPtrs_[i]->to<mrs_realvec>());
		}
	}
	else if(marsystemsSize_ == 0) //composite has no children!
	{
		MRSWARN("FlowThru::process: composite has no children MarSystems - passing input to output without changes.");
	}
}



