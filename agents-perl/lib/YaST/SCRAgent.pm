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

sub OtherCommand ()
{
    my $class = shift;
    my $command = shift;
    y2error ("OtherCommand ($command) not implemented in this agent");
    return undef;
}

sub Run ()
{
    my $class = shift;

    y2milestone ("Agent $class started");
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
	elsif ($command =~ m/^Read|Write|Dir|Execute$/)
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
		    # try if the sub $command_$path exists
		    my $sym = "$command$path";
		    $sym =~ s/\./_/g;
		    # UNIVERSAL::can
		    if ($class->can ($sym))
		    {
			# call it without the path
			$ret = $class->{$sym}(@arguments);
		    }
		    else
		    {
			no strict; # fatal error otherwise about wrong types

			# try if we can Read/Write an "our" variable directly
			# test if all this really works
			my $can_var = 0;

			# get the symbol table of the package
			*stash = *{"${class}::"};
			if (exists $stash{$sym})
			{
			    *symglob = $stash{$sym};
			    if ($command eq "Read")
			    {
				$can_var = 1;
				if (defined (%symglob))
				{
				    $ret = \%symglob;
				}
				elsif (defined (@symglob))
				{
				    $ret = \@symglob;
				}
				elsif (defined ($symglob))
				{
				    $ret = $symglob;
				}
				else
				{
				    y2error ("Not a scalar/array/hash: $sym");
				    $can_var = 0;
				}
			    }
			    elsif ($command eq "Write")
			    {
				# todo test if arg exists
				my $arg = $arguments[0];
				$can_var = 1;

				if (defined (%symglob))
				{
				    %symglob = %{$arg};
				    $modified = 1;
				}
				elsif (defined (@symglob))
				{
				    @symglob = @{$arg};
				    $modified = 1;
				}
				elsif (defined ($symglob))
				{
				    $symglob = $arg;
				    $modified = 1;
				}
				else
				{
				    y2error ("Not a scalar/array/hash: $sym");
				    $can_var = 0;
				}
			    }
			}

			if (!$can_var)
			{
			    # call it the ordinary way
			    $ret = $class->$command($path, @arguments);
			}
		    }
		    if ($command eq "Write")
		    {
			$ret = ycp::to_bool ($ret);
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
    y2milestone ("Agent $class finished");
}

# indicate correct initialization
1;
