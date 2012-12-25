#!/usr/bin/perl

use strict;
use warnings;
use Digest::CRC;

while(<>) {
	unless( m/^( *[0-9.]+): ([0-9a-f ]+)$/ ) {
		print STDERR "Unrecognized line: $_";
		next;
	}
	my ($time, $msg) = ($1, $2);
	my @msg = map { hex($_) } split / /, $msg;
	#print "$time: ", join(' ', map { sprintf "%02x", $_ } @msg), "\n";

	my $crc = Digest::CRC->new(width=>16, init=>0x0000, xorout=>0x0000, refout=>0, poly=>0x1021, refin=>0, cont=>1 );
	$crc->add( join '', map { chr($_) } @msg[0..$#msg] );

	printf "%s: %s   %04x\n", $time, join(' ', map { sprintf "%02x", $_ } @msg), $crc->digest;
}
