#!/usr/bin/perl
# Offline regression tests for the playerbot strategies (no server, no database).
#
# Every contrib/tests/*_test.cpp is compiled with MSVC and run. A line
#   //@extract <file> | <signature> [| <after marker>]
# is replaced by that function (or enum/struct body) copied unchanged from
# src/modules/PlayerBots/playerbot/strategy/<file>, so the tests exercise the
# production code against small fixtures. With an after marker the search
# starts behind it (e.g. the TBC part of a strategy file).
#
# Usage (Git Bash, from anywhere):  perl contrib/tests/run_tests.pl [name filter]
use strict;
use warnings;
use File::Basename qw(dirname basename);
use File::Path qw(make_path);
use Cwd qw(abs_path);

# Mixed Windows paths (C:/...) work for Perl and cl.exe; MSYS must not rewrite /nologo and co.
$ENV{MSYS2_ARG_CONV_EXCL} = '*';
my $root = abs_path(dirname(abs_path($0)) . '/../..');
chomp($root = `cygpath -m "$root"`) if $root =~ m{^/};
my $testsDir = "$root/contrib/tests";
my $strategyDir = "$root/src/modules/PlayerBots/playerbot/strategy";
my $outDir = "$root/build/tests";
my $filter = shift // '';

make_path($outDir);

sub ReadFile
{
    my ($path) = @_;
    open(my $fh, '<', $path) or die "cannot read $path: $!\n";
    local $/;
    my $text = <$fh>;
    close($fh);
    $text =~ s/\r\n/\n/g;
    return $text;
}

sub WriteFile
{
    my ($path, $text) = @_;
    open(my $fh, '>', $path) or die "cannot write $path: $!\n";
    print $fh $text;
    close($fh);
}

# Same brace matching as the former Python helpers: from the signature to its closing brace
sub Extract
{
    my ($file, $signature, $after) = @_;
    my $text = ReadFile("$strategyDir/$file");
    my $from = 0;
    if (defined $after)
    {
        $from = index($text, $after);
        die "marker '$after' not found in $file\n" if $from < 0;
    }

    my $start = index($text, $signature, $from);
    die "'$signature' not found in $file\n" if $start < 0;

    my $pos = index($text, '{', $start) + 1;
    my $depth = 1;
    while ($depth)
    {
        die "unbalanced braces after '$signature' in $file\n" if $pos >= length($text);
        my $c = substr($text, $pos++, 1);
        $depth++ if $c eq '{';
        $depth-- if $c eq '}';
    }

    return substr($text, $start, $pos - $start) . "\n";
}

sub Expand
{
    my ($source) = @_;
    $source =~ s{^//\@extract\s+(.+?)\s*$}{
        my @parts = map { s/^\s+|\s+$//gr } split(/\|/, $1);
        "// --- extracted from $parts[0]\n" . Extract(@parts);
    }gme;
    return $source;
}

# cl.exe and its INCLUDE/LIB environment, taken from vcvars64.bat once
sub SetupCompiler
{
    return 'cl' if $ENV{INCLUDE} && `where cl 2>NUL`;

    my $vswhere = 'C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe';
    die "vswhere.exe not found, run from a Visual C++ developer prompt\n" unless -f $vswhere;
    my $install = `"$vswhere" -latest -products "*" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`;
    $install =~ s/\s+$//;
    die "no Visual C++ installation found\n" unless $install;

    my $bat = "$outDir/env.bat";
    WriteFile($bat, "\@call \"$install\\VC\\Auxiliary\\Build\\vcvars64.bat\" >NUL 2>NUL\r\n\@set\r\n");
    my $winBat = $bat =~ s{/}{\\}gr;
    foreach my $line (`cmd.exe /c "$winBat"`)
    {
        $line =~ s/\r?\n$//;
        next unless $line =~ /^([^=]+)=(.*)$/;
        my ($name, $value) = ($1, $2);
        # Git Bash keeps its own PATH format; only the compiler directories are added
        if (uc($name) eq 'PATH')
        {
            my @dirs = grep { /Microsoft Visual Studio|Windows Kits/i } split(/;/, $value);
            $ENV{PATH} = join(':', map { my $d = $_ =~ s{\\}{/}gr; $d =~ s{^([A-Za-z]):}{'/' . lc($1)}e; $d } @dirs) . ":$ENV{PATH}";
            next;
        }
        $ENV{$name} = $value if $name =~ /^(INCLUDE|LIB|LIBPATH|VCToolsInstallDir|WindowsSdkDir)$/i;
    }

    die "vcvars64.bat did not set INCLUDE\n" unless $ENV{INCLUDE};
    return 'cl';
}

# readdir instead of glob: the repository path contains spaces
opendir(my $dh, $testsDir) or die "cannot open $testsDir: $!\n";
my @tests = sort map { "$testsDir/$_" } grep { /_test\.cpp$/ && /\Q$filter\E/ } readdir($dh);
closedir($dh);
die "no tests match '$filter'\n" unless @tests;

my $cl = SetupCompiler();
my ($passed, @failed) = (0);
foreach my $test (@tests)
{
    my $name = basename($test, '.cpp');
    my $cpp = "$outDir/$name.cpp";
    my $exe = "$outDir/$name.exe";
    print "== $name\n";

    my $source = eval { Expand(ReadFile($test)) };
    if (!defined $source)
    {
        print "FAIL (extract): $@";
        push @failed, $name;
        next;
    }
    WriteFile($cpp, $source);

    my $compile = `"$cl" /nologo /EHsc /std:c++17 /W0 /I"$testsDir" "$cpp" /Fo"$outDir/$name.obj" /Fe"$exe" 2>&1`;
    if ($? != 0)
    {
        print "FAIL (compile):\n$compile";
        push @failed, $name;
        next;
    }

    my $output = `"$exe" 2>&1`;
    print $output;
    if ($? != 0)
    {
        print "FAIL (exit code " . ($? >> 8) . ")\n";
        push @failed, $name;
        next;
    }
    $passed++;
}

print "\n$passed passed, " . scalar(@failed) . " failed" . (@failed ? ": @failed" : '') . "\n";
exit(@failed ? 1 : 0);
