#ifndef __VELBUSMESSAGE_HPP__
#define __VELBUSMESSAGE_HPP__

#include <string>
#include <stdexcept>
#include <memory>

namespace VelbusMessage {

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

class VelbusMessage {
public:
	unsigned char m_prio;
	unsigned char m_addr;
	unsigned char m_rtr;

protected:
	VelbusMessage(unsigned char prio, unsigned char addr, unsigned char rtr) throw() :
		m_prio(prio), m_addr(addr), m_rtr(rtr) {};

public:
	virtual ~VelbusMessage() {}

	virtual std::string data() throw() =0;

	std::string message() throw();
	virtual std::string string() throw() =0;
};

VelbusMessage* parse_and_consume(std::string &msg) throw(InsufficientData, FormError);

class Unknown : public VelbusMessage {
public:
	std::string m_data;

protected:
	Unknown(unsigned char prio, unsigned char addr, unsigned char rtr, std::string const &data) :
		VelbusMessage(prio, addr, rtr), m_data(data) {}

public:
	static Unknown* factory(unsigned char prio, unsigned char addr, unsigned char rtr, std::string const &data) {
		return new Unknown(prio, addr, rtr, data);
	}

	virtual std::string data() throw() { return m_data; }
	virtual std::string string() throw();
};

class Deframer {
protected:
	std::string m_buffer;
public:
	void add_data(std::string const &data) throw() { m_buffer.append(data); }
	std::auto_ptr< VelbusMessage > get_message() throw();
};

} // Namespace

#endif // __VELBUSMESSAGE_HPP__
