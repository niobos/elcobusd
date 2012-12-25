#ifndef __ELCOBUSMESSAGE_Unknown_HPP__
#define __ELCOBUSMESSAGE_Unknown_HPP__

#include "ElcobusMessage.hpp"

namespace ElcobusMessage {

class Unknown : public ElcobusMessage {
public:
	std::string m_data;

protected:
	Unknown(std::string const &data) :
		ElcobusMessage(), m_data(data) {}

public:
	static Unknown* factory(std::string const &data) {
		return new Unknown(data);
	}

	virtual std::string data() throw() { return m_data; }
	virtual std::string string() throw();
};

} // Namespace

#endif // __ELCOBUSMESSAGE_Unknown_HPP__
