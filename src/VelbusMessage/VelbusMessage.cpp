#include "VelbusMessage.hpp"
#include "Registrar.hpp"
#include "../utils/output.hpp"

#include <sstream>

#include <stdio.h>

namespace VelbusMessage {

std::auto_ptr<VelbusMessage> parse_and_consume(std::string &msg)
		throw(InsufficientData, FormError) {

	if( msg.length() < 6 ) throw InsufficientData();
	int sum = 0;

	sum += msg[0];
	if( msg[0] != 0x0f ) throw FormError("data[0] != 0x0f");

	sum += msg[1];
	if( (msg[1] & 0xfc) != 0xf8 ) throw FormError("data[1] & 0xfc != 0xf8");
	unsigned char prio = msg[1] & 0x03;

	sum += msg[2];
	unsigned char addr = msg[2];

	sum += msg[3];
	if( (msg[3] & 0xb0) != 0x00 ) throw FormError("data[3] & 0xb0 != 0x00");
	unsigned char rtr = (msg[3]>>6)&0x01;
	size_t length = msg[3] & 0x0f;

	if( msg.length() < 6+length ) throw InsufficientData();

	for(int i=4; i<4+length; i++) sum += msg[i];
	std::string data = msg.substr(4, length);

	sum = (-sum)&0xff;
	if( (msg[4+length]&0xff) != sum ) throw FormError("Checksum incorrect");

	if( msg[4+length+1] != 0x04 ) throw FormError("data[-1] != 0x04");

	msg = msg.substr(4+length+2); // Consume

	// Now identify the type of message
	if( data.length() == 0 ) {
		return std::auto_ptr<VelbusMessage>( Unknown::factory(prio, addr, rtr, data) );
	}

	// length > 0
	try {
		struct factory_methods f = Registrar::get_instance().get( data[0] );
		return std::auto_ptr<VelbusMessage>( f.factory(prio, addr, rtr, data) );

	} catch( NotFound &e ) {
		return std::auto_ptr<VelbusMessage>( Unknown::factory(prio, addr, rtr, data) );

	} catch( FormError &e ) {
		return std::auto_ptr<VelbusMessage>( Unknown::factory(prio, addr, rtr, data) );
	}
}

std::string VelbusMessage::message() throw() {
	std::string data = this->data();

	char header[4];
	header[0] = 0x0f;
	header[1] = 0xf8 | (m_prio & 0x03);
	header[2] = m_addr;
	header[3] = 0x00 | ((m_rtr & 0x01)<<6) | (data.length() & 0x0f);
	std::string out(header, 4);

	out.append(data, 0, data.length() & 0x0f); // Strip length of data (if needed)

	int sum = 0;
	for( int i = out.length()-1; i>=0; --i ) {
		sum += out[i];
	}

	sum = (-sum)&0xff;

	out.append(1, sum);
	out.append(1, 0x04);

	return out;
}

std::string Unknown::string() throw() {
	std::ostringstream o;
	o << "Unknown message:"
	  << " Prio=" << static_cast<unsigned int>(m_prio)
	  << " Addr=0x" << std::hex << static_cast<int>(m_addr) << std::dec
	  << " RTR=" << static_cast<unsigned int>(m_rtr)
	  << " data[" << m_data.length() << "]=";
	for( int i = 0; i < m_data.length(); i++ ) {
		o << hex(m_data[i]) << " "; // Ugly hex-dump hack
	}
	return o.str();
}

std::auto_ptr<VelbusMessage> Deframer::get_message() throw() {
	std::auto_ptr<VelbusMessage> ret;
	while(1) {
		try {
			ret = parse_and_consume( m_buffer );
			return ret;
		} catch( InsufficientData &e ) {
			// try again later
			return ret; // NULL pointer
		} catch( FormError &e ) {
			// Strip of first byte and try again
			m_buffer = m_buffer.substr(1);
		}
	}
}

} // namespace