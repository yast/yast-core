# Author: Martin Vidner, mvidner@suse.cz
# $Id$

=head1 NAME

YaST::SCRAgent - a class for stdio-communicating YaST agents.

=head1 SYNOPSIS

  package my_agent;
  use YaST::SCRAgent;
  our @ISA = ("YaST::SCRAgent");
  sub Read { if ($path eq ".foo") { ... return ["a", "b"];} }
  sub Write { ... return 1; }

  package main;
  my_agent->Run;

=cut

package YaST::SCRAgent;

use strict;
use ycp;
#use Devel::Peek 'Dump';

# used when Write directly modifies a variable
our $modified = 0;

sub Read ()
{
    my ($class, $path, @rest) = @_;
    y2error ("Read not implemented in this agent for $path");
    return undef;
}

sub Write ()
{
    my ($class, $path, @rest) = @_;
    y2error ("Write not implemented in this agent for $path");
    return 0;
}

sub Dir ()
{
    my ($class, $path, @rest) = @_;
    y2error ("Dir not implemented in this agent for $path");
    return [];
}

sub Execute ()
{
    my ($class, $path, @rest) = @_;
    y2error ("Execute not implemented in this agent for $path");
    return undef;
}

my %__error = ();

# used by the agent
sub SetError {
    my $class = shift;
    %__error = @_;
    if( !$__error{package} && !$__error{file} && !$__error{line})
    {
        @__error{'package','file','line'} = caller();
    }
    if ( defined $__error{summary} )
    {
        y2error($__error{code}." ".$__error{summary});
    } else {
        y2error($__error{code});
    }
    return undef;
}

# SCR::Error
sub Error {
    my ($class, $path, @rest) = @_;
    return \ %__error;
}

sub OtherCommand ()
{
    my $class = shift;
    my $command = shift;
    y2error ("OtherCommand ($command) not implemented in this agent");
    return undef;
}

# try if the sub $command_$path exists
# returns ($ret, $ok)
sub TryCommandPath ()
{
    my $class = shift;
    my ($command, $path, @arguments) = @_;

    my $sym = "$command$path";
    $sym =~ s/\./_/g;

    no strict;

    # get the symbol table of the package
    local *stash = *{"${class}::"};

    if (exists $stash{$sym})
    {
	*symglob = $stash{$sym};
	if (defined (&symglob))
	{
	    y2debug "TryCommandPath 1";
	    return (&symglob (@arguments), 1);
	}
    }
    y2debug "TryCommandPath 0";
    return (undef, 0);
}

# try if the variable exists
# returns ($ret, $ok)
sub TryPathVariable ()
{
    my $class = shift;
    my ($command, $path, @arguments) = @_;

    my $sym = $path;
    $sym =~ s/^\.//;
    $sym =~ s/\./_/g;

    my ($ret, $ok) = (undef, 0);
    no strict;

    # get the symbol table of the package
    local *stash = *{"${class}::"};

    if (exists $stash{$sym})
    {
	*symglob = $stash{$sym};
	# try if we can Read/Write an "our" variable directly
	# test if all this really works

	if ($command eq "Read")
	{
#	    Dump *symglob;

	    $ok = 1;
	    # see *foo{THING} in perlref
	    if (*symglob{HASH})
	    {
		$ret = *symglob{HASH};
	    }
	    elsif (*symglob{ARRAY})
	    {
		$ret = *symglob{ARRAY};
	    }
	    # for scalars, it _creates_ an anonymous one, can't use it
	    elsif (*symglob{CODE} || *symglob{IO})  # GLOB also defined :(
	    {
		y2error ("Is a sub or a filehandle: $sym");
		$ok = 0;
	    }
	    else #SCALAR
	    {
		$ret = $symglob;
	    }
	}
	elsif ($command eq "Write")
	{
	    # todo test if arg exists
	    my $arg = $arguments[0];

	    if (*symglob{HASH})
	    {
		if (ref ($arg) eq "HASH")
		{
		    %symglob = %{$arg};
		    $modified = $ok = 1;
		}
		else
		{
		    y2error ("Cannot assign this to a hash: ", ref ($arg) || $arg);
		}
	    }
	    elsif (*symglob{ARRAY})
	    {
		if (ref ($arg) eq "ARRAY")
		{
		    @symglob = @{$arg};
		    $modified = $ok = 1;
		}
		else
		{
		    y2error ("Cannot assign this to an array: ", ref ($arg) || $arg);
		}
	    }
	    elsif (*symglob{CODE} || *symglob{IO}) # GLOB also defined :(
	    {
		y2error ("Is a sub or a filehandle: $sym");
		$ok = 0;
	    }
	    else #SCALAR
	    {
		if (! ref ($arg))
		{
		    $symglob = $arg;
		    $modified = $ok = 1;
		}
		else
		{
		    y2error ("Cannot assign this to a scalar: ", ref ($arg));
		}
	    }
	    $ret = $ok;
	}
    }
    y2debug "TryPathVariable $ok";
    return ($ret, $ok);
}

sub Run ()
{
    my $class = shift;

    y2debug ("Agent $class started");
    while ( <STDIN> )
    {
	chomp;
	y2debug ("Got: ", $_);
	if (/^nil$/)
	{
	    print "nil\n";
	    next;
	}

	my ($command, @arguments) = ycp::ParseTerm ($_);
	my $ret;

	if ($command eq "result")
	{
	    return;
	}
	elsif ($command =~ m/^Read|Write|Dir|Execute|Error$/)
	{
	    # Standard commands, they have a path as the first argument
	    # Convert it to a string

	    # future enhancement:
	    # directly call Read_services_data for Read(.services.data)
	    # (exact match, no Read_services)
	    # or read the variable of the same name

	    my $pathref = shift @arguments;
	    if (!defined $pathref)
	    {
		y2error ("Missing path argument to $command");
	    }
	    else
	    {
		if (ref($pathref) ne "SCALAR" || $$pathref !~ /^\./)
		{
		    y2error ("The first argument is not a path. ('$pathref')");
		}
		else
		{
		    my $path = $$pathref;
		    my $ok;

		    ($ret, $ok) = $class->TryCommandPath ($command, $path, @arguments);
		    if (!$ok)
		    {
			($ret, $ok) = $class->TryPathVariable ($command, $path, @arguments);
			if (!$ok)
			{
			    # call it the ordinary way
			    $ret = $class->$command($path, @arguments);
			}
		    }

		    if ($command eq "Write")
		    {
			$ret = ($ret ? \ "true" : \ "false");
		    }
		    ycp::Return ($ret, 1);
		}
	    }
	}
	else
	{
	    $ret = $class->OtherCommand ($command, @arguments);
	    ycp::Return ($ret, 1);
	}

	print "\n";
    }
    y2debug ("Agent $class finished");
}

# indicate correct initialization
1;
