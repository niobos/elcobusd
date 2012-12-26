#include <config.h>
#include "Unknown.hpp"
#include "../utils/output.hpp"
#include <sstream>

namespace ElcobusMessage {

std::string Unknown::string() throw() {
	std::ostringstream o;
	o << "Unknown message:"
	  << " Hdr=0x" << hex(m_header)
	  << " Src=0x" << hex(m_from)
	  << " Dst=0x" << hex(m_to)
	  << " data[" << m_data.length() << "]=";
	for( unsigned int i = 0; i < m_data.length(); i++ ) {
		o << hex(m_data[i]) << " "; // Ugly hex-dump hack
	}
	return o.str();
}

} // namespace
