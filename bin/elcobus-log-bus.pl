#!/usr/bin/perl

use strict;
use warnings;
use Digest::CRC;
use POSIX;

$|=1; # Don't buffer output

sub decode;

my @buf;
while( sysread(STDIN, my $ibuf, 256) ) {
	push @buf, map { ord($_) } split //, $ibuf;
	$ibuf = ""; # if we redo this, don't re-add the new data

	next unless @buf > 4;

	my $header = shift @buf;  # shift it off, so we retry on the next byte
	redo unless ($header & 0xdc) == 0xdc;

	redo unless ($buf[1-1] & 0x80) == 0x80; # From high bit set?
	redo unless ($buf[2-1] & 0x80) == 0x00; # To high bit clear?

	my $src = $buf[1-1] & 0x7f;
	my $dst = $buf[2-1] & 0x7f;
	my $len = $buf[3-1];

	if( @buf < $len-1 ) {
		unshift @buf, $header;
		next;
	}

	my $crc = Digest::CRC->new(width=>16, init=>0x0000, xorout=>0x0000,
	                           refout=>0, poly=>0x1021, refin=>0, cont=>1 );
	$crc->add(chr($header));
	$crc->add( join '', map { chr($_) } @buf[0..($len-2-1-1)] );
	my $crc_expected = $crc->digest;

	my $crc_actual = $buf[ $len-2-1 ]*256 + $buf[ $len-1-1 ];

	if( $crc_expected != $crc_actual ) {
		printf STDERR "CRC mismatch (got 0x%04x, expected 0x%04x)\n", $crc_actual, $crc_expected;
		redo;
	}

	my @data = @buf[(4-1)..($len-2-1-1)];
	@buf = @buf[($len-1)..$#buf];

	decode($header, $src, $dst, @data);

	redo if @buf;
	# next (i.e. read more data) if @buf is empty
}

sub decode ($$$@) {
	my ($header, $src, $dst, @data) = @_;

	printf "%s STDIN : Unknown message: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[%d]=%s\n",
		   POSIX::strftime("%Y-%m-%dT%H:%M:%S%z", localtime),
		   $header, $src, $dst, scalar @data,
		   join(' ', map { sprintf "%02x", $_ } @data);
}
