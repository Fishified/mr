#!/usr/bin/perl

use strict;

my ($i, $j, $n);
my ($F, $pid, @pid);

sub quit 
{
  kill 'KILL', @pid;

  for ($i = 0; $i < $n; $i++) {
    unlink "/tmp/reader$i";
  }
};

$SIG{INT} = $SIG{KILL} = \&quit;

$n = $ARGV[0] || 2; # min 1, max 25
$n = 25 if $n > 25;

for ($i = 0; $i < $n; $i++) {
  warn "creating reader thread $i\n";
  if ($pid = fork) {
    push @pid, $pid;
    next;
  } else {
    if (!open $F, ">/tmp/reader$i") {
      die "can't open reader$i ($!)\n";
    }
    select $F;
    $|++;
    for ($j = 0; $j < 60; $j++) {
      print "LA$j B R$i\n";
      sleep 3;
    } last;
  }
}

sleep 60*3;

&quit;
