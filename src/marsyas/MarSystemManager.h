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

#ifndef MARSYAS_MARSYSTEMMANAGER_H
#define MARSYAS_MARSYSTEMMANAGER_H

#include <string>
#include <map>
#include <vector>

#include "MarSystem.h"

namespace Marsyas
{
class MarSystem;

/** 
\class MarSystemManager
\brief MarSystemManager of MarSystem objects

Combines a series of MarSystem objects to a single MarSystem 
corresponding to executing the System objects one after the other 
in sequence. 
*/

class MarSystemManager
{
protected:
  std::map<std::string, MarSystem*> registry_;
  std::map<std::string, MarSystem*> workingSet_; 
  
public:
  MarSystemManager();
  ~MarSystemManager();
  void registerPrototype(std::string type, MarSystem *);
  MarSystem* getPrototype(std::string type);
  MarSystem* create(std::string type, std::string name);
  MarSystem* getMarSystem(std::istream& is, MarSystem *parent=NULL);

  std::map<std::string, MarSystem*> getWorkingSet(std::istream& is);

  bool isRegistered (std::string name);

  std::vector <std::string> registeredPrototypes(); 
};

}//namespace Marsyas

#endif

	

	
	
