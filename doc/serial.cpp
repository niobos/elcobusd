#include <iostream>
//#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysexits.h>
#include <errno.h>
#include <assert.h>

void copy(int in, int out) {
	char buf[1024];
	size_t nbuf = read(in, buf, sizeof(buf));
	if( nbuf == -1 ) {
		perror("read()");
		exit(EX_IOERR);
	} else if( nbuf == 0 ) {
		std::cerr << "EOF on fd " << in << "\n";
		exit(0);
	}

	// Invert bit
	// XXX THIS IS ELCO-BUS SPECIFIC!!!!!! REMOVE FOR REGULAR SERIAL IO XXX
	for( size_t i=0; i<nbuf; i++ ) {
		buf[i] = ~buf[i];
	}

	size_t w = write(out, buf, nbuf);
	if( w == -1 ) {
		perror("write()");
		exit(EX_IOERR);
	} else if( w != nbuf ) {
		std::cerr << "write() count mismatch\n";
		exit(EX_IOERR);
	}
}

int main(int argc, char *argv[]) {
	if( argc != 2 ) {
		std::cerr << "Usage: " << argv[0] << " /dev/ttyS0\n";
		exit(EX_USAGE);
	}

	int sock = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
	// Open in Read-Write; don't become controlling TTY; (temp) set Nonblock
	if( sock == -1 ) {
		std::cerr << "Could not open serial: ";
		perror("open()");
		exit(EX_NOINPUT);
	}
	std::cerr << "Opened serial port\n";

	int flags = fcntl(sock, F_GETFL, 0);
	if( flags == -1 ) {
		perror("fcntl(<fd>, F_GETFL, 0)");
		exit(EX_OSERR);
	}
	if( fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == -1 ) {
		perror("fcntl(<fd>, F_SETFL, <flags>)");
		exit(EX_OSERR);
	}

	// Setting up port
	struct termios port_options;
	tcgetattr(sock, &port_options);

	cfsetispeed(&port_options, B4800);
	cfsetospeed(&port_options, B4800);

	port_options.c_cflag |= CLOCAL; // Don't change owner of port
	port_options.c_cflag |= CREAD; // Enable receiver

	port_options.c_cflag &= ~CSIZE; // Mask the character size bits
	port_options.c_cflag |= CS8;    // Select 8 data bits

	port_options.c_cflag |= PARENB; // expect parity
	port_options.c_cflag |= PARODD; // odd parity
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
	if( tcsetattr(sock, TCSANOW, &port_options) == -1 ) {
		perror("tcsetattr()");
		exit(EX_OSERR);
	}

	std::cerr << "Configured serial port, listening\n";

	do {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds); // stdin
		FD_SET(sock, &rfds); // serial
		int rv = select(sock+1, &rfds, NULL, NULL, NULL);

		if( rv == -1 ) {
			perror("select()");
			exit(EX_IOERR);
		}
		if( rv == 0 ) {
			std::cerr << "select() returned 0??\n";
			exit(EX_SOFTWARE);
		}

		if( FD_ISSET(0, &rfds) ) {
			copy(0, sock);

		} else if( FD_ISSET(sock, &rfds) ) {
			copy(sock, 1);

		} else {
			std::cerr << "select() returned non-zero, but couldn't find ready fd\n";
			exit(EX_SOFTWARE);
		}

	} while(1);
}
