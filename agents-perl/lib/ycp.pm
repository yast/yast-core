########################################################################
#
#                                                                    #
#                    __   __    ____ _____ ____                      #
#                    \ \ / /_ _/ ___|_   _|___ \                     #
#                     \ V / _` \___ \ | |   __) |                    #
#                      | | (_| |___) || |  / __/                     #
#                      |_|\__,_|____/ |_| |_____|                    #
#                                                         -o)
#------------------------------------------------------   /\\  --------
#                                                        _\_v
#
#   Author:        Michael Hager <mike@suse.de>
#		   Martin Vidner <mvidner@suse.cz>
#
#   Description:   perl interface for YCP
#
#   Purpose:       Call a perl script within a YCP script
#
#----------------------------------------------------------------------
# $Id$

package ycp;

=head1 NAME

ycp - a Perl module for parsing and writing the YaST2 Communication Protocol

=head1 SYNOPSIS

C<($symbol, @config) = ycp::ParseTerm ('MyAgentConfig ("/etc/file", false, true, $["a":1, "b":2])');>

C<($command, $path, $arg) = ycp::ParseCommand ('Write (.magic.path, "abacadabra")');>

C<ycp::Return (["arbitrarily", "complex", "data"]);>

=head1 DATA

=head2 PerlYCPValue

PerlYCPValue is a convention for storing a YCP value in a Perl variable.
L</ParseYcp> parses YCP string representation into PerlYCPValues.

A PerlYCPValue cannot represent a term but only a term is allowed
to initialize an agent in a .scr file. Therefore L</ParseTerm> is provided.

=over 4

=item string, integer, boolean

Stored as a scalar.

=item list

Stored as a reference to a list of PerlYCPValues.

=item map

Stored as a reference to a map  of PerlYCPValues.

=item path

Stored as a reference to a string (starting with a "." as expected).

=item nil (void)

Stored as an undef.

=back

=head1 PARSING

=cut

use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION);
use Exporter;
use diagnostics;
use strict;
use Time::localtime;
use Sys::Hostname;

@ISA     = qw(Exporter);


my @e_io = qw(
	       ParseTerm
	       ParseCommand
	       PathComponents
	       Return
	       );

@EXPORT_OK = @e_io;

my @e_logging = qw(y2debug y2milestone y2warning y2error y2security y2internal);
my @e_obsolete  = qw(
	       ycpDoVerboseLog
	       ycpInit
	       ycpArgIsMap
	       ycpArgIsList
	       ycpArgIsInteger
	       ycpArgIsString
	       ycpArgIsNil
	       ycpArgIsNone
	       ycpGetArgMap
	       ycpGetArgList
	       ycpGetArgString
	       ycpGetArgInteger
	       ycpReturnSkalarAsInt
	       ycpReturnArrayAsList
	       ycpReturnSkalarAsBoolean
	       ycpReturnHashAsMap
	       ycpReturnSkalarAsString
	       ycpCommandIsDir
	       ycpCommandIsRead
	       ycpCommandIsWrite
	       ycpCommandIsExecute
	       ycpCommandIsResult
	       ycpGetCommand
	       ycpGetPath
	       ycpGetArgType
	       ycpReturn );

@EXPORT = (@e_logging, @e_obsolete);

our %EXPORT_TAGS = (IO => [@e_io],
		    LOGGING => [@e_logging],
		    OBSOLETE => [@e_obsolete]);

my $ycpcommand    = "";
my $ycppath       = "";

my $type          = "unknown";
my $ismap         = 0;
my $islist        = 0;
my $isinteger     = 0;
my $isstring      = 0;
my $isknown       = 0;
my $isnil         = 0;
my $isnone        = 0;

my %arghash;
my @argarray;
my $argskalar;

my $verbose = 0;
my $againstcompileerror = 1;

my $hostname = hostname();


################################################################################
# Parsing
################################################################################

=head2 ParseCommand

ParseComand $line

C<($command, $path, $arg) = ParseCommand ('Write (.moria.gate, "mellon")');>

Parse a SCR command of the form Command (.some.path, optional_argument)

Returns a three element list ("Command", ".some.path", $argument)
where the argument is a L</PerlYCPValue> and will be undef
if it was not specified.
Note that the path is converted to a string.

If there was a parse error, the command or path will be the empty string.

=cut

sub ParseCommand ($)
{
    my @term = ParseTerm (shift);

    my $command = shift @term || "";

    my $path = "";
    my $pathref = shift @term;
    if (defined $pathref)
    {
	if (ref($pathref) eq "SCALAR" && $$pathref =~ /^\./)
	{
	    $path = $$pathref;
	}
	# 'result (nil)' is a standard command
	elsif ($command ne "result")
	{
	    y2error ("The first argument is not a path. ('$pathref')");
	}
    }

    my $argument = shift @term;
    y2warning ("Superfluous command arguments ignored") if (@term > 0);

    return ($command, $path, $argument);
}

=head2 ParseTerm

ParseTerm $line

C<($symbol, @config) = ParseTerm ('MyAgentConfig ("/etc/file", false, true, $["a":1, "b":2])');>

Parse a YCP term. Note that there can be no other term inside.

Returns a list whose first element is the term symbol as a string
(or C<""> in case of an error) and the remaining elements are the term
arguments (L</PerlYCPValue>)

=cut

sub ParseTerm ($)
{
    my $input = shift;

    my $symbol;
    my @ret;

    $input =~ s/^\s*`?(\w*)\s*//; # allow both Term and `Term (for the NI)
    $symbol = $1;
    if (! $symbol)
    {
	y2error ("No term symbol");
    }
    push @ret, $symbol;

    if ($input !~ m/^\(/)
    {
	y2error ("No term parentheses");
    }

    my ($argref, $err, $rest) = ParseYcpTermBody ($input);
    if ($err)
    {
	y2error ("$err ('$rest')");
    }
    else
    {
	push @ret, @$argref;
    }

    return @ret;
}

# ------------------------------------------------------------
# Internal parsing functions start here.

# PerlYCPParserResult is a triple:
# ($result, $error, $rest_of_input)
# where
#   $result is a PerlYCPValue.
#   $error is either "" or an error description.
#        In that case, $result is not specfied.
#   $rest_of_input is the unmatched part of the input.
#        On success, parsing can go on, on error, the ofending input is there.

# this is how it looks like in lex:
# PATHSEGMENT [[:alnum:]_-]+|\"([^\\"]*(\\.)*)+\"
my $lex_pathsegment = qr{
		(?:				# outer group
		[[:alnum:]_-]+			# ordinary segment
		|
		"
		(?:
		[^\\"]*				# any-except-bkls-quot
		(?: \\ . )*			# bksl, any
		)+
		"
		)
		}x;		# enable whitespace and comments in regex

# Internal
# Parses a YCP value. See PerlYCPValue. Notably terms are not supported.
# Returns PerlYCPParserResult.
sub ParseYcp ($)
{
    my $ycp_value = shift;

    #remove leading whitespace;
    $ycp_value =~ s/^\s+//;


    if ($ycp_value =~ /^nil(.*)/)
    {
	return (undef, "", $1);
    }
    elsif ($ycp_value =~ /^false(.*)/)
    {
	return (0, "", $1);
    }
    elsif ($ycp_value =~ /^true(.*)/)
    {
	return (1, "", $1);
    }
    # numbers. TODO not only integers: floats
    elsif ($ycp_value =~ /^(-?\d+)(.*)/)
    {
	my $num = $1;
	my $rest = $2;
	$num = oct ($num) if $num =~ /^0/;
	return ($num, "", $rest);
    }
    elsif ($ycp_value =~ /^\"/) #"
    {
	return ParseYcpString ($ycp_value);
    }
    elsif ($ycp_value =~ /^((?:\.${lex_pathsegment})+|\.)(.*)/)
    {
	my $path = $1; # must be a "my" variable, not \$1.
	return (\$path, "", $2);
    }
    elsif ($ycp_value =~ /^\[/)
    {
	return ParseYcpList ($ycp_value);
    }
    elsif ($ycp_value =~ /^\$\[/)
    {
	return ParseYcpMap ($ycp_value);
    }
    elsif ($ycp_value =~ /^\#\[/)
    {
	return ParseYcpByteblock ($ycp_value);
    }
    elsif ($ycp_value =~ /^$/)
    {
	return ("", "Unexpected end of input.", $ycp_value);
    }
    else
    {
	return ("", "Construct not supported.", $ycp_value);
    }
}

# Internal
# Parses a YCP string. The input must start with a double quote.
# Returns PerlYCPParserResult.

# we limit ourselves to parsing the output of YCPStringRep::toString()
# (see YCPString.cc in libycp)
sub ParseYcpString ($)
{
    my $ycp_value = shift;
    my $ret = "";

    #remove the leading quote
    $ycp_value =~ s/^"//; #";

    while (1)
    {
	# ordinary characters
	if ($ycp_value =~ s/^([^\"\\]+)//) #" #newline?
	{
	    $ret .= $1;
	}
	# octal escapes
	elsif ($ycp_value =~ s/^\\([0-7]{3})//)
	{
	    $ret .= chr (oct ($1));
	}
	# weird behavior for 1 or 2 digits
	elsif ($ycp_value =~ s/^\\([0-7]{1,2})//)
	{
	    $ret .= $1;
	}
	# other escapes
	elsif ($ycp_value =~ s/^\\([^0-7])//)
	{
	    if ($1 eq "n")
	    {
		$ret .= "\n";
	    }
	    elsif ($1 eq "t")
	    {
		$ret .= "\t";
	    }
	    elsif ($1 eq "r")
	    {
		$ret .= "\r";
	    }
	    elsif ($1 eq "f")
	    {
		$ret .= "\f";
	    }
	    elsif ($1 eq "b")
	    {
		$ret .= "\b";
	    }
	    elsif ($1 eq "\\")
	    {
		$ret .= "\\";
	    }
	    elsif ($1 eq "\"")
	    {
		$ret .= "\"";
	    }
	    else
	    {
		$ret .= $1;
	    }
	}
	elsif ($ycp_value =~ /^"(.*)/) #");
	{
	    return ($ret, "", $1);
	}
	elsif ($ycp_value =~ /^$/)
	{
	    return ("", "Unexpected end of input.", $ycp_value);
	}
	else
	{
	    #can't happen
	    return ("", "Can't happen in ParseYcpString", $ycp_value);
	}
    }
}

=head2 PathComponents

PathComponents $path_ref

 ($cmd, $path) = ParseCommand ('`Read (.foo."%gconf.d"."gernel")'
 @c = PathComponents (\$path);
 if ($c[0] eq '%gconf.d' && $c[1] eq "gernel") {...}

Converts a path (a string reference, L</PerlYCPValue>) to a list
of its components. It deals with the nontrivial parts of path syntax.
On error it returns undef.

 .			-> ()
 .foo.bar		-> ('foo', 'bar')
 ."foo"			-> ('foo')
 ."double\"quote"	-> ('double"quote')
 ."a.dot"		-> ('a.dot')

=cut

sub PathComponents ($)
{
    my $path_ref = shift;
    if (ref ($path_ref) ne "SCALAR") {
	y2error ("Expecting a reference to a scalar");
	return undef;
    }
    my $path = $$path_ref;

    return undef if $path eq "";
    return () if $path eq ".";

    my @result = ();

    while ($path =~ s/^\.(${lex_pathsegment})(.*)/$2/o) {
	my $segment = $1;
	if ($segment =~ /^"/) {
	    # FIXME check whether paths are like strings, unify
	    my ($parsed, $err, $rest) = ParseYcpString ($segment);
	    if ($err ne "") {
		y2error ("Bad complex path component: '$err'");
		return undef;
	    }
	    elsif ($rest ne "") {
		y2error ("Extra characters in path component: '$rest'");
		return undef;
	    }
	    $segment = $parsed;
	}
	push @result, $segment;
    }
    if ($path ne "") {
	y2error ("Extra characters in path: '$path'");
	return undef;
    }
    return @result;
}

# Internal
# Parses a YCP list. The input must start with "["
# A comma after the last element is permitted.
# Returns PerlYCPParserResult.
sub ParseYcpList ($)
{
    return ParseYcpGenericList (shift, "list", qr/\[/, qr/\]/);
}

# Internal
# Parses a term argument list. The input must start with "("
# A comma after the last element is permitted.
# Returns PerlYCPParserResult.
sub ParseYcpTermBody ($)
{
    return ParseYcpGenericList (shift, "term", qr/\(/, qr/\)/);
}

# Internal
# Parses a comma delimited list introduced by $open and terminated by $close.
# A comma after the last element is permitted.
# Returns PerlYCPParserResult.
sub ParseYcpGenericList ($$$$)
{
    my ($ycp_value, $description, $open, $close) = @_;
    my $ret = [];
    my $elem;
    my $err;

    #remove leading bracket and whitespace;
    if ($ycp_value !~ s/^$open\s*//)
    {
	return ("", "Expecting /$open/ in a $description",$ycp_value);
    }

    my $seen_comma = 0;
    my $seen_elem = 0;

    # if there's a bracket, eat it and return
    until ($ycp_value =~ s/^$close\s*//)
    {
	if ($seen_elem && ! $seen_comma)
	{
	    return ("", "Expecting /$close/ or a comma in a $description",$ycp_value);
	}

	($elem, $err, $ycp_value) = ParseYcp ($ycp_value);
	return ("", $err, $ycp_value) if $err;
	push @{$ret}, $elem;
	$seen_elem = 1;

	# skip spaces and comma
	$ycp_value =~ s/^\s*(,)?\s*//;
	$seen_comma = defined $1;
    }
    return ($ret, "", $ycp_value);
}

# Internal
# Parses a YCP map. The input must start with "$["
# A comma after the last element is permitted.
# Returns PerlYCPParserResult.
sub ParseYcpMap ($)
{
    my $ycp_value = shift;
    my $ret = {};
    my $key;
    my $value;
    my $err;

    #remove leading dollar-bracket and whitespace;
    $ycp_value =~ s/^\$\[\s*//;

    my $seen_comma = 0;
    my $seen_elem = 0;

    # if there's a bracket, eat it and return
    until ($ycp_value =~ s/^\]\s*//)
    {
	if ($seen_elem && ! $seen_comma)
	{
	    return ("", "Expecting a bracket or a comma in a map",$ycp_value);
	}

	($key, $err, $ycp_value) = ParseYcp ($ycp_value);
	return ("", $err, $ycp_value) if $err;

	# skip spaces, match a colon
	if ($ycp_value !~ s/^\s*:\s*//)
	{
	    return ("", "Expecting a colon in a map", $ycp_value);
	}

	($value, $err, $ycp_value) = ParseYcp ($ycp_value);
	return ("", $err, $ycp_value) if $err;

	$ret->{$key} = $value;
	$seen_elem = 1;

	# skip spaces and comma
	$ycp_value =~ s/^\s*(,)?\s*//;
	$seen_comma = defined $1;
    }
    return ($ret, "", $ycp_value);
}

# Internal
# Parses a YCP byteblock. The input must start with "#["
# Returns PerlYCPParserResult.
sub ParseYcpByteblock ($)
{
    my $ycp_value = shift;
    my $ret = "";
    my $err;

    #remove leading hash-bracket and whitespace;
    $ycp_value =~ s/^\#\[\s*//;

    # if there's a bracket, eat it and return
    until ($ycp_value =~ s/^\]\s*//)
    {
	if ($ycp_value =~ s/^(([[:xdigit:]][[:xdigit:]])+)(\s|\n)*//)
	{
	    $ret .= pack ('H*', $1);
	}
	else
	{
	    return ("", "Unexpected characters in byteblock",$ycp_value);
	}
    }
    return ($ret, "", $ycp_value);
}


################################################################################
#                         R E T U R N                                          #
################################################################################
# Function which return a ycp value to the calling YCP-Server

=head1 WRITING

=cut

# Autoflush output, otherwise the caller would not get the answer.
$| = 1;

=head2 Return

C<Return (["arbitrarily", "complex", "data"]);>

Sends a L</PerlYCPValue> to the partner YCP component.

If there's just one argment, scalars are interpreted this way:
"true" or "false" are sent as
booleans, integers or strings of digits are sent as integers, otherwise as
strings.
If a second argument exists and is true, all scalars are written as strings.
If a second argument exists and is false, all scalars are written as byteblocks.

To send a list, call Return(\@list), not Return(@list).
Similarly for a map. You can use references to anonymous lists [] and hashes {}.

The difference from L</ycpReturn> is that Return can return scalars directly,
strings are properly escaped if needeed and paths can be returned.

=cut

sub Return ($;$);
sub Return ($;$)
{
    my ($val, $quote_everything) = @_;

    my $reftype = ref ($val);
    if (! defined ($val))
    {
	print "(nil)";
    }
    elsif (! $reftype)
    {
	if (! defined $quote_everything)
	{
	    if ($val =~ /^(true|false|\s*-?\d+\s*)$/)
	    {
		print "($val)";
	    }
	    else
	    {
		print WriteYcpString($val);
	    }
	}
	elsif ($quote_everything)
	{
	    print WriteYcpString($val);
	}
	else
	{
	    print WriteYcpByteblock($val);
	}
    }
    elsif ($reftype eq "SCALAR")
    {
	# a path
	print "($$val)";
    }
    elsif ($reftype eq "ARRAY")
    {
	print "[";
	foreach my $elem (@$val)
	{
	    Return ($elem, $quote_everything);
	    print ","; # trailing comma is allowed
	}
	print "] "; # no "]:"
    }
    elsif ($reftype eq "HASH")
    {
	print "\$[";
	while (my ($key, $value) = each %$val)
	{
	    Return ($key, $quote_everything);
	    print ":";
	    Return ($value, $quote_everything);
	    print ","; # trailing comma is allowed
	}
	print "] "; # no "]:"
    }
    else
    {
	y2error ("Cannot pass $reftype to YCP");
	print "(nil)";
    }
}

# Internal
# Returns a properly escaped string.
# (Double quotes, backslashes and control characters are handled)
#
# 'qux'				-> '"qux"'
# 'with "quotes"'		-> '"with \"quotes\""'
sub WriteYcpString ($)
{
    my $string = shift;
    # We deal with binary strings (binmode raw), and most code is fine with it.
    # But XML parsers will mark the strings as text, so convert them back.
    # bnc#512536, thanks mls
    # (Note that binmode STDOUT, :utf8 without the same for STDIN would
    # make bnc#448217).
    utf8::encode($string) if utf8::is_utf8($string);

    my @substrings = split /\\/, $string, -1;
    foreach my $substring (@substrings)
    {
	$substring =~ s/"/\\"/g;# escape quotes
	# escape control chars except newline (easier to debug a parse error)
	$substring =~ s/([\000-\011\013-\027])/sprintf "\\%03o",ord($1)/eg;
    }
    return '"'. join ("\\\\", @substrings) .'"';
}

# Internal
# Returns a byteblock.
sub WriteYcpByteblock ($)
{
    my $bb = shift;
    return "#[". unpack ("H*", $bb) ."] ";
}


################################################################################
#                         L O G G I N G                                        #
################################################################################

=head1 LOGGING

If you are running in the main yast process and thus can afford to import
YaST::YCP, it is better to use its logging functions because they use log.conf
and logging just works. In such case, you should not need to use ycp.pm at all.
Instead, C<use YaST::YCP (":LOGGING")>.

The log output can now be redirected, which will be useful for test suites.
If the first command-line option is "-l", the second argument is taken as
the log file. A hyphen "-" designates standard output.

Otherwise, F</var/log/YaST2/y2log> and F<$HOME/.y2log> are tried, in that order.

=cut

my $Y2DEBUG;
my $log_good;

# Constructor: open the log and set the two above variables
# so that y2logger has small overhead.
sub BEGIN
{
    $Y2DEBUG = $ENV{"Y2DEBUG"};
    my $home = $ENV{"HOME"};
    my @names = ( "/var/log/YaST2/y2log" );
    if (defined ($home))
    {
        push(@names, "$home/.y2log")
    }
    if (defined ($ARGV[0]) && $ARGV[0] =~ /^(-l|--log)$/)
    {
	@names = ( $ARGV[1] );
    }

    foreach my $name (@names)
    {
	$log_good = open (LOG, ">>$name");
	if ($log_good)
	{
	    my $old_handle = select (LOG);
	    $| = 1;		# autoflush
	    select ($old_handle);
	    return;
	}
    }

    # no log?! cry to STDERR.
    print STDERR "Could not open log file: '", join("' nor '", @names), "'.\n";
}

sub END
{
    close LOG;
}

##--------------------------------------
# @perlapi y2debug
# Logs debug messages to /var/log/YaST2/y2log.
# Other then ycp-y2debug the output is <b>always</b> logt
# to /var/log/YaST2/y2log
# and usually you <b>have to root</b> to do this
# @example ..;  y2debug( "In the script: param1:", myarray, " param2: ", hash2 );
##--------------------------------------

=head2 y2debug

y2debug,
y2milestone,
y2warning,
y2error,
y2security,
y2internal

Logs debug messages to F</var/log/YaST2/y2log> or F<$HOME/.y2log>

Note a B<semantic change> in y2debug: now the environment variable
Y2DEBUG is honored so y2debug will not produce output unless this
variable is set. This is for compatibility with the logging system in libycp.

=cut

my $log_component = $0;		# use program name as the log component
$log_component =~ s:.*/::;	# strip path part

sub y2debug	{ y2logger (0, @_); }
sub y2milestone	{ y2logger (1, @_); }
sub y2warning	{ y2logger (2, @_); }
sub y2error	{ y2logger (3, @_); }
sub y2security	{ y2logger (4, @_); }
sub y2internal	{ y2logger (5, @_); }

# Internal
sub y2logger ($@)
{
    my $level = shift;
    if (!$log_good || ($level == 0 && ! defined ($Y2DEBUG)))
    {
	return;
    }

    my $tm = localtime;
    my $datestr = sprintf( "%04d-%02d-%02d %02d:%02d:%02d <%d> %s(%d) [%s]",
			   $tm->year+1900, $tm->mon+1, $tm->mday,
			   $tm->hour, $tm->min, $tm->sec,
			   $level, $hostname, $$, $log_component);

    print LOG "$datestr ", join(" ", @_), "\n";
}

##--------------------------------------
# @perlapi ycpDoVerboseLog
# Turns on verbose logging of this the perl interface lib
# Logging output is ALWAYS send to /var/log/YaST2/y2log
# and usually you <b>have to be root</b> to do this
# @example ..;  ycpDoVerboseLog;
##--------------------------------------

=head2 ycpDoVerboseLog

Enables output of y2verbose which is used in some of the obsolete functions.

=cut

sub ycpDoVerboseLog
{
   $verbose = 1;
}

# Internal
sub y2verbose
{
    if ( $verbose )
    {
	y2debug( @_ );
    }
}


################################################################################
# Old functions
################################################################################

=head1 OBSOLETE FUNCTIONS

=cut

#########################################
## Check type of the given Argument
#########################################
#

##--------------------------------------
# @perlapi ycpArgIsMap -> 0 or 1
# Checks, if the given argument is a map.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsMap ) \n { my %arg_hash =  ycpGetArgMap;  }
##--------------------------------------

=head2 ycpArgIsMap

Obsolete. Use (ref($arg) eq "HASH") instead.

=cut

sub ycpArgIsMap
{
   return( $ismap );
}


##--------------------------------------
# @perlapi ycpArgIsList -> 0 or 1
# Checks, if the given argument is a list.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsList ) \n { my @arg_array = ycpGetArgList;  }
##--------------------------------------

=head2 ycpArgIsList

Obsolete. Use (ref($arg) eq "ARRAY") instead.

=cut

sub ycpArgIsList
{
   return( $islist );
}

##--------------------------------------
# @perlapi ycpArgIsInteger -> 0 or 1
# Checks, if the given argument is a Integer.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsInteger ) \n { my $arg_int =  ycpGetArgInteger; }
##--------------------------------------

=head2 ycpArgIsInteger

Not really obsolete because the new parser simply treats
integers, booleans and strings as scalars. But who cares,
nobody used this anyway.

=cut

sub ycpArgIsInteger
{
   return( $isinteger );
}

##--------------------------------------
# @perlapi ycpArgIsString -> 0 or 1
# Checks, if the given argument is a String.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsString ) \n { my $new_string =  ycpGetArgString; }
##--------------------------------------

=head2 ycpArgIsString

Not really obsolete because the new parser simply treats
integers, booleans and strings as scalars. But who cares,
nobody used this anyway.

=cut

sub ycpArgIsString
{
   return( $isstring );
}

##--------------------------------------
# @perlapi ycpArgIsNil -> 0 or 1
# Checks, if the given argument is a nil.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsNil ) \n { ... }
##--------------------------------------

=head2 ycpArgIsNil

Obsolete. Use (ref($arg) eq "SCALAR" && $$arg eq "nil") instead.

=cut

sub ycpArgIsNil
{
   return( $isnil );
}

##--------------------------------------
# @perlapi ycpArgIsNone -> 0 or 1
# Checks, if the given argument is a None.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsNone ) \n { ... }
##--------------------------------------

=head2 ycpArgIsNone

Obsolete. Use (defined ($arg)) instead.

=cut

sub ycpArgIsNone
{
   return( $isnone );
}

##--------------------------------------
# @perlapi ycpCommandIsDir -> 0 or 1
# Checks, if the given command is a <tt>Dir</tt>.
# requirements: a call of  ycpInit()
# @example if ( ycpCommandIsDir ) \n { ... }
##--------------------------------------

=head2 ycpCommandIsDir

Obsolete. Use ($command eq "Dir")

=cut

sub ycpCommandIsDir
{
   return( $ycpcommand =~ /Dir/i );
}

##--------------------------------------
# @perlapi ycpCommandIsRead -> 0 or 1
# Checks, if the given command is a <tt>Read</tt>.
# requirements: a call of  ycpInit()
# @example if ( ycpCommandIsRead ) \n { ... }
##--------------------------------------

=head2 ycpCommandIsRead

Obsolete. Use ($command eq "Read")

=cut

sub ycpCommandIsRead
{
   return( $ycpcommand =~ /Read/i );
}

##--------------------------------------
# @perlapi ycpCommandIsWrite -> 0 or 1
# Checks, if the given command is a <tt>Write</tt>.
# requirements: a call of  ycpInit()
# @example if ( ycpCommandIsWrite ) \n { ... }
##--------------------------------------

=head2 ycpCommandIsWrite

Obsolete. Use ($command eq "Write")

=cut

sub ycpCommandIsWrite
{
   return( $ycpcommand =~ /Write/i );
}

##--------------------------------------
# @perlapi ycpCommandIsExecute -> 0 or 1
# Checks, if the given command is a <tt>Execute</tt>.
# requirements: a call of  ycpInit()
# @example if ( ycpCommandIsExecute ) \n { ... }
##--------------------------------------

=head2 ycpCommandIsExecute

Obsolete. Use ($command eq "Execute")

=cut

sub ycpCommandIsExecute
{
   return( $ycpcommand =~ /Execute/i );
}

##--------------------------------------
# @perlapi ycpCommandIsResult -> 0 or 1
# Checks, if the given command is a <tt>result</tt>.
# requirements: a call of  ycpInit()
# @example if ( ycpCommandIsResult ) \n { ... }
##--------------------------------------

=head2 ycpCommandIsResult

Obsolete. Use ($command eq "result"), note the lowercase 'r'.

=cut

sub ycpCommandIsResult
{
   return( $ycpcommand =~ /result/i );
}


########################################
# Return the argument, converted to perl
# datatype
########################################


##--------------------------------------
# @perlapi ycpGetCommand -> "Read" or "Write" or "Execute" or "Dir"
# Returns the current command.
# requirements: a call of  ycpInit()
##--------------------------------------

=head2 ycpGetCommand

Obsolete. Use the return value of L</ParseCommand>.

=cut

sub ycpGetCommand
{
   return( $ycpcommand );
}

##--------------------------------------
# @perlapi ycpGetPath -> <string>
# Returns the current <b>sub</b>path of the current call.
# If the script is mounted on <tt>.ping</tt> and the agent is called
# with <tt>.ping.suse</tt> the subpath is <tt>.suse</tt>
# requirements: a call of  ycpInit()
##--------------------------------------

=head2 ycpGetPath

Obsolete. Use the return value of L</ParseCommand>.

=cut

sub ycpGetPath
{
   return( $ycppath );
}

##--------------------------------------
# @perlapi ycpGetArgType -> <string>
# Returns the type of the current argument.
# At the moment "string", "integer", "list" and "map" are supported.
# requirements: a call of  ycpInit()
##--------------------------------------

=head2 ycpGetArgType

Obsolete. Use ref on a return value of L</ParseCommand>.

Umm, string/integer/boolean?

=cut

sub ycpGetArgType
{
   return( $type );
}

##--------------------------------------
# @perlapi ycpGetArgMap -> <hash>
# Returns the curren argument as a hash, if the argument was a map.
# Otherwise the return value is not defined.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsMap ) \n { my %arg_hash =  ycpGetArgMap;  }
##--------------------------------------

=head2 ycpGetArgMap

Obsolete. See L</PerlYCPValue>.

=cut

sub ycpGetArgMap
{
   return( %arghash );
}

##--------------------------------------
# @perlapi ycpGetArgList -> <array>
# Returns the current argument as an arry, if the argument was a list.
# Otherwise the return value is not defined.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsList ) \n { my @arg_array =  ycpGetArgList;  }
##--------------------------------------

=head2 ycpGetArgList

Obsolete. See L</PerlYCPValue>.

=cut

sub ycpGetArgList
{
   return( @argarray );
}

##--------------------------------------
# @perlapi ycpGetArgString -> <string>
# Returns the current argument as a string, if the argument was a string
# Otherwise the return value is not defined.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsString ) \n { my $arg_string =  ycpGetArgString;  }
##--------------------------------------

=head2 ycpGetArgString

Obsolete. See L</PerlYCPValue>.

=cut

sub ycpGetArgString
{
   return( $argskalar );
}

##--------------------------------------
# @perlapi ycpGetArgInteger -> <integer>
# Returns the current argument as an integer, if the argument was an integer
# Otherwise the return value is not defined.
# requirements: a call of  ycpInit()
# @example if ( ycpArgIsInteger ) \n { my $arg_string =  ycpGetArgInteger;  }
##--------------------------------------

=head2 ycpGetArgInteger

Obsolete. See PerlYCPValue.

Umm, string/integer/boolean?

=cut

sub ycpGetArgInteger
{
   return( $argskalar );
}


# OBSOLETE WRITING


##--------------------------------------
# @perlapi ycpReturnSkalarAsInt
# Sends a scalar as a YCP-Integer to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function-
# @example ycpReturnSkalarAsInt( 17 ) -> Returns a
##--------------------------------------

=head2 ycpReturnSkalarAsInt

Obsolete. Use L</Return>.

=cut

sub ycpReturnSkalarAsInt( $ )
{
    my ( $entry ) = @_;

    printf( "(%d)", $entry );
}

##--------------------------------------
# @perlapi ycpReturnSkalarAsBoolean
# Sends a scalar as a YCP-Boolean to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function-
# @example ycpReturnSkalarAsBoolean( 1 ) -> Returns a true
# @example ycpReturnSkalarAsBoolean( 0 ) -> Returns a false
##--------------------------------------

=head2 ycpReturnSkalarAsBoolean

Obsolete. Use L</Return>("true" or "false")

=cut

sub ycpReturnSkalarAsBoolean( $ )
{
    my ( $entry ) = @_;

    printf( "(%s)", $entry ? "true" : "false");
}

##--------------------------------------
# @perlapi ycpReturnSkalarAsString
# Sends a scalar as a YCP-String to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function
# @example ycpReturnSkalarAsString( "ok" )
##--------------------------------------

=head2 ycpReturnSkalarAsString

Obsolete. Works only on strings not containing backslashes and quotes
that would need escaping.

Use L</Return>.

=cut

sub ycpReturnSkalarAsString( $ )
{
    my ( $entry ) = @_;
    y2verbose( "assk: ", $entry );
    printf( "(\"%s\")", $entry );
}

##--------------------------------------
# @perlapi ycpReturnArrayAsList
# Sends a array as a YCP-List to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function
# @example my @array = ( 1,2,3); ycpReturnSkalarAsString( array )
##--------------------------------------

=head2 ycpReturnArrayAsList

Obsolete. Works only on list of strings not containing backslashes and quotes
that would need escaping.

Use L</Return>.

=cut

sub ycpReturnArrayAsList( @ )
{
    my ( @entry ) = @_;

    # starting the List
    printf ( "[ " );

    if ( @entry > 0 )
    {
	printf( "\"%s\"", shift( @entry ));

	foreach my $elem ( @entry )
	{
	   printf( ", \"%s\"", $elem);
	}
    }

    # end of list
    printf ( " ] " ); # no "]:"
}


##--------------------------------------
# @perlapi ycpReturnHashAsMap
# Sends a hash as a YCP-List to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function
# @example my %myhash; ...; ycpReturnSkalarAsString( myhash );
##--------------------------------------

=head2 ycpReturnHashAsMap

Obsolete. Works only on maps of strings not containing backslashes and quotes
that would need escaping.

Use L</Return>.

=cut

sub ycpReturnHashAsMap
{
    my ( %entry ) = @_;

    my $first = 1;

    # starting the Map
    printf ( " \$\[ " );


    foreach my $key (keys (%entry))
    {
	if ( $first )
	{
	    $first = 0;
	}
	else
	{
	    printf( ", " );
	}

	printf( "\"%s\":\"%s\"", $key, $entry{$key});
    }

    printf ( " ] " ); # no "]:"
}


##--------------------------------------
# @perlapi ycpReturnSkalarAsIntSub
# Private function to return a ycp-formatted int.
##--------------------------------------

sub ycpReturnSkalarAsIntSub( $ )
{
    my ( $entry ) = @_;

    return sprintf( "(%d)", $entry );
}

##--------------------------------------
# @perlapi ycpReturnSkalarAsBooleanSub
# Private function to return a ycp-formatted boolean.
##--------------------------------------

sub ycpReturnSkalarAsBooleanSub( $ )
{
    my ( $entry ) = @_;

    return sprintf( "(%s)", $entry ? "true" : "false");
}



##--------------------------------------
# @perlapi ycpReturnSkalarAsStringSub
# Private function to return a ycp-formatted string.
##--------------------------------------

sub ycpReturnSkalarAsStringSub( $ )
{
    my ( $entry ) = @_;
    y2verbose( "assk: ", $entry );
    return sprintf( "(\"%s\")", $entry );
}


##--------------------------------------
# @perlapi ycpReturnArrayAsListSub
# Sends a array as a YCP-List to the calling server, the SCR
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function
# @example my @array = ( 1,2,3); ycpReturnSkalarAsString( array )
##--------------------------------------

sub ycpReturnArrayAsListSub( @ )
{
    my ( @entry ) = @_;
    my $ret;

    # starting the List
    $ret =  "[ ";

    if ( @entry > 0 )
    {
	$ret .= sprintf( "\"%s\"", shift( @entry ));

	foreach my $elem ( @entry )
	{
	   $ret .= sprintf( ", \"%s\"", $elem);
	}
    }

    # end of list
    $ret .= " ] "; # no "]:"
    return $ret;
}

##--------------------------------------
# @perlapi ycpReturn
# sends a complex data structure to the calling server, the SCR.
# Attention: It is very important to send in <b>ANY</b> case
# exactly one YCP value to the server. So you must take care, that
# for every call of the server, your script calls exactly one ycpReturn<..>
# function
# The complex data structure is build in perl using references. A perl array
# is transformed to a ycp list, a perl hash will become a ycp map and a scalar
# will be a simple string, integer or boolean.
# This function takes one refernce to one of the mentioned data
# types. It easily can be a refernce to a tree of refernces to all valid data types.
# For example: To build a map, containing a list, in perl, you do:
#
# @example my @list_of_values = ( 'l1', 'l2', 'l3', 'l4' );
# @example my %maphash = { "key1" => "value1", "key2" => \@list_of_values };
# @example ycpReturn( \%maphash );
##--------------------------------------

=head2 ycpReturn

Obsolete. Use L</Return>

=cut

sub ycpReturn
{
    print ycpReturnSub(@_);
}

##--------------------------------------
# @perlapi ycpReturnSub
# private function to provide the functionality for ycpReturn
##--------------------------------------
sub ycpReturnSub
{
    my ($itemref) = @_;
    my $ret = "";
    unless( ref($itemref) ) {
	# Error: Not a reference at all !
	# Was tun ?
	return "[] "; # no "]:"
    }

    if( ref( $itemref ) eq "SCALAR" )
    {
	$ret = formatItem( $$itemref );
    }
    elsif( ref( $itemref ) eq "ARRAY" )
    {
	my $docomma = 0;
	my $list = "[ ";
	foreach my $item ( @$itemref )
	{
	    my $append = "";
	    if( $docomma ) {
		$append .= ", ";
	    } else {
		$docomma = 1;
	    }

	    if( ref( $item ) )
	    {
		$append .= &ycpReturnSub( $item );
	    }
	    else
	    {
		$append .= formatItem( $item );
	    }
	    $list .= $append;
	}
	$list .= " ] "; # no "]:"
	$ret = $list;
    }
    elsif( ref( $itemref ) eq "HASH" )
    {
	my $docomma = 0;
	$ret = "\$\[ ";
	my $oneMapItem;
	while ( my ($key, $value) = each %$itemref )
	{
	    if( ref( $key ) )
	    {
		# Error - darf keine referenz sein, oder ?
	    }
	    else
	    {
		if( $docomma )
		{
		    $ret .= ", ";
		}
		else
		{
		    $docomma = 1;
		}

		$oneMapItem = formatItem( $key ) . ":";
		my $expanded = "";
		if( ref( $value ) )
		{
		    $expanded = &ycpReturnSub( $value );
		}
		else
		{
		    $expanded = formatItem( $value );
		}
		$oneMapItem .= $expanded;
	    }
	    $ret .= $oneMapItem;
	}
	$ret .= " \] "; # no "]:"
    }
    return $ret;
}

##--------------------------------------
# @perlapi formatItem
# private function that formats exactly one item according to it's type.
# It handles boolean, integers and Strings.
# needed by ycpReturn, this function is not exported.
##--------------------------------------
sub formatItem( $ )
{
    my ($item) = @_;

    # Format boolean, must be true or false literally.
    if( $item =~ /true|false/i )
    {
	return sprintf( "%s", lc $item );
    }

    # Format integers
    if( $item =~ /^\s*\d+\s*$/ )
    {
	return( sprintf( "%d", $item ) );
    }

    # Format strings
    return( sprintf( "\"%s\"", $item ));
}


# OBSOLETE PARSING

##############################
# String or Integer or Boolean to Skalar
##############################

sub ycpSIBtoSkalar
{
    my ( $entry ) = @_;
    if( $entry =~ /^\s*\$\[.*\]\s*$/ )
    {
	# It is unintentional a map -> not handled yet -> TODO
	$entry = "Unhandled";
    }
    elsif ( $entry =~ /^\s*\"(.*)\"\s*$/ )
    {
	# "
	$entry = $1;
    }
    else
    {
	if ( $entry =~ /^\s+(.*)$/ )
	{
	    $entry = $1;
	}
	if ( $entry =~ /^(.*?)\s+$/ )
	{
	    $entry = $1;
	}

    }

    return( $entry );
}

sub ycpmap
{
    my @input = @_;

    chop @input if( $input[0] =~ /\n$/ );

    my $wholeline = join( "", @input );

    my %result;

    if( $wholeline =~ /^\s*\$\[(.+)\]\s*$/ )
    {
	my $mapcont = $1;
	# Killing all lists etc inside the map. TODO !!!!
	$mapcont =~ s/\$\[.+\]//g;

	my @mapentries = split( /\s*,\s*/, $mapcont );

	foreach my $mentry ( @mapentries )
	{
	    if( $mentry =~ /(.+?):(.+)$/ )
	    {
		# " fill the hash
		my $key = ycpSIBtoSkalar($1);
		my $val = ycpSIBtoSkalar($2);
		$result{$key} = $val;

		y2verbose( "new hashentry key:-$key- Value:-$val- ");
	    }
	}
    }
    return( %result );
}

sub ycplist
{
    my @input = @_;

    chop @input if( $input[0] =~ /\n$/ );
    my $wholeline = join( "", @input );

    my @result;

    if( $wholeline =~ /^\s*\[(.+)\]\s*$/ )
    {
	my $mapcont = $1;

	my @mapentries = split( /\s*,\s*/, $mapcont );

	foreach my $mentry ( @mapentries )
	{
	    $mentry = ycpSIBtoSkalar( $mentry );

	    push( @result, $mentry );
	    y2verbose( "listentry: -$mentry-");
	}

	@result = @mapentries;
    }
    return( @result );
}

##--------------------------------------
# @perlapi ycpInit
# Initializes a call of the server. Therefor this function reads a YCP Value
# from <tt>stdin</tt>. To use this value, call the convenice functions:
# ycpArgIs<..>, ycpGetArg<..>, ycpCommandIs<..>, ycpGetCommand, ycpGetPath
# As parameter, you have to insert $_
# @example ycpInit( $_ );
##--------------------------------------

=head2 ycpInit

Obsolete. Use L</ParseCommand>.

=cut

sub ycpInit
{
    my @input = @_;

    chop @input if( $input[0] =~ /\n$/ );

    my $wholeline = join( "", @input );

    y2verbose( "call: <", $wholeline, ">" );

    my %result;

    if( $wholeline =~ /^(.*)\((.*?),(.*)\)\s*$/)
    {
	y2verbose( "work: <", $wholeline, ">" );

	$ycpcommand    = $1;
	$ycppath       = $2;
	my $parameter  = $3;

	y2verbose( "com:", $ycpcommand, " path:", $ycppath, " param:", $parameter);

	$type     = "unknown";

	$ismap     = 0;
	$islist    = 0;
	$isinteger = 0;
	$isstring  = 0;
	$isknown   = 0;
	$isnil     = 0;
	$isnone    = 0;


	# is a map?
	if( $parameter =~ /^\s*\$\[.*\]\s*$/ )
	{
	    y2verbose( "Is a map", $parameter );
	    %arghash  = ycpmap(  $parameter );
	    $ismap    = 1;
	    $type     = "map";
	}

	# is a list?
	elsif( $parameter =~ /^\s*\[.*\]\s*$/ )
	{
	    y2verbose(   "Is a list", $parameter);
	    @argarray  = ycplist( $parameter );
	    $islist    = 1;
	    $type      = "list";
	}

	# is a integer?
	elsif( $parameter =~ /^\s*([0-9]+)\s*$/ )
	{
	    y2verbose(   "Is a integer", $parameter);
	    $argskalar = $1;
	    $isinteger = 1;
	    $type      = "integer";
	}

	# is a string?
	elsif( $parameter =~ /^\s*\"(.*)\"\s*$/ )
	{
	    y2verbose(   "Is a string", $parameter);
	    $argskalar = $1;
	    $isstring  = 1;
	    $type      = "string";
	}

	# is a string?
	elsif( $parameter =~ /^\s*nil\s*$/ )
	{
	    y2verbose(   "Is a nil", $parameter);
	    $isnil     = 1;
	    $type      = "nil";
	}
	else
	{
	    $isknown = 1;
	    $type    = "unknown";
	}
    }
    elsif ($wholeline =~ /^(.*)\((.*?)\)\s*$/) {

	y2verbose( "work: <", $wholeline, ">" );

	$ycpcommand    = $1;
	$ycppath       = $2;

	y2verbose( "com:", $ycpcommand, " path:", $ycppath, " param: none" );

	$type     = "unknown";

	$ismap     = 0;
	$islist    = 0;
	$isinteger = 0;
	$isstring  = 0;
	$isknown   = 0;
	$isnil     = 0;
	$isnone    = 1;

    }
    return( %result );
}

1;

################################## EOF ########################################
