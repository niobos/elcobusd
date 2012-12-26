#include <config.h>
#include "ElcobusMessage.hpp"
#include "Registrar.hpp"
#include "../utils/output.hpp"
#include <sstream>
#include <stdio.h>
#include "Unknown.hpp"
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

namespace ElcobusMessage {

boost::crc_optimal<16, 0x1021, 0x0000, 0x0000, false, false> elco_crc;

ElcobusMessage* parse_and_consume(std::string &msges, std::string *msg)
		throw(InsufficientData, FormError) {

	if( msges.length() < 4 ) throw InsufficientData();

	unsigned char header = msges[0];
	if( (header & 0xfc) != 0xdc ) throw FormError("Header not 0xdc");

	if( (msges[1] & 0x80) != 0x80 ) throw FormError("From high bit not set");
	if( (msges[2] & 0x80) != 0x00 ) throw FormError("To high bit not clear");
	unsigned char from = msges[1] & 0x7f;
	unsigned char to = msges[2] & 0x7f;

	size_t length = msges[3];
	if( msges.length() < length ) throw InsufficientData();

	boost::uint16_t crc_msg = (static_cast<uint16_t>(static_cast<unsigned char>(msges[length-2])) << 8)
	                        | static_cast<uint16_t>(static_cast<unsigned char>(msges[length-1]));

	elco_crc.reset();
	elco_crc.process_bytes(msges.data(), length-2); // Calculate correct CRC as well
	boost::uint16_t crc_calc = elco_crc.checksum();
	if( crc_msg != crc_calc ) {
		std::ostringstream e;
		e << "CRC incorrect for length " << length
		  <<" (got 0x" << hex(crc_msg)
		  << " expected 0x" << hex(crc_calc)
		  << ")";
		throw FormError(e.str());
	}

	std::string data = msges.substr(4, length-2-4); // Copy, no header & no CRC

	if( msg != NULL ) *msg = msges.substr(0, length); // Copy
	msges = msges.substr(length); // Consume


	// Now identify the type of message
	try {
		struct registrar_key k;
		k.tbd1 = 0;
		k.tbd2 = 0;
		struct factory_methods f = Registrar::get_instance().get( k );
		return f.factory(header, from, to, data);

	} catch( NotFound &e ) {
		return Unknown::factory(header, from, to, data);

	} catch( FormError &e ) {
		return Unknown::factory(header, from, to, data);
	}
}

std::string ElcobusMessage::message() throw() {
	std::string out;

	out.append(1, m_header);
	out.append(1, 0x80 | m_from);
	out.append(1, m_to & 0x7f);

	out.append( this->data() );

	elco_crc.reset();
	elco_crc.process_bytes(out.data(), out.length());
	boost::uint16_t crc = elco_crc.checksum();

	out.append(1, (crc >> 8) & 0xff);
	out.append(1, (crc     ) & 0xff);

	return out;
}

} // namespace
