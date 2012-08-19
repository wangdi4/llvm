#!/usr/bin/perl

#
# Process stdout of MIC OpenCL application that was compiled with ENABLE_MIC_TBB_TRACER option.
# Processing is done in 2 stages:
#   1. Process raw output and create global table with trace of NDRange commands
#   2. Process raw output and create detailed table of specific NDRange command. The ID of NDRange command should be taken from
#      the global table.
#
# In both cases output should be redirected to the .csv file and opened by Excel.
# May take input from file or stdin. In the latter case pass '-' instead of file name in command line.
#

my %ndranges; # ndrange_id -> (global start time, global duration, threads_participated)

# 0 - global start time
# 1 - global duration
# 2 - number of threads participated 
my $global_start_time           = 0;
my $global_duration             = 1;
my $global_threads_participated = 2;


my %threads; # $thread_id->hash (ndrange_id->array(start time, end time, delay, duration, items))

# 0 - start time
# 1 - end time
# 2 - delay
# 3 - duration
# 4 - items processed (space-separated string)
my $thread_start_time      = 0;
my $thread_end_time        = 1;
my $thread_start_delay     = 2;
my $thread_duration        = 3;
my $thread_items_processed = 4;

my @ndrage_order;
my @threads_order;

sub parse_input
{
	my %tmp_ndranges;
	
	while (<>)
	{
	   chomp();
	   if (!/^MIC_TBB_TRACER: NDRANGE/)
	   {
	        next;
	   }
	   
	   ($ndrange_id, $thread_id, $attach, $detach, $items) = /^MIC_TBB_TRACER:\s+NDRANGE\s+(\d+)\s+THREAD\s+(\d+):\s*attach=(\d+)\s+detach=(\d+)\s+indices:\s*(.*)$/;
	   $ndrange_id += 0;
	   $thread_id += 0;
	   $attach += 0;
	   $detach += 0;
	   
	   $tmp_ndranges{$ndrange_id} = 1;
	   $threads{$thread_id}{$ndrange_id}[$thread_start_time] 		= $attach;
	   if (0 != $attach)
	   {
			$threads{$thread_id}{$ndrange_id}[$thread_end_time] 		= $detach;
	   		$threads{$thread_id}{$ndrange_id}[$thread_duration] 		= ($detach - $attach);
	   		$threads{$thread_id}{$ndrange_id}[$thread_items_processed] 	= $items;
	   }
	}   

	@ndrage_order  = sort { $a <=> $b }  keys %tmp_ndranges;
	@threads_order = sort { $a <=> $b }  keys %threads;

	# calculate delays per thread and duration

	foreach $cmd (@ndrage_order)
	{
	    my @starts, @ends;
	    foreach $trd (@threads_order)
	    {
	        push(@starts, $threads{$trd}{$cmd}[$thread_start_time]);
	        push(@ends,   $threads{$trd}{$cmd}[$thread_end_time]);
	    }
	    my @sorted_starts = sort { $a <=> $b } @starts;    
	    while (0 == $sorted_starts[0]) { shift @sorted_starts; }
	    $global_start = $sorted_starts[0];;

	    my @sorted_ends = sort { $b <=> $a } @ends;    
	    $global_end = shift @sorted_ends;
	    
	    $ndranges{$cmd}[$global_start_time] 			= $global_start;
	    $ndranges{$cmd}[$global_duration] 				= ($global_end - $global_start);
	    $ndranges{$cmd}[$global_threads_participated] 	= $#sorted_starts + 1;

	    foreach $trd (@threads_order)
	    {
	        if (defined ($threads{$trd}{$cmd}[$thread_start_time]) && (0 != $threads{$trd}{$cmd}[$thread_start_time]))
	        {
	            $threads{$trd}{$cmd}[$thread_start_delay] = $threads{$trd}{$cmd}[$thread_start_time] - $global_start;
	        }
	    }
	}
}

sub print_global_table
{
	# print header
	print "NDRange ID,";
	foreach $cmd (@ndrage_order)
	{
	    print ",$cmd";
	}
	print "\n";

	print "Global Start Time,";
	foreach $cmd (@ndrage_order)
	{
	    print ",$ndranges{$cmd}[$global_start_time]";
	}
	print "\n";

	print "Global Duration,";
	foreach $cmd (@ndrage_order)
	{
	    print ",$ndranges{$cmd}[$global_duration]";
	}
	print "\n";

	print "Threads Participated,";
	foreach $cmd (@ndrage_order)
	{
	    print ",$ndranges{$cmd}[$global_threads_participated]";
	}
	print "\n";


	#print threads
	foreach $thread (@threads_order)
	{
	    $ranges = $threads{$thread};

		print "Thread delay,$thread";
        foreach $cmd (@ndrage_order)
        {
            print ",$ranges->{$cmd}[$thread_start_delay]";
        }
        print "\n";
	    
		print "Thread duration,$thread";
        foreach $cmd (@ndrage_order)
        {
            print ",$ranges->{$cmd}[$thread_duration]";
        }
        print "\n";
	}
}

sub print_specific_table
{
	my ($required_ndrange) = @_;

	print "NDRange #$required_ndrange\n";
	print "Thread,Delay,Duration,Items,Sequential Groups,Average Group,Max Group,Min Group, Groups\n";

	#print threads
	foreach $trd (@threads_order)
	{
	    if (defined( $threads{$trd}{$required_ndrange}[$thread_start_delay] ))
	    {
	    	my $items = $threads{$trd}{$required_ndrange}[$thread_items_processed];
	    	$items =~ s/^\s+//;
	    	$items =~ s/\s+$//;
	    	my @items = split(/\s+/, $items);

	    	my $items_count = $#items + 1;

			# find groups and count them
			# create array of arrays (groups)
			my $grp_idx = -1;
			my $prev_item = 0xFFFFFFFFFFFFFFF0;
			my @groups;
			foreach $item (@items)
			{
				$item += 0;
				if (($prev_item + 1) != $item)
				{
					# new group
					++$grp_idx;
				}
				push( @{$groups[$grp_idx]}, $item );
				$prev_item = $item;
			}

			# calculate number of groups, min, max sizes
			# create right string
			my $min = 0x7FFFFFFFFF;
			my $max = 0;
			my $groups_count = $#groups + 1;
			my $items_string = "";
	    	foreach $grp (@groups)
	    	{
	    		if ($min > ($#{$grp} + 1))
	    		{
	    			$min = ($#{$grp} + 1);
	    		}
	    			
	    		if ($max < ($#{$grp} + 1))
	    		{
	    			$max = ($#{$grp} + 1);
	    		}

				$items_string = $items_string . "(" . join(" ", @{$grp}) . ")  ";
	    	}
	    	my $string = "$trd,$threads{$trd}{$required_ndrange}[$thread_start_delay],$threads{$trd}{$required_ndrange}[$thread_duration],";
			$string = $string . "$items_count,$groups_count," . sprintf("%.2f",$items_count/$groups_count) . ",$max,$min,$items_string";
	        print "$string\n";
	    }
	}
}

sub main
{
	my $arguments_given = $#ARGV + 1;
	my $required_ndrange = 0;

	if ($arguments_given == 1)
	{
		# global table mode
	}
	elsif ($arguments_given == 2)
	{
		#specific table mode
		$required_ndrange = shift @ARGV;
		$required_ndrange += 0;	
	}
	else
	{
		# usage message
	    print "
Post-process output of MIC_TBB_TRACER.

Usages:
	1. Create global threads usage table:
		$0  <log-file>

	2. Create specific ndrange usage table:
		$0  <ndrange-id> <log-file>

	<ndrange-id> should be taken from global threads usage table.
	In both cases pass '-' instead of <log-file> to read stdin.
	Output is printed to stdout in .csv format.

";	
	    return 1;
	}

	parse_input();

	if (0 == $required_ndrange)
	{
		print_global_table();
	}
	else
	{
		print_specific_table( $required_ndrange );
	}

	return 0;
}

# run 
exit main();

	
