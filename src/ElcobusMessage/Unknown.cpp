#include <config.h>
#include "Unknown.hpp"
#include "../utils/output.hpp"
#include <sstream>

namespace ElcobusMessage {

std::string Unknown::string() throw() {
	std::ostringstream o;
	o << "Unknown message:"
	  << " data[" << m_data.length() << "]=";
	for( unsigned int i = 0; i < m_data.length(); i++ ) {
		o << hex(m_data[i]) << " "; // Ugly hex-dump hack
	}
	return o.str();
}

} // namespace
