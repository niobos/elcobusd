#include <config.h>
#include "ElcobusMessage.hpp"
#include "Registrar.hpp"
#include "../utils/output.hpp"
#include <sstream>
#include <stdio.h>
#include "Unknown.hpp"

namespace ElcobusMessage {

ElcobusMessage* parse_and_consume(std::string &msges, std::string *msg)
		throw(InsufficientData, FormError) {

	if( msges.length() < 4 ) throw InsufficientData();

	size_t length = msges[3];
	if( msges.length() < length ) throw InsufficientData();

	std::string data = msges.substr(0, length); // Copy

	if( msg != NULL ) *msg = msges.substr(0, length); // Copy
	msges = msges.substr(length); // Consume


	// Now identify the type of message
	try {
		struct registrar_key k;
		k.tbd1 = 0;
		k.tbd2 = 0;
		struct factory_methods f = Registrar::get_instance().get( k );
		return f.factory(data);

	} catch( NotFound &e ) {
		return Unknown::factory(data);

	} catch( FormError &e ) {
		return Unknown::factory(data);
	}
}

std::string ElcobusMessage::message() throw() {
	std::string out = this->data();
	return out;
}

} // namespace
