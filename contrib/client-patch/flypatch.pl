#!/usr/bin/perl
# Usage: perl flypatch.pl <server dbc dir> <out.MPQ>
# Sets AreaTable flag 0x400 (client "flyable") on all Azeroth areas and packs
# DBFilesClient\AreaTable.dbc into an uncompressed MPQ v1 archive.
use strict; use warnings;
my ($dbcdir, $out) = @ARGV;
sub rd { open my $f, '<:raw', $_[0] or die "$_[0]: $!"; local $/; my $d = <$f>; $d }

# --- WorldMapArea: zone -> virtual map
my $wma = rd("$dbcdir/WorldMapArea.dbc");
my (undef, $wn, undef, $wrs) = unpack('a4V3', $wma);
my %virt;
for my $i (0 .. $wn - 1) {
    my @v = unpack('V*', substr($wma, 20 + $i * $wrs, $wrs));
    my $vm = $v[8] >= 0x80000000 ? -1 : $v[8];
    $virt{$v[2]} = $vm >= 0 ? $vm : $v[1];
}

# --- AreaTable
my $at = rd("$dbcdir/AreaTable.dbc");
my (undef, $n, undef, $rs) = unpack('a4V3', $at);
my (%rec, $changed);
for my $i (0 .. $n - 1) { my @v = unpack('V5', substr($at, 20 + $i * $rs, 20)); $rec{$v[0]} = [$i, @v]; }
for my $id (keys %rec) {
    my ($i, undef, $map, $zone, undef, $flags) = @{ $rec{$id} };
    my $root = $zone || $id;
    my $vmap = $map == 530 ? ($virt{$root} // 530) : $map;
    next unless $vmap == 0 || $vmap == 1;
    next unless $map == 0 || $map == 1 || $map == 530;
    next if $flags & 0x400;
    substr($at, 20 + $i * $rs + 16, 4) = pack('V', $flags | 0x400);
    $changed++;
}
print "areas flagged: $changed\n";

# --- MPQ crypto
my @ct; { my $s = 0x00100001;
  for my $a (0 .. 255) { my $b = $a;
    for (0 .. 4) { $s = ($s * 125 + 3) % 0x2AAAAB; my $t1 = ($s & 0xFFFF) << 16;
                   $s = ($s * 125 + 3) % 0x2AAAAB; $ct[$b] = $t1 | ($s & 0xFFFF); $b += 256; } } }
sub hs { my ($str, $t) = @_; my ($s1, $s2) = (0x7FED7FED, 0xEEEEEEEE);
  for my $c (map { ord } split //, uc $str) {
    $s1 = ($ct[$t * 256 + $c] ^ (($s1 + $s2) & 0xFFFFFFFF)) & 0xFFFFFFFF;
    $s2 = ($c + $s1 + $s2 + (($s2 << 5) & 0xFFFFFFFF) + 3) & 0xFFFFFFFF; }
  $s1 }
sub enc { my ($data, $key) = @_; my $s = 0xEEEEEEEE; my @o;
  for my $v (unpack('V*', $data)) {
    $s = ($s + $ct[0x400 + ($key & 0xFF)]) & 0xFFFFFFFF;
    push @o, ($v ^ (($key + $s) & 0xFFFFFFFF));
    $key = (((~$key << 0x15) & 0xFFFFFFFF) + 0x11111111 & 0xFFFFFFFF) | ($key >> 0x0B);
    $s = ($v + $s + (($s << 5) & 0xFFFFFFFF) + 3) & 0xFFFFFFFF; }
  pack('V*', @o) }

my $name = 'DBFilesClient\\AreaTable.dbc';
my $hsize = 16;
my $hash = pack('V4', 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF) x $hsize;
substr($hash, (hs($name, 0) & ($hsize - 1)) * 16, 16) = pack('VVvvV', hs($name, 1), hs($name, 2), 0, 0, 0);
my $hpos = 32 + length $at;
my $bpos = $hpos + $hsize * 16;
my $block = pack('V4', 32, length $at, length $at, 0x80000000);
my $total = $bpos + 16;
open my $o, '>:raw', $out or die "$out: $!";
print $o pack('a4VVvvVVVV', "MPQ\x1A", 32, $total, 0, 3, $hpos, $bpos, $hsize, 1);
print $o $at, enc($hash, hs('(hash table)', 3)), enc($block, hs('(block table)', 3));
close $o;
print "wrote $out ($total bytes)\n";
