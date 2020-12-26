/*
 * PortabilityImpl.cpp
 *
 * Copyright 2002, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2002, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

/*
  This file contains various portability functions
*/

#include "PortabilityImpl.hh"

#ifndef LOG4CPP_HAVE_SSTREAM
#include <IOStream.h>
namespace std {
    std::string ostringstream::str() { 
        (*this) << '\0'; 
        std::string msg(ostream::str()); 
        //20200110
		//ostrstream::freeze(false); //unfreeze stream 
        return msg;         
    } 
}

#endif
