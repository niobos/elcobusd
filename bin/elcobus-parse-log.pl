#!/usr/bin/perl

use strict;
use warnings;
use Data::ParseBinary;
use Data::Dumper;


sub hexdump { # {{{
	return join(' ', map { sprintf "%02x", $_ } @_);
} # }}}

sub binary ($) { # {{{
	return join '', unpack "B8", pack "C", $_[0];
} # }}}

sub enum { # {{{
	my ($key, %hash) = @_;
	return $hash{$key} if defined $hash{$key};
	return sprintf "Unknown[0x%02x]", $key;
} # }}}

sub twos_complement { # {{{
	my (@byte) = @_;
	my $ret = 0;
	if( $byte[0] & 0x80 ) {
		# negative
		while( @byte ) {
			$ret <<= 8;
			$ret |= (shift @byte) ^ 0xff;
		}
		$ret = -($ret + 1);
	} else {
		# positive
		while( @byte ) {
			$ret <<= 8;
			$ret |= shift @byte;
		}
	}
	return $ret;
} # }}}

my @parser;

push @parser, sub { # Boiler Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x05;
	return undef unless $data[4] == 0x19;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("BoilerTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Boiler Set Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x09;
	return undef unless $data[4] == 0x23;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("BoilerSetTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Return Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x05;
	return undef unless $data[4] == 0x1a;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("ReturnTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Outdoor Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x05;
	return undef unless $data[4] == 0x21;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("OutdoorTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Tap Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x05;
	return undef unless $data[4] == 0x2f;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("TapTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Tap Set Temperature {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 8;
	return undef unless $data[3] == 0x07;
	return undef unless $data[4] == 0x4b;

	my $temp = twos_complement( @data[6..7] ) / 64;

	return sprintf("TapSetTemp: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[6]=%s  Temp=%3.2fC",
			$header, $src, $dst, hexdump(@data[0..5]), $temp);
}; # }}}

push @parser, sub { # Status {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 7;
	return undef unless $data[3] == 0x30;
	return undef unless $data[4] == 0x34;

	return sprintf("Status: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[5]=%s  Status=%d",
			$header, $src, $dst, hexdump(@data[0..5]), $data[6]);
}; # }}}

push @parser, sub { # Modulation % {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 7;
	return undef unless $data[3] == 0x30;
	return undef unless $data[4] == 0x5f;

	return sprintf("Modulation: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[5]=%s  Modulation=%d%%",
			$header, $src, $dst, hexdump(@data[0..5]), $data[6]);
}; # }}}

push @parser, sub { # Pump % {{{
	my ($header, $src, $dst, $data, @data) = @_;

	return undef unless @data == 7;
	return undef unless $data[3] == 0x04;
	return undef unless $data[4] == 0xa2;

	return sprintf("Pump: Hdr=0x%02x Src=0x%02x Dst=0x%02x data[5]=%s  Pump=%d%%",
			$header, $src, $dst, hexdump(@data[0..5]), $data[6]);
}; # }}}







$| = 1; # disable buffering

while(<>) {
	# 2011-08-30T09:29:46+0200 SERIAL : Unknown message: Hdr=0xdc Src=0x00 Dst=0x00 data[8]=fb 01 00 00 00 00 00 00
	#               2011-08-30T09:29:46+0200                     SERIAL : Unknown message: Hdr0xdc3            Src=0x2              Dst=0x00           data [ 8 ]=fb 01 00 00 00 00 00 00
	#              1                                             2                              3                    4                  5                         6
	next unless m/^(\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\d\+\d\d\d\d) (.*?) : Unknown message: Hdr=0x([0-9a-fA-F]+) Src=0x([0-9a-fA-F]+) Dst=0x([0-9a-fA-F]+) data\[\d\]=(.*)$/;
	my ($timestamp, $source, $header, $src, $dst, $data) = ($1, $2, $3, $4, $5, $6);
	$header = hex $header;
	$src = hex $src;
	$dst = hex $dst;
	my @data = map { hex $_ } split / /, $data;
	$data = join '', map { chr($_); } @data;

	for my $p (@parser) {
		my $temp = &{$p}($header, $src, $dst, $data, @data);
		if( defined $temp ) {
			$_ = "$timestamp $source : $temp\n";
			last; # `for` iteration
		}
	}

} continue {
	print $_;
}

# vim: set foldmethod=marker:
