#!/usr/bin/perl

use strict;
use warnings;
use Digest::CRC;
use Term::ANSIColor;

$|=1; # autoflush

sub decode;

my @buf;
while( sysread(STDIN, my $ibuf, 256 ) ) {
	push @buf, map { ord($_) } split //, $ibuf;

	while( @buf && $buf[0] != 0xdc ) {
		# remove leading garbage
		shift @buf;
	}

	next unless @buf >= 4;
	my $len = $buf[3];
	next unless @buf >= $len;

	my @msg = @buf[0..$len-1];
	@buf = @buf[$len..$#buf];

	decode(time, @msg);
}

sub from_s16be {
	return unpack("s>", join('', map{chr($_)} @_))
}

sub decode {
	my ($time, @msg) = @_;

	my $crc = Digest::CRC->new(width=>16, init=>0x0000, xorout=>0x0000,
							   refout=>0, poly=>0x1021, refin=>0, cont=>1 );
	$crc->add( join '', map { chr($_) } @msg[0..$#msg] );

	my $cor_crc = Digest::CRC->new(width=>16, init=>0x0000, xorout=>0x0000,
							   refout=>0, poly=>0x1021, refin=>0, cont=>1 );
	$cor_crc->add( join '', map { chr($_) } @msg[0..$#msg-2] );

	print sprintf("% 8.3f", $time),
		": ",
		sprintf("%02x", $msg[0]),
		" [",
		(($msg[1] & 0x80) == 0x80 ? colored("1", "green") : colored("0", "red")),
		" src=" . sprintf("0x%02x", $msg[1] & 0x7f) . "]",
		" [" . (($msg[2] & 0x80) == 0x00 ? colored("0", "green") : colored("1", "red")),
		" dst=" . sprintf("0x%02x", $msg[2] & 0x7f) . "]",
		" ",
		"[" . ($msg[3] == $#msg+1 ? colored("len", "green") : colored(sprintf("%02x", $msg[3]), "red")) . "]",
		" ",
		join(' ', map { sprintf "%02x", $_; } @msg[4..$#msg-2]),
		" ",
		($crc->digest == 0 ? colored("CRC OK","green") : colored("CRC FAILED (".$cor_crc->digest,"red")),
		"   ";

	if( $#msg == 13 && ( @msg[7,8] == (0x05, 0x19) || @msg[7,8] == (0x05,0x23) ) ) {
			print "Boilertemp" . ($msg[8]==0x23 ? "_target" : "" ) . "=" . ($msg[10]*256+$msg[11])/64;

	} elsif( $#msg == 13 && @msg[7,8] == (0x05, 0x1a) ) {
		print "Return=" . ($msg[10]*256+$msg[11])/64;

	} elsif( $#msg == 13 && @msg[7,8] == (0x05, 0x21) ) {
		print "Outdoor=" . from_s16be(@msg[10..11])/64;  # TODO: verify 2's complement

	} elsif( $#msg == 13 && @msg[7,8] == (0x05, 0x2f) ) {
		print "Tap=" . ($msg[10]*256+$msg[11])/64;

	} elsif( $#msg == 12 && @msg[7,8] == (0x30, 0x34) ) {
		print "Status=" . $msg[10];

	}

	print "\n";
}
