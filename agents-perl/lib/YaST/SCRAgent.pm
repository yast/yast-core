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

sub Read ()
{
    my $class = shift;
    y2error ("Read not implemented in this agent");
    return undef;
}

sub Write ()
{
    my $class = shift;
    y2error ("Write not implemented in this agent");
    return 0;
}

sub Dir ()
{
    my $class = shift;
    y2error ("Dir not implemented in this agent");
    return [];
}

sub Execute ()
{
    my $class = shift;
    y2error ("Execute not implemented in this agent");
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
		    # call it
		    my $ret = $class->$command($path, @arguments);
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
	    my $ret = $class->OtherCommand ($command, @arguments);
	    ycp::Return ($ret, 1);
	}

	print "\n";
    }
    y2milestone ("Agent $class finished");
}

# indicate correct initialization
1;
