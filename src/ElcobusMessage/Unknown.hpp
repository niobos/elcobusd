#ifndef __ELCOBUSMESSAGE_Unknown_HPP__
#define __ELCOBUSMESSAGE_Unknown_HPP__

#include "ElcobusMessage.hpp"

namespace ElcobusMessage {

class Unknown : public ElcobusMessage {
public:
	std::string m_data;

protected:
	Unknown(unsigned char header, unsigned char from, unsigned char to, std::string const &data) :
		ElcobusMessage(header, from, to), m_data(data) {}

public:
	static Unknown* factory(unsigned char header, unsigned char from, unsigned char to, std::string const &data) {
		return new Unknown(header, from, to, data);
	}

	virtual std::string data() throw() { return m_data; }
	virtual std::string string() throw();
};

} // Namespace

#endif // __ELCOBUSMESSAGE_Unknown_HPP__
