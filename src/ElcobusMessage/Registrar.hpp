#ifndef ELCOBUSMESSAGE_REGISTRAR_H__
#define ElCOBUSMESSAGE_REGISTRAR_H__

#include "../utils/Registrar.t"
#include "ElcobusMessage.hpp"

namespace ElcobusMessage {
	struct registrar_key {
		unsigned char tbd1;
		unsigned char tbd2;

		operator std::string() const throw();
		bool operator < (struct registrar_key b) const throw();
	};

	struct factory_methods {
		ElcobusMessage* (*factory)(unsigned char, unsigned char,
				unsigned char, std::string const &);
	};

	typedef ::Registrar<
		struct registrar_key,
		struct factory_methods
	  > Registrar;
}

#endif // ELCOBUSMESSAGE_REGISTRAR_H__
