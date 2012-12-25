#!/usr/bin/perl

use strict;
use warnings;
use Audio::Wav;
use Digest::CRC;
use Term::ANSIColor;


my %stthreshold = (low => -4000, high => -3000);
my $bit_duration = 44100 / 4800;
my $end_of_tx = 2;

my $wav = (new Audio::Wav)->read($ARGV[0]);

sub decode;

my $state = 0;
my $stx;
my ($prev_bit, $bit) = (0,0);
my @byte;
my @msg;
while( my @sample = $wav->read() ) {
	$prev_bit = $bit;
	$bit = ($sample[0] > ( $prev_bit ? $stthreshold{low} : $stthreshold{high} ) ? 0 : 1);

	if( $state == -1 && $wav->position_samples() > $stx + (11+$end_of_tx)*$bit_duration ) {
		decode $stx/44100, @msg;

		$state = 0;
		@msg = ();
	}

	if( $state <= 0 ) {
		next unless $bit == 1;
		$state = 0.5 - 1/$bit_duration;
		$stx = $wav->position_samples();
		@byte = ();
	}

	$state += 1/$bit_duration;

	if( ($state - int($state)) < 1/$bit_duration ) {
		push @byte, $bit;
	}

	if( $state > 11 ) {
		# stop bit received
		$state = -1;

		print STDERR "$stx: start bit not 1\n" unless $byte[0] == 1;
		print STDERR "$stx: stop bit not 0\n" unless $byte[10] == 0;
		my $parity = 0; $parity += $_ foreach (@byte[1..8]); $parity %= 2;
		print STDERR "$stx: parity not even\n" unless $byte[9] == $parity;

		my $byte = unpack "C", pack "b8", join('', @byte[1..8]); # TODO: bit order?, bit polarity?

		push @msg, $byte;
	}
}

sub decode {
	my ($time, @msg) = @_;

	my $crc = Digest::CRC->new(width=>16, init=>0x0000, xorout=>0x0000,
							   refout=>0, poly=>0x1021, refin=>0, cont=>1 );
	$crc->add( join '', map { chr($_) } @msg[0..$#msg] );

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
		($crc->digest == 0 ? colored("CRC OK","green") : colored("CRC FAILED","red")),
		"\n";
}
