#ifndef __ELCOBUSMESSAGE_HPP__
#define __ELCOBUSMESSAGE_HPP__

#include <string>
#include <stdexcept>
#include <memory>

namespace ElcobusMessage {

class InsufficientData : public std::runtime_error {
public:
	InsufficientData() throw() :
		std::runtime_error("Insufficient data") {}
	~InsufficientData() throw() {}
};

class FormError : public std::runtime_error {
private:
	std::string m_details;
public:
	FormError(std::string const &msg = "") throw() :
		std::runtime_error("Form error in message"),
		m_details(msg) {}
	~FormError() throw() {}

	std::string details() throw() { return m_details; }
};

class ElcobusMessage {
public:
	unsigned char m_header;
	unsigned char m_from;
	unsigned char m_to;

protected:
	ElcobusMessage(unsigned char header, unsigned char from, unsigned char to) throw() :
		m_header(header), m_from(from), m_to(to) {};

public:
	virtual ~ElcobusMessage() {}

	virtual std::string data() throw() =0;

	std::string message() throw();
	virtual std::string string() throw() =0;
};

ElcobusMessage* parse_and_consume(std::string &msges, std::string *msg = NULL) throw(InsufficientData, FormError);
/* Try to extract a message from msges
 * And return the corresponding ElcobusMessage object or throw an eror
 * When a message is extracted, it is removed from the msges variable
 * and placed in *msg (unless msg is NULL)
 */

} // Namespace

#endif // __ELCOBUSMESSAGE_HPP__
