#!/usr/bin/perl -w

# Gather allocation stats from memory debugging output. The debugging
# information is produced by the R-2.1.1rcc interpreter or
# RCC-compiled code if the R_DUMP_STATS environment variable is
# defined.
#
# author: John Garvin

use strict;
use warnings;

my %objects;  # key = pointer of allocated object
              # value = (output string,
              #          stack depth on allocation,
              #          minimum stack depth to which object may escape)

my @stack;
my $dup = 0;
my $in_env = 0;  # whether objects currently being allocated are part
		 # of an environment (environment allocation takes
		 # place after entering a procedure until allocation
		 # the actual environment object)
my $debug = 0;

my %stats = ('alloc'       => 0,    # total objects allocated
             'heap'        => 0,    # objects allocated on heap
             'stack'       => 0,    # objects allocated on stack
             'dup'         => 0,    # objects allocated by duplicate()
	     'escape'      => 0,    # objects that escape (returned or assigned)
	     'assigns'     => 0,    # number of nonlocal assignments that occur
	     'returns'     => 0,    # number of procedure returns that occur
	     'p_assigned'  => 0,    # inc. for each pointer escaping by nonlocal assignment
	     'p_returned'  => 0,    # inc. for each pointer escaping by return

	     'heap_cons'        => 0,    # objects allocated by cons()
	     'heap_vector'      => 0,    # objects allocated by allocVector()
	     'heap_SEXP'        => 0,    # objects allocated by allocSExp()
	     'heap_promise'     => 0,    # objects allocated by mkPROMISE()
	     'heap_environment' => 0,    # objects allocated by NewEnvironment()
	     'heap_list'        => 0,    # objects allocated by allocList()

	     'stack_cons'        => 0,    # objects allocated by cons()
	     'stack_vector'      => 0,    # objects allocated by allocVector()
	     'stack_SEXP'        => 0,    # objects allocated by allocSExp()
	     'stack_promise'     => 0,    # objects allocated by mkPROMISE()
	     'stack_environment' => 0,    # objects allocated by NewEnvironment()
	     'stack_list'        => 0,    # objects allocated by allocList()

	     'unknown'           => 0,    # objects where we don't know how they got allocated
	     
	     'in_env'      => 0,    # objects that are part of an
				    # environment (both cons cells and
				    # environment objects)
	     'parent'      => 0,    # at least once moved to parent
				    # pool due to lack of space
	     
	     'gc0' => 0,    # times garbage collection has occurred, for each generation
	     'gc1' => 0,
	     'gc2' => 0,
    );

sub min {
    my $x = shift;
    my $y = shift;
    if ($x < $y) {
	return $x;
    } else {
	return $y;
    }
}

while (<>) {
    my @words = split();

    if (/^Alloc:/) {
	$stats{alloc}++;
	if ($in_env) {
	    $stats{in_env}++;
	}
	my $ptr = $words[-1];
	if ($dup > 0) {
	    $stats{dup}++;
	}
	if ($_ =~ 'environment') {
	    $in_env = 0;
	}
	if (defined($objects{$ptr})) {
	    my ($v0, $v1, $v2) = @{ $objects{$ptr} };
	    if ($debug) {
		print("pushing (a): ");
		print($ptr, "\n");
		print($v0);
		print($v1, "\n");
		print($v2, "\n");
	    }
	    analyze_line($v0, $v1, $v2);
	}
	$objects{$ptr} = [$_, scalar(@stack), scalar(@stack)];
    } elsif (/^Entering/) {
	push(@stack, $words[-1]);
	$in_env = 1;
    } elsif (/^Returning/) {
	pop(@stack);
	$stats{returns}++;
	if (scalar(@words) > 6) {
	    my $ptr;
	    foreach $ptr (@words[6,]) {
		# returned pointers may be used in caller
		if (!defined($objects{$ptr})) { die("pointer $ptr not found: $!\n"); }
		$stats{p_returned}++;
		$objects{$ptr}[2] = min($objects{$ptr}[2], scalar(@stack));
	    }
	}
    } elsif (/^Duplicating/) {
	# meaning: following alloc(s) due to duplicate()
	$dup++;
    } elsif (/^End of duplication/) {
	$dup--;
    } elsif (/^Nonlocal/) {
	$stats{assigns}++;
	if (scalar(@words) > 4) {
	    my $ptr;
	    foreach $ptr (@words[4, ]) {
		if (!defined($objects{$ptr})) { die("pointer $ptr not found: $!\n"); }
		$stats{p_assigned}++;
		$objects{$ptr}[2] = 0;
	    }
	}
    } else {
	# no-op
    }
}

foreach my $ptr (keys(%objects)) {
    my ($v0, $v1, $v2) = @{ $objects{$ptr} };
    if ($debug) {
	print("pushing (b): ");
	print($v0);
	print($v1, "\n");
	print($v2, "\n");
    }
    analyze_line($v0, $v1, $v2);
}

sub analyze_line {
    my ($str, $alloc_depth, $esc_depth) = @_;
    if ($alloc_depth == $esc_depth) {
    } else {
	$stats{escape}++;
    }
    if ($str =~ 'parent') {
	$stats{parent}++;
    }

    if ($str =~ 'GC0') {
	$stats{gc0}++;
    } elsif ($str =~ 'GC1') {
	$stats{gc1}++;
    } elsif ($str =~ 'GC2') {
	$stats{gc2}++;
    } elsif ($str =~ 'GC') {
	die "unrecognized GC line: $_"
    }

    if ($str =~ 'heap') {
	$stats{heap}++;
	if ($str =~ 'cons') {
	    $stats{heap_cons}++;
	} elsif ($str =~ 'vector') {
	    $stats{heap_vector}++;
	} elsif ($str =~ 'SEXP') {
	    $stats{heap_SEXP}++;
	} elsif ($str =~ 'promise') {
	    $stats{heap_promise}++;
	} elsif ($str =~ 'environment') {
	    $stats{heap_environment}++;
	} elsif ($str =~ 'list') {
	    $stats{heap_list}++;
	} else {
	    $stats{unknown}++;
	    print("Warning: unknown alloc line: ", $str);
	}
    } elsif ($str =~ 'stack') {
	$stats{stack}++;
	if ($str =~ 'cons') {
	    $stats{stack_cons}++;
	} elsif ($str =~ 'vector') {
	    $stats{stack_vector}++;
	} elsif ($str =~ 'SEXP') {
	    $stats{stack_SEXP}++;
	} elsif ($str =~ 'promise') {
	    $stats{stack_promise}++;
	} elsif ($str =~ 'environment') {
	    $stats{stack_environment}++;
	} elsif ($str =~ 'list') {
	    $stats{stack_list}++;
	} else {
	    $stats{unknown}++;
	    print("Warning: unknown alloc line: ", $str);
	}
    } else {
	$stats{unknown}++;
	print("Warning: unknown alloc line: ", $str);
    }

}

sub percent {
    my $n = shift;
    return $n / $stats{alloc}*100;
}

print($stats{returns}, " procedure returns\n");
print($stats{assigns}, " nonlocal assignments\n");
print($stats{p_returned}, " pointers escaping by return\n");
print($stats{p_assigned}, " pointers escaping by nonlocal assignment\n");
print($stats{gc0} + $stats{gc1} + $stats{gc2}, " total garbage collections\n");
print("  ", $stats{gc0}, " GC in generation 0\n");
print("  ", $stats{gc1}, " GC in generation 1\n");
print("  ", $stats{gc2}, " GC in generation 2\n");
print("\n");
print($stats{alloc}, " total allocations\n");
print($stats{heap}, " heap allocations ", percent($stats{heap}), "%\n");
print($stats{stack}, " stack allocations ", percent($stats{stack}), "%\n");
print($stats{dup}, " duplicates ", percent($stats{dup}), "%\n");
print($stats{escape}, " escape ", percent($stats{escape}), "%\n\n");
print($stats{in_env}, " part of environment ", percent($stats{in_env}), "%\n");
print($stats{parent}, " allocated in parent pool at least once ", percent($stats{parent}), "%\n");
print($stats{unknown}, " unknown allocations ", percent($stats{unknown}), "%\n");
print($stats{heap_cons} + $stats{stack_cons}, " cons cells ", percent($stats{heap_cons} + $stats{stack_cons}), "%\n");
print($stats{heap_vector} + $stats{stack_vector}, " vectors ", percent($stats{heap_vector} + $stats{stack_vector}), "%\n");
print($stats{heap_SEXP} + $stats{stack_SEXP}, " SEXPs ", percent($stats{heap_SEXP} + $stats{stack_SEXP}), "%\n");
print($stats{heap_promise} + $stats{stack_promise}, " promises ", percent($stats{heap_promise} + $stats{stack_promise}), "%\n");
print($stats{heap_environment} + $stats{stack_environment}, " environments ", percent($stats{heap_environment} + $stats{stack_environment}), "%\n");
print($stats{heap_list} + $stats{stack_list}, " lists ", percent($stats{heap_list} + $stats{stack_list}), "%\n");
print("In heap:\n");
print($stats{heap_cons}, " cons cells ", percent($stats{heap_cons}), "%\n");
print($stats{heap_vector}, " vectors ", percent($stats{heap_vector}), "%\n");
print($stats{heap_SEXP}, " SEXPs ", percent($stats{heap_SEXP}), "%\n");
print($stats{heap_promise}, " promises ", percent($stats{heap_promise}), "%\n");
print($stats{heap_environment}, " environments ", percent($stats{heap_environment}), "%\n");
print($stats{heap_list}, " lists ", percent($stats{heap_list}), "%\n");
print("In stack:\n");
print($stats{stack_cons}, " cons cells ", percent($stats{stack_cons}), "%\n");
print($stats{stack_vector}, " vectors ", percent($stats{stack_vector}), "%\n");
print($stats{stack_SEXP}, " SEXPs ", percent($stats{stack_SEXP}), "%\n");
print($stats{stack_promise}, " promises ", percent($stats{stack_promise}), "%\n");
print($stats{stack_environment}, " environments ", percent($stats{stack_environment}), "%\n");
print($stats{stack_list}, " lists ", percent($stats{stack_list}), "%\n");
