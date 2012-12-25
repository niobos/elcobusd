#include "Registrar.hpp"

namespace ElcobusMessage {

registrar_key::operator std::string() const throw() {
	std::string ret("TODO"); // TODO
	return ret;
}

bool registrar_key::operator < (struct registrar_key b) const throw() {
	if( tbd1 < b.tbd1 ) return true;
	if( tbd1 > b.tbd1 ) return false;

	if( tbd2 < b.tbd2 ) return true;
	return false;
}

} /* namespace */
