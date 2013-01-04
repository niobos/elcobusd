#include <config.h>

#include <string>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sysexits.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdio.h>
#include <stdexcept>
#include <boost/ptr_container/ptr_list.hpp>
#include <ev.h>
#include <memory>
#include <typeinfo>
#include <assert.h>
#include <signal.h>

#include "Socket.hpp"
#include "utils/TimestampLog.hpp"
#include "utils/output.hpp"
#include "ElcobusMessage/ElcobusMessage.hpp"

static const size_t READ_SIZE = 4096;
static const int MAX_CONN_BACKLOG = 32;

class IOError : public std::runtime_error {
public:
	IOError( const std::string &what ) :
	    std::runtime_error( what ) {}
};
class WouldBlock : public IOError {
public:
	WouldBlock() :
	    IOError("Would block") {}
};
class EOFreached : public std::runtime_error {
public:
	EOFreached() :
	    std::runtime_error( std::string("EOF reached") ) {}
};

struct connection {
	Socket sock;
	std::string buf;
	std::string id;
	ev_io read_ready;
	ev_idle processing_todo;
};

std::string logfilename;
std::ofstream logfile;
std::auto_ptr<std::ostream> log;
Socket s_listen;
struct connection serial;
boost::ptr_list< struct connection > c_network;

std::string read(int from) throw(IOError, EOFreached) {
	char buf[READ_SIZE];
	int n = read(from, buf, sizeof(buf));
	if( n == -1 ) {
		std::ostringstream e;
		e << "Could not read(): " << errno << " ";
		char error_descr[256];
		strerror_r(errno, error_descr, sizeof(error_descr));
		e << error_descr;
		throw IOError( e.str() );
	} else if( n == 0 ) {
		throw EOFreached();
	}
	return std::string(buf, n);
}

void write(int to, std::string const &what) throw(IOError) {
	int rv = write(to, what.data(), what.length());
	if( rv == -1 ) {
		if( errno == EAGAIN ) {
			throw WouldBlock();
		} // else

		std::ostringstream e;
		e << "Could not write(): " << errno << " ";
		char error_descr[256];
		strerror_r(errno, error_descr, sizeof(error_descr));
		e << error_descr;
		throw IOError( e.str() );
	} else if( rv != what.length() ) {
		throw IOError( "Not enough bytes written" );
	}
}

void kill_connection(EV_P_ ev_io *w) {
	// Remove from event loop
	ev_io_stop(EV_A_ w );

	// Find and erase this connection in the list
	for( typeof(c_network.begin()) i = c_network.begin(); i != c_network.end(); ++i ) {
		if( &(i->read_ready) == w ) {
			c_network.erase(i);
			break; // Stop searching
		}
	}
}


void received_sigint(EV_P_ ev_signal *w, int revents) throw() {
	*log << "Received SIGINT, exiting\n" << std::flush;
	ev_unloop(EV_A_ EVUNLOOP_ALL);
}
void received_sigterm(EV_P_ ev_signal *w, int revents) throw() {
	*log << "Received SIGTERM, exiting\n" << std::flush;
	ev_unloop(EV_A_ EVUNLOOP_ALL);
}

void received_sighup(EV_P_ ev_signal *w, int revents) throw() {
	*log << "Received SIGHUP, closing this logfile\n" << std::flush;
	logfile.close();
	logfile.open(logfilename.c_str(), std::ios_base::app | std::ios_base::out );
	*log << "Received SIGHUP, (re)opening this logfile\n" << std::flush;
}

void ready_to_read(EV_P_ ev_io *w, int revents) throw() {
	struct connection *c = reinterpret_cast<struct connection*>(w->data);

	std::string buf;
	try {
		buf = read(c->sock);

	} catch( IOError &e ) {
		*log << c->id << " : IO error, closing connection: " << e.what() << "\n" << std::flush;
		if( c == &serial ) throw;
		kill_connection(EV_A_ w);
		return; // early

	} catch( EOFreached &e ) {
		*log << c->id << " : Disconnect\n" << std::flush;
		if( c == &serial ) throw;
		kill_connection(EV_A_ w);
		return; // early
	}

	c->buf.append(buf);

	// Queue the processing job
	ev_idle_start(EV_A_ &c->processing_todo );
	/* We're no longer interested in reading (until processing is done)
	 * Or more accurately: we don't want to be responsible for the buffering
	 * the data until processing is done; let the kernel handle that
	 */
	ev_io_stop(EV_A_ &c->read_ready );
}

void process_read_data(EV_P_ ev_idle *w, int revents) {
	struct connection *c = reinterpret_cast<struct connection*>(w->data);

	std::auto_ptr<ElcobusMessage::ElcobusMessage> m;
	std::string msg;
	try {
		m.reset( ElcobusMessage::parse_and_consume(c->buf, &msg) );

	} catch( ElcobusMessage::InsufficientData &e ) {
		ev_idle_stop(EV_A_ &c->processing_todo ); // No more processing to do
		ev_io_start(EV_A_ &c->read_ready ); // We're ready to read again
		return; // and wait for more data

	} catch( ElcobusMessage::FormError &e ) {
		*log << c->id << " : Form Error in data (" << e.details() << "), ignoring byte "
			 << "0x" << hex(c->buf[0]) << "\n" << std::flush;
		c->buf = c->buf.substr(1);
		return; // and retry from next byte in the next event-loop
	}

	*log << c->id << " : " << m->string() << "\n" << std::flush;
	if( c != &serial ) {
		try {
			write(serial.sock, msg);
		} catch( IOError &e ) {
			throw;
		}
	}
	for( typeof(c_network.begin()) i = c_network.begin(); i != c_network.end(); ++i ) {
		if( &(*i) == c ) continue; // Don't loop input to same socket
		try {
			write(i->sock, msg);
		} catch( IOError &e ) {
			*log << i->id << " : IO error, closing connection: " << e.what() << "\n" << std::flush;
			ev_io *w = &( i->read_ready );
			--i; // Prepare iterator for deletion
			kill_connection(EV_A_ w );
		}
	}
}

void incomming_connection(EV_P_ ev_io *w, int revents) {
	std::auto_ptr<struct connection> new_con( new struct connection );
	std::auto_ptr<SockAddr::SockAddr> client_addr;
	new_con->sock = s_listen.accept(&client_addr);
	new_con->id = client_addr->string();
	*log << new_con->id << " : Connection opened\n" << std::flush;

	// Set socket non-blocking
	int flags = fcntl(new_con->sock, F_GETFL);
	if( flags == -1 ) {
		char error_descr[256];
		strerror_r(errno, error_descr, sizeof(error_descr));
		*log << new_con->id << " : Could not fcntl(, F_GETFL): " << error_descr << "\n" << std::flush;
		*log << new_con->id << " : Closing connection\n" << std::flush;
		return; // Without setting watcher & without keeping connection
	}
	int rv = fcntl(new_con->sock, F_SETFL, flags | O_NONBLOCK);
	if( rv == -1 ) {
		char error_descr[256];
		strerror_r(errno, error_descr, sizeof(error_descr));
		*log << new_con->id << " : Could not fcntl(, F_SETFL): " << error_descr << "\n" << std::flush;
		*log << new_con->id << " : Closing connection\n" << std::flush;
		return; // Without setting watcher & without keeping connection
	}

	ev_io_init( &new_con->read_ready, ready_to_read, new_con->sock, EV_READ );
	new_con->read_ready.data = new_con.get();
	ev_io_start( EV_A_ &new_con->read_ready );

	ev_idle_init( &new_con->processing_todo, process_read_data );
	new_con->processing_todo.data = new_con.get();

	c_network.push_back( new_con.release() );
}

int main(int argc, char* argv[]) {
	// Defaults
	struct {
		bool fork;
		std::string pid_file;
		std::string serial_port;
		std::string bind_addr;
	} options = {
		/* fork = */ true,
		/* pid_file = */ "",
		/* serial_port = */ "/dev/ttyS0",
		/* bind_addr = */ "[::1]:[10000]"
		};
	log.reset( new TimestampLog( std::cerr ) );

	{ // Parse options
		char optstring[] = "?hfp:s:b:l:";
		struct option longopts[] = {
			{"help",			no_argument, NULL, '?'},
			{"forgeground",		no_argument, NULL, 'f'},
			{"pid-file",		required_argument, NULL, 'p'},
			{"serialport",		required_argument, NULL, 's'},
			{"bind",			required_argument, NULL, 'b'},
			{"log",				required_argument, NULL, 'l'},
			{NULL, 0, 0, 0}
		};
		int longindex;
		int opt;
		while( (opt = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1 ) {
			switch(opt) {
			case '?':
			case 'h':
				std::cerr <<
				//  >---------------------- Standard terminal width ---------------------------------<
					"Options:\n"
					"  -h -? --help                    Displays this help message and exits\n"
					"  --foreground -f                 Don't fork into the background after init\n"
					"  --pid-file -p file              The file to write the PID to, especially\n"
					"                                  usefull when running as a daemon\n"
					"  --serialport -s  /dev/ttyS0     The serial device to use\n"
					"  --bind -b host:port             Bind to the specified address\n"
					"                                  host and port resolving can be bypassed by\n"
					"                                  placing [] around them\n"
					"  --log -l file                   Log to file\n"
					;
				exit(EX_USAGE);
			case 'f':
				options.fork = false;
				break;
			case 'p':
				options.pid_file = optarg;
				break;
			case 's':
				options.serial_port = optarg;
				break;
			case 'b':
				options.bind_addr = optarg;
				break;
			case 'l':
				logfilename = optarg;
				logfile.open(logfilename.c_str(), std::ios_base::app | std::ios_base::out );
				log.reset( new TimestampLog( logfile ) );
				break;
			}
		}
	}

	*log << "Parsed options, opening serial port \"" << options.serial_port << "\"\n" << std::flush;

	{ // Open serial port
		serial.id = "SERIAL";
		serial.sock = open(options.serial_port.c_str(), O_RDWR | O_NOCTTY);
		// Open in Read-Write; don't become controlling TTY
		if( serial.sock == -1 ) {
			std::cerr << "Could not open \"" << options.serial_port << "\": ";
			perror("open()");
			exit(EX_NOINPUT);
		}
		*log << "Opened port \"" << options.serial_port << "\"\n" << std::flush;

		// Setting up port
		struct termios port_options;
		tcgetattr(serial.sock, &port_options);

		cfsetispeed(&port_options, B4800);
		cfsetospeed(&port_options, B4800);

		port_options.c_cflag |= CLOCAL; // Don't change owner of port
		port_options.c_cflag |= CREAD; // Enable receiver

		port_options.c_cflag &= ~CSIZE; // Mask the character size bits
		port_options.c_cflag |= CS8;    // Select 8 data bits

		port_options.c_cflag |= PARENB; // Set parity
		port_options.c_cflag |= PARODD; // Set parity to odd
		port_options.c_cflag &= ~CSTOPB; // Clear "2 Stopbits" => 1 stopbit

		port_options.c_cflag &= ~CRTSCTS; // Disable hardware flow control

		port_options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN); // Raw mode

		port_options.c_iflag |= IGNPAR; // Ignore parity (since none is used)
		port_options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
		port_options.c_iflag &= ~INLCR; // Turn off any conversions
		port_options.c_iflag &= ~ICRNL;
		port_options.c_iflag &= ~IGNCR;

		port_options.c_oflag &= ~OPOST; // Disable output processing = raw mode

		// Apply port_options
		tcsetattr(serial.sock, TCSANOW, &port_options);

		*log << "Configured port \"" << options.serial_port << "\"\n" << std::flush;
	}

	{ // Open listening socket
		std::string host, port;

		/* Address format is
		 *   - hostname:portname
		 *   - [numeric ip]:portname
		 *   - hostname:[portnumber]
		 *   - [numeric ip:[portnumber]
		 */
		size_t c = options.bind_addr.rfind(":");
		if( c == std::string::npos ) {
			std::cerr << "Invalid bind string \"" << options.bind_addr << "\": could not find ':'\n";
			exit(EX_DATAERR);
		}
		host = options.bind_addr.substr(0, c);
		port = options.bind_addr.substr(c+1);

		std::auto_ptr< boost::ptr_vector< SockAddr::SockAddr> > bind_sa
			= SockAddr::resolve( host, port, 0, SOCK_STREAM, 0);
		if( bind_sa->size() == 0 ) {
			std::cerr << "Can not bind to \"" << options.bind_addr << "\": Could not resolve\n";
			exit(EX_DATAERR);
		} else if( bind_sa->size() > 1 ) {
			// TODO: allow this
			std::cerr << "Can not bind to \"" << options.bind_addr << "\": Resolves to multiple entries:\n";
			for( typeof(bind_sa->begin()) i = bind_sa->begin(); i != bind_sa->end(); i++ ) {
				std::cerr << "  " << i->string() << "\n";
			}
			exit(EX_DATAERR);
		}
		s_listen = Socket::socket( (*bind_sa)[0].proto_family() , SOCK_STREAM, 0);
		s_listen.set_reuseaddr();
		s_listen.bind((*bind_sa)[0]);
		s_listen.listen(MAX_CONN_BACKLOG);
		*log << "Listening on " << (*bind_sa)[0].string() << "\n" << std::flush;
	}

	{
		struct sigaction act;
		if( sigaction(SIGPIPE, NULL, &act) == -1) {
			std::cerr << "Can not get signal handler for SIGPIPE\n";
			exit(EX_OSERR);
		}
		act.sa_handler = SIG_IGN; // Ignore SIGPIPE (we'll handle the write()-error)
		if( sigaction(SIGPIPE, &act, NULL) == -1 ) {
			std::cerr << "Can not set signal handler for SIGPIPE\n";
			exit(EX_OSERR);
		}
	}

	{
		/* Open pid-file before fork()
		 * That way, failing to open the pid-file will cause a pre-fork-abort
		 */
		std::ofstream pid_file;
		if( options.pid_file.length() > 0 ) pid_file.open( options.pid_file.c_str() );

		if( options.fork ) {
			pid_t child = fork();
			if( child == -1 ) {
				char error_descr[256];
				strerror_r(errno, error_descr, sizeof(error_descr));
				*log << "Could not fork: " << error_descr << "\n" << std::flush;
				exit(EX_OSERR);
			} else if( child == 0 ) {
				// We are the child
				// continue on with the program
			} else {
				// We are the parent
				exit(0);
			}
		}

		if( options.pid_file.length() > 0 ) pid_file << getpid();
	}

	{
		ev_signal ev_sigint_watcher;
		ev_signal_init( &ev_sigint_watcher, received_sigint, SIGINT);
		ev_signal_start( EV_DEFAULT_ &ev_sigint_watcher);
		ev_signal ev_sigterm_watcher;
		ev_signal_init( &ev_sigterm_watcher, received_sigterm, SIGTERM);
		ev_signal_start( EV_DEFAULT_ &ev_sigterm_watcher);

		ev_signal ev_sighup_watcher;
		ev_signal_init( &ev_sighup_watcher, received_sighup, SIGHUP);
		ev_signal_start( EV_DEFAULT_ &ev_sighup_watcher);

		ev_io_init( &serial.read_ready, ready_to_read, serial.sock, EV_READ );
		ev_set_priority( &serial.read_ready, 1); // Always process serial first
		serial.read_ready.data = &serial;
		ev_io_start( EV_DEFAULT_ &serial.read_ready );

		ev_idle_init( & serial.processing_todo, process_read_data );
		serial.processing_todo.data = &serial;
		// Don't start

		ev_io e_listen;
		ev_io_init( &e_listen, incomming_connection, s_listen, EV_READ );
		ev_io_start( EV_DEFAULT_ &e_listen );

		*log << "Setup done, starting event loop\n" << std::flush;

		ev_loop(EV_DEFAULT_ 0);
	}

	*log << "Cleaning up\n" << std::flush;
	if( options.pid_file.length() > 0 ) remove( options.pid_file.c_str() );

	return 0;
}
