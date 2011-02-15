#!/usr/bin/perl -w

# Gather allocation stats from memory debugging output. Set the
# R_DUMP_STATS environment variable to produce debugging output on
# stderr. Works for R-2.1.1rcc interpretation and RCC-compiled code.
#
# Option -s: print summary only, no call graph
#
# author: John Garvin

use strict;
use warnings;

use List::Util("min", "sum");
use Graph::Directed;

my %objects;  # key = address of allocated object
              # value = (output string,
              #          stack depth on allocation,
              #          minimum stack depth to which object may escape,
              #          whether allocation is in R inititialization)

my @callstack;    # procedure call stack
push @callstack, "<MAIN>";

my $callgraph = Graph::Directed->new(countedged => 1);

my $dup = 0;  # true if objects being allocated are part of a
	      # duplicate() call

my $in_env = 0;  # whether objects currently being allocated are part
		 # of an environment (environment allocation takes
		 # place after entering a procedure until allocation
		 # the actual environment object)

my $in_init = 1;  # true if program is in R initialization

my $debug = 0;

my %stats = ('alloc'       => 0,    # total objects allocated
             'dup'         => 0,    # objects allocated by duplicate()
	     'escape'      => 0,    # objects that escape (returned or assigned)
	     'assigns'     => 0,    # number of nonlocal assignments that occur
	     'returns'     => 0,    # number of procedure returns that occur
	     'p_assigned'  => 0,    # inc. for each pointer escaping by nonlocal assignment
	     'p_returned'  => 0,    # inc. for each pointer escaping by return

	     'unknown'     => 0,    # objects where we don't know how they got allocated
	     
	     'in_env'      => 0,    # objects that are part of an
				    # environment (both cons cells and
				    # environment objects)
	     'parent'      => 0,    # at least once moved to parent
				    # pool due to lack of space
	     
	     'gc0' => 0,    # times garbage collection has occurred, for each generation
	     'gc1' => 0,
	     'gc2' => 0,
    );

my %proto_calls = (
    'calls'       => 0,    # number of procedure calls that occur
    'promise'     => 0,    # arguments that are promises
    'nonpromise'  => 0,    # arguments that are not promises
    );

my %setup_calls = %proto_calls;
my %post_calls = %proto_calls;

# prototype hash for allocation stats
my %proto_alloc = (
	     'cons'        => 0,    # objects allocated by cons()
	     'vector'      => 0,    # objects allocated by allocVector()
	     'SEXP'        => 0,    # objects allocated by allocSExp()
	     'promise'     => 0,    # objects allocated by mkPROMISE()
	     'environment' => 0,    # objects allocated by NewEnvironment()
	     'list'        => 0,    # objects allocated by allocList()
    );

my %setup = %proto_alloc;     # objects allocated in R initialization
my %heap = %proto_alloc;      # after init, heap allocated
my %locally = %proto_alloc;   # after init, locally allocated

my $print_callgraph = 1;
if (defined($ARGV[0]) and $ARGV[0] eq "-s") {
    $print_callgraph = 0;
    shift;
}

while (<STDIN>) {
    chomp();
    analyze_line();
}

foreach my $ptr (keys(%objects)) {
    my ($v0, $v1, $v2, $v3) = @{ $objects{$ptr} };
    if ($debug) {
	print("pushing (b): ");
	print($v0);
	print($v1, "\n");
	print($v2, "\n");
	print($v3, "\n");
    }
    analyze_alloc_line($v0, $v1, $v2, $v3);
}

sub by_decreasing_edge_count {
    $callgraph->get_edge_count($b->[0], $b->[1]) <=> $callgraph->get_edge_count($a->[0], $a->[1]);
}
my $hist_size = 20;  # highlight top $hist_size edges
my @edges = sort by_decreasing_edge_count $callgraph->unique_edges;
my $threshold; # size of least of top $hist_size edges
if (@edges >= $hist_size) {
    $threshold = $callgraph->get_edge_count($edges[$hist_size-1]->[0], $edges[$hist_size-1]->[1]);
} else {
    $threshold = 0;
}
if ($print_callgraph) {
    print("digraph dynamic_call_graph {\n");
    foreach my $edge ($callgraph->unique_edges) {
	my $a = $edge->[0];
	my $b = $edge->[1];
	my $color;
	if ($a =~ /^I/) {
	    $color = "blue";
	} else {
	    $color = "red";
	}
	print("  \"", $a, "\" -> \"", $b, "\" ");
	print("[label=", $callgraph->get_edge_count($a,$b), "] ");
	print("[weight=", $callgraph->get_edge_count($a,$b), "] ");
	print("[color=", $color, "] ");
	print("[penwidth=3] ") if ($callgraph->get_edge_count($a,$b) >= $threshold);
	print(";\n");
    }
    print("}\n");
}
print("/*\n") if $print_callgraph;
print("Top twenty procedure calls:\n");
foreach my $i (0..$hist_size-1) {
    print("  ", $callgraph->get_edge_count($edges[$i]->[0], $edges[$i]->[1]), " ", $edges[$i]->[0], " -> ", $edges[$i]->[1], "\n") if (@edges > $i);
}
print("\n");
print($setup_calls{calls} + $post_calls{calls}, " procedure calls\n");
print("  in setup:\n");
print("    ", $setup_calls{calls}, " calls\n");
print("    ", $setup_calls{promise} + $setup_calls{nonpromise}, " total args\n");
print("    ", $setup_calls{promise}, " promise args\n");
print("    ", $setup_calls{nonpromise}, " non-promise args\n");
print("  post-setup:\n");
print("    ", $post_calls{calls}, " calls\n");
print("    ", $post_calls{promise} + $post_calls{nonpromise}, " total args\n");
print("    ", $post_calls{promise}, " promise args\n");
print("    ", $post_calls{nonpromise}, " non-promise args\n");
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
print(sum(values(%setup)), " allocations during setup ", percent(sum(values(%setup))), "%\n");
print(sum(values(%heap)), " heap allocations after setup ", percent(sum(values(%heap))), "%\n");
print(sum(values(%locally)), " local allocations after setup ", percent(sum(values(%locally))), "%\n");
print($stats{dup}, " duplicates ", percent($stats{dup}), "%\n");
print($stats{escape}, " escape ", percent($stats{escape}), "%\n\n");
print($stats{in_env}, " part of environment ", percent($stats{in_env}), "%\n");
print($stats{parent}, " allocated in parent pool at least once ", percent($stats{parent}), "%\n");
print($stats{unknown}, " unknown allocations ", percent($stats{unknown}), "%\n");
print($setup{cons} + $heap{cons} + $locally{cons}, " cons cells ", percent($heap{cons} + $locally{cons}), "%\n");
print($setup{vector} + $heap{vector} + $locally{vector}, " vectors ", percent($heap{vector} + $locally{vector}), "%\n");
print($setup{SEXP} + $heap{SEXP} + $locally{SEXP}, " SEXPs ", percent($heap{SEXP} + $locally{SEXP}), "%\n");
print($setup{promise} + $heap{promise} + $locally{promise}, " promises ", percent($heap{promise} + $locally{promise}), "%\n");
print($setup{environment} + $heap{environment} + $locally{environment}, " environments ", percent($heap{environment} + $locally{environment}), "%\n");
print($setup{list} + $heap{list} + $locally{list}, " lists ", percent($heap{list} + $locally{list}), "%\n");
print("In setup:\n");
breakdown(\%setup);
print("In heap (post-setup):\n");
breakdown(\%heap);
print("Procedure-local (post-setup):\n");
breakdown(\%locally);
print("\n");
print("*/\n") if $print_callgraph;

sub breakdown {
    my $h = shift;
    print("  ", $$h{cons}, " cons cells ", percent($$h{cons}), "%\n");
    print("  ", $$h{vector}, " vectors ", percent($$h{vector}), "%\n");
    print("  ", $$h{SEXP}, " SEXPs ", percent($$h{SEXP}), "%\n");
    print("  ", $$h{promise}, " promises ", percent($$h{promise}), "%\n");
    print("  ", $$h{environment}, " environments ", percent($$h{environment}), "%\n");
    print("  ", $$h{list}, " lists ", percent($$h{list}), "%\n");
}

sub analyze_line {
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
	    my ($v0, $v1, $v2, $v3) = @{ $objects{$ptr} };
	    if ($debug) {
		print("pushing (a): ");
		print($ptr, "\n");
		print($v0);
		print($v1, "\n");
		print($v2, "\n");
		print($v3, "\n");
	    }
	    analyze_alloc_line($v0, $v1, $v2, $v3);
	}
	$objects{$ptr} = [$_, scalar(@callstack), scalar(@callstack), $in_init];
    } elsif (/^Entering/) {
	my ($func, $source, $arguments) = @words[2..4];
	$func = $source . ":" . $func;
	$callgraph->add_edge($callstack[$#callstack], $func) unless $in_init;
	push(@callstack, $func);
	
	$arguments =~ /^\[.*\]$/ or die "bad args info in \"Entering\" line";
	my $h;
	if ($in_init) {
	    $h = \%setup_calls;
	} else {
	    $h = \%post_calls;
	}
	$$h{calls}++;
	while ($arguments =~ /P/g) {
	    $$h{promise}++;
	}
	while ($arguments =~ /N/g) {
	    $$h{nonpromise}++;
	}
	$in_env = 1;
    } elsif (/^Returning/) {
	pop(@callstack);
	$stats{returns}++;
	if (scalar(@words) > 6) {
	    my $ptr;
	    foreach $ptr (@words[6,]) {
		# returned pointers may be used in caller
		if (!defined($objects{$ptr})) { die("pointer $ptr returned but not allocated?\n"); }
		$stats{p_returned}++;
		$objects{$ptr}[2] = min($objects{$ptr}[2], scalar(@callstack));
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
		if (!defined($objects{$ptr})) { die("pointer $ptr assigned but not allocated?\n"); }
		$stats{p_assigned}++;
		$objects{$ptr}[2] = 0;
	    }
	}
    } elsif (/^end of setup/) {
	$in_init = 0;
    } else {
	# ignore all other lines
    }
}

sub analyze_alloc_line {
    my ($str, $alloc_depth, $esc_depth, $in_init) = @_;
    if ($alloc_depth != $esc_depth) {
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

    my $h;
    if ($in_init) {
	$h = \%setup;
    } elsif ($str =~ '[hH]eap') {
	$h = \%heap;
    } elsif ($str =~ 'stack') {
	$h = \%locally;
    } else {
	$stats{unknown}++;
	print("Warning: unknown alloc line: ", $str);
	return;
    }

    if ($str =~ 'cons') {
	$$h{cons}++;
    } elsif ($str =~ 'vector') {
	$$h{vector}++;
    } elsif ($str =~ 'SEXP') {
	$$h{SEXP}++;
    } elsif ($str =~ 'promise') {
	$$h{promise}++;
    } elsif ($str =~ 'environment') {
	$$h{environment}++;
    } elsif ($str =~ 'list') {
	$$h{list}++;
    } else {
	$stats{unknown}++;
	print("Warning: unknown alloc line: ", $str);
	return;
    }
}

sub percent {
    my $n = shift;
    if ($stats{alloc} == 0) {
	return 0;
    } else {
	return $n / $stats{alloc}*100;
    }
}

# sub min {
#     my $x = shift;
#     my $y = shift;
#     if ($x < $y) {
# 	return $x;
#     } else {
# 	return $y;
#     }
# }

