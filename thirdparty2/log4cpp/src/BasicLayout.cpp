/*
 * BasicLayout.cpp
 *
 * Copyright 2000, LifeLine Networks BV (www.lifeline.nl). All rights reserved.
 * Copyright 2000, Bastiaan Bakker. All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#include "PortabilityImpl.hh"
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/FactoryParams.hh>
#include <memory>

#ifdef LOG4CPP_HAVE_SSTREAM
#include <sstream>
#endif

namespace log4cpp {

    BasicLayout::BasicLayout() {
    }
    
    BasicLayout::~BasicLayout() {
    }

    std::string BasicLayout::format(const LoggingEvent& event) {
        std::ostringstream message;

        const std::string& priorityName = Priority::getPriorityName(event.priority);
        message << event.timeStamp.getSeconds() << " " << priorityName.c_str() << " " 
                << event.categoryName.c_str() << " " << event.ndc.c_str() << ": " 
                << event.message.c_str() << std::endl;

        return message.str();
    }

    std::auto_ptr<Layout> create_basic_layout(const FactoryParams& params)
    {
       return std::auto_ptr<Layout>(new BasicLayout);
    }
}
