#!/usr/bin/perl

use Getopt::Std;
use File::Basename;

#
# Process stderr of MIC OpenCL application that was compiled with ENABLE_MIC_TBB_TRACER option.
# Processing is done in 2 stages:
#   1. Process raw output and create global table with trace of NDRange commands
#   2. Process raw output and create detailed table of specific NDRange command. The ID of NDRange command should be taken from
#      the global table.
#
# In both cases output should be redirected to the .csv file and opened by Excel.
# May take input from file or stdin. In the latter case pass '-' instead of file name in command line.
#

my %ndranges; # ndrange_id -> (global start time, global duration, threads_participated, number of coordinates, columns, raws, pages)

# 0 - global start time
# 1 - global duration
# 2 - number of threads participated 
# 3 - number of coordinates
# 4 - number of columns
# 5 - number of raws
# 6 - number of pages
my $global_start_time           = 0;
my $global_duration             = 1;
my $global_threads_participated = 2;
my $global_coordinates 			= 3;
my $global_columns 				= 4;
my $global_raws 				= 5;
my $global_pages				= 6;


my %threads; # $thread_id->hash (ndrange_id->array(core id, thread on core id, start time, end time, delay, duration, work search time, items))

# 0 - core id
# 1 - thread on the core id
# 2 - start time
# 3 - end time
# 4 - delay
# 5 - duration
# 6 - work search time
# 7 - items processed (space-separated string)
my $thread_core_id         = 0;
my $thread_hw_thread_id    = 1;
my $thread_start_time      = 2;
my $thread_end_time        = 3;
my $thread_start_delay     = 4;
my $thread_duration        = 5;
my $thread_work_search_time= 6;
my $thread_items_processed = 7;

my @ndrange_order;
my @threads_order;

sub parse_input
{
	my ($ndranges_limit) = @_;
	
	my %tmp_ndranges; # ndrange_id -> (coords, cols, raws, pages)
	my %tmp_thread_to_core; # thread_id -> (core id, hw thread id)

	my $ndranges_processed = 0;
	
	READ_DATA: while (<>)
	{
	   chomp();

	   if (!/^MIC_TBB_TRACER: /)
	   {
	        next; # continue to the next iteration
	   }

	   if (/^MIC_TBB_TRACER: THREAD/)
	   {
	       if (($thread_id, $core_id, $hw_thread_id) = /^MIC_TBB_TRACER: THREAD\s+(\d+)\s+ATTACH_TO\s+HW_CORE=(\d+)\s+HW_THREAD_ON_CORE=(\d+)\s*$/)
	       {
	       		$thread_id += 0;
	       		$core_id += 0;
	       		$hw_thread_id += 0;
	       		@{$tmp_thread_to_core{$thread_id}} = ($core_id, $hw_thread_id);
	       }
	       elsif (($thread_id) = /^MIC_TBB_TRACER: THREAD\s+(\d+)\s+DETACH\s*$/)
	       {
	       		$thread_id += 0;
	       		delete $tmp_thread_to_core{$thread_id};
	       }
	       next; # continue to the next iteration
	   }
	   
	   if (!/^MIC_TBB_TRACER: NDRANGE/)
	   {
	        next; # continue to the next iteration
	   }

	   if (($ndrange_id, $coords, $cols, $raws, $pages) = /^MIC_TBB_TRACER:\s+NDRANGE\s+(\d+)\s+COORDINATES\s+(\d+):\s*COLS=(\d+)\s+RAWS=(\d+)\s+PAGES=(\d+)\s*$/)
	   {
		   $ndrange_id += 0;
		   $coords += 0;
		   $cols += 0;
		   $raws += 0;
		   $pages += 0;

	   	   ++$ndranges_processed;
	   	   if (($ndranges_limit > 0) && ($ndranges_processed > $ndranges_limit))
	   	   {
	   		   # stop reading input
	   		   # exit while loop
	   		   last READ_DATA;
	   	   }

		   @{$tmp_ndranges{$ndrange_id}} = ($coords, $cols, $raws, $pages);
	   	   next; # continue to the next iteration
	   }
	   
	   ($ndrange_id, $thread_id, $attach, $detach, $search, $items) = /^MIC_TBB_TRACER:\s+NDRANGE\s+(\d+)\s+THREAD\s+(\d+):\s*attach=(\d+)\s+detach=(\d+)\s+search=(\d+)\s+indices:\s*(.*)$/;
	   $ndrange_id += 0;
	   $thread_id += 0;
	   $attach += 0;
	   $detach += 0;
	   $search += 0;

	   if (! exists $tmp_ndranges{$ndrange_id})
	   {
	   		++$ndranges_processed;
	   		if (($ndranges_limit > 0) && ($ndranges_processed > $ndranges_limit))
	   		{
	   			# stop reading input
	   			# exit while loop
	   			last READ_DATA;
	   		}
	   		@{$tmp_ndranges{$ndrange_id}} = (1); # 1D array by default
	   }
	   $threads{$thread_id}{$ndrange_id}[$thread_start_time] 		= $attach;
	   if (0 != $attach)
	   {
			$threads{$thread_id}{$ndrange_id}[$thread_end_time] 		= $detach;
	   		$threads{$thread_id}{$ndrange_id}[$thread_duration] 		= ($detach - $attach);
	   		$threads{$thread_id}{$ndrange_id}[$thread_work_search_time]	= $search;
	   		$threads{$thread_id}{$ndrange_id}[$thread_items_processed] 	= $items;
	   }
	}   

	@ndrange_order  = sort { $a <=> $b }  keys %tmp_ndranges;
	@threads_order = sort { $a <=> $b }  keys %threads;

	# calculate delays per thread and duration

	foreach $cmd (@ndrange_order)
	{
	    my $global_start = 0x7FFFFFFFFFFFFFFF + 0;
	    my $global_end   = 0;
	    my $active_threads = 0;
	    foreach $trd (@threads_order)
	    {
	    	my $my_start = $threads{$trd}{$cmd}[$thread_start_time];
	    	my $my_end   = $threads{$trd}{$cmd}[$thread_end_time];
	    	if ($my_start > 0)
	    	{
	    		++$active_threads;
	    		if ($global_start > $my_start)
		    	{
		    		$global_start = $my_start;
		    	}
		    }
	    	if (($my_end > 0) && ($global_end < $my_end))
	    	{
	    		$global_end = $my_end;
	    	}
	    }
	    
	    $ndranges{$cmd}[$global_start_time] 		= $global_start;
	    $ndranges{$cmd}[$global_duration] 			= ($global_end - $global_start);
	    $ndranges{$cmd}[$global_threads_participated] 	= $active_threads;
	    $ndranges{$cmd}[$global_coordinates] 		= $tmp_ndranges{$cmd}[0];
	    $ndranges{$cmd}[$global_columns] 			= $tmp_ndranges{$cmd}[1];
	    $ndranges{$cmd}[$global_raws] 			= $tmp_ndranges{$cmd}[2];
	    $ndranges{$cmd}[$global_pages] 			= $tmp_ndranges{$cmd}[3];

	    foreach $trd (@threads_order)
	    {
	        if (defined ($threads{$trd}{$cmd}[$thread_start_time]) && (0 != $threads{$trd}{$cmd}[$thread_start_time]))
	        {
	            $threads{$trd}{$cmd}[$thread_start_delay] = $threads{$trd}{$cmd}[$thread_start_time] - $global_start;

       			$threads{$trd}{$cmd}[$thread_core_id] 		= $tmp_thread_to_core{$trd}[0];
				$threads{$trd}{$cmd}[$thread_hw_thread_id] 	= $tmp_thread_to_core{$trd}[1];
	        }
	    }
	}
}

sub print_global_table
{
	# print header
	print "NDRange ID,";
	foreach $cmd (@ndrange_order)
	{
	    print ",$cmd";
	}
	print "\n";

	print "Global Start Time,";
	foreach $cmd (@ndrange_order)
	{
	    print ",$ndranges{$cmd}[$global_start_time]";
	}
	print "\n";

	print "Global Duration,";
	foreach $cmd (@ndrange_order)
	{
	    print ",$ndranges{$cmd}[$global_duration]";
	}
	print "\n";

	print "Threads Participated,";
	foreach $cmd (@ndrange_order)
	{
	    print ",$ndranges{$cmd}[$global_threads_participated]";
	}
	print "\n";


	#print threads
	foreach $thread (@threads_order)
	{
	    $ranges = $threads{$thread};

		print "Thread HW Core,$thread";
        foreach $cmd (@ndrange_order)
        {
            print ",$ranges->{$cmd}[$thread_core_id]";
        }
        print "\n";

		print "Thread HW Thread,$thread";
        foreach $cmd (@ndrange_order)
        {
            print ",$ranges->{$cmd}[$thread_hw_thread_id]";
        }
        print "\n";

		print "Thread delay,$thread";
        foreach $cmd (@ndrange_order)
        {
            print ",$ranges->{$cmd}[$thread_start_delay]";
        }
        print "\n";
	    
		print "Thread duration,$thread";
        foreach $cmd (@ndrange_order)
        {
            print ",$ranges->{$cmd}[$thread_duration]";
        }
        print "\n";

		print "Thread stealing time,$thread";
        foreach $cmd (@ndrange_order)
        {
            print ",$ranges->{$cmd}[$thread_work_search_time]";
        }
        print "\n";
	}
}

# what is the next item to be?
sub project_next_item
{
	my ($cmd, $item) = @_;
	
	my ($col,$raw,$page) = split(':', $item);
	$col  += 0;
	$raw  += 0;
	$page += 0;

	my $coords      = $ndranges{$cmd}[$global_coordinates];
	my $cols_count  = $ndranges{$cmd}[$global_columns];
	my $raws_count  = $ndranges{$cmd}[$global_raws];
	my $pages_count = $ndranges{$cmd}[$global_pages];

	my @next_item;

	# forward column
	++$col;

	if (($coords > 1) && ($col >= $cols_count))
	{
		# forward raw
		++$raw;
		$col  = 0;
	}

	if (($coords > 2) && ($raw >= $raws_count))
	{
		# forward page
		++$page;
		$raw = 0;
		$col = 0;
	}

	push(@next_item, $col);
	if ($coords > 1)
	{
		push(@next_item, $raw);
	}
	if ($coords > 2)
	{
		push(@next_item, $page);
	}
	return join(':',@next_item);
}

#
# get command id and string with numbers
# returns an array with pointers to arrays. Each internal array contains list of sequential subset of input numbers 
#
sub create_sequential_groups
{
	my ($cmd, $items) = @_;

	$items =~ s/^\s+//;
	$items =~ s/\s+$//;
	my @items = split(/\s+/, $items);

	my $items_count = $#items + 1;

	# find groups and count them
	# create array of arrays (groups)
	my $grp_idx = -1;
	my $prev_item = "-2";
	my @groups;
	foreach $item (@items)
	{
		my $projected_item = project_next_item( $cmd, $prev_item );
		if ($item ne $projected_item)
		{
			# new group
			++$grp_idx;
		}
		push( @{$groups[$grp_idx]}, $item );
		$prev_item = $item;
	}

	return @groups;
}

sub print_specific_table
{
	my ($required_ndrange) = @_;

	print "NDRange #$required_ndrange,coordinates=$ndranges{$required_ndrange}[$global_coordinates],columns=$ndranges{$required_ndrange}[$global_columns],raws=$ndranges{$required_ndrange}[$global_raws],pages=$ndranges{$required_ndrange}[$global_pages]\n";
	print "Thread,HW Core,HW Thread,Delay,Duration,Stealing,Items,Sequential Groups,Average Group,Max Group,Min Group, Groups\n";

	#print threads
	foreach $trd (@threads_order)
	{
	    if (defined( $threads{$trd}{$required_ndrange}[$thread_start_delay] ))
	    {
	    	my @groups = create_sequential_groups( $required_ndrange, 
	    										   $threads{$trd}{$required_ndrange}[$thread_items_processed] );

			# calculate number of groups, min, max sizes
			# create right string
			my $min = 0x7FFFFFFFFF;
			my $max = 0;
			my $groups_count = $#groups + 1;
			my $items_string = "";
			my $items_count = 0;
	    	foreach $grp (@groups)
	    	{
	    		my $grp_size =  $#{$grp} + 1;
	    		$items_count += $grp_size;
	    		
	    		if ($min > $grp_size)
	    		{
	    			$min = $grp_size;
	    		}
	    			
	    		if ($max < $grp_size)
	    		{
	    			$max = $grp_size;
	    		}

				$items_string = $items_string . "(" . join(" ", @{$grp}) . ")  ";
	    	}
	    	my $string = "$trd,$threads{$trd}{$required_ndrange}[$thread_core_id],$threads{$trd}{$required_ndrange}[$thread_hw_thread_id],";
	    	$string = $string . "$threads{$trd}{$required_ndrange}[$thread_start_delay],$threads{$trd}{$required_ndrange}[$thread_duration],";
	    	$string = $string . "$threads{$trd}{$required_ndrange}[$thread_work_search_time],";
			$string = $string . "$items_count,$groups_count," . sprintf("%.2f",$items_count/$groups_count) . ",$max,$min";
			$string = $string .",'$items_string";
	        print "$string\n";
	    }
	}
}

#
# ------------- Sequentiality ---------------------------------
#  Assume N is an overall amount of WGs
#  Assume $warmup_count is a number of sequential WGs to warm cache
#  Assume T is a number of threads participated
#
#  Ideal sequentiality is:
#  		if (N <= T*$warmup_count) --> 1
#       else --> (N - T*$warmup_count)
#
#  Real sequentiality is:
#       sum of per-thread warmed runs / ideal sequentiality count
#
# ------------- Balancing ---------------------------------
#
#  Ideal balance is N/T items per thread. If the number is not integer, it may be +1 also.
#  Real balance is number of threads with ideal balancing / T = % from the perfect balance.
#
# --------------- Return ----------------------------------
#
#  Return pair (sequentiality, balance)
#
sub calculate_single_sequentiality
{
	my ($ndrange_id, $warmup_count) = @_;

	my $command = $ndranges{$ndrange_id};

	my $T = $command->[$global_threads_participated];
	my $global_warm_count = $T * $warmup_count;

	my $N = $command->[$global_columns] * $command->[$global_raws] * $command->[$global_pages];

	if ($N <= $global_warm_count)
	{
		return (1,1);
	}

	my $ideal_sequentiality = $N - $global_warm_count;
	my $ideal_balance_real = $N / $T;
	my %ideal_balances;
	$ideal_balances{ int( $ideal_balance_real ) } = 1;
	if ( $ideal_balance_real != int( $ideal_balance_real ))
	{
		$ideal_balances{ int( $ideal_balance_real ) + 1 } = 1;
	}

	my $ideal_balances_count = 0;
	
	# get real sequentiality
	my $real_sequentiality = 0;
	foreach $trd (@threads_order)
	{
	    if (defined( $threads{$trd}{$ndrange_id}[$thread_start_delay] ))
	    {
	    	my $my_items_done = 0;
	    	
	    	my @groups = create_sequential_groups( $ndrange_id, 
	    										   $threads{$trd}{$ndrange_id}[$thread_items_processed] );

			foreach $grp (@groups)
			{
			    my $grp_count = $#{$grp} + 1;
			    if ($grp_count > $warmup_count)
			    {
			    	$real_sequentiality += ($grp_count - $warmup_count);
			    }

			    $my_items_done += $grp_count;
			}

			if (1 == $ideal_balances{ $my_items_done })
			{
				++$ideal_balances_count;
			}
	    }
	}

	return ($real_sequentiality / $ideal_sequentiality, $ideal_balances_count / $T);
}

sub print_single_sequentiality
{
	my ( $warmup_count, $ndrange_id ) = @_;

	my ( $sequentiality, $balance ) = calculate_single_sequentiality( $ndrange_id, $warmup_count );
	my $threads = $ndranges{$ndrange_id}[$global_threads_participated];

	printf ("NDRANGE %04d SEQUENTALITY=%5.1f%% BALANCE=%5.1f%% THREADS=%d\n", $ndrange_id, $sequentiality * 100, $balance * 100, $threads );
}

# calculate average and median
sub print_global_sequentiality
{ 
	my ( $warmup_count ) = @_;

	my @sequentiality;
	my @balance;
	my $average_sequentiality = 0;
	my $average_balance = 0;

	foreach $cmd (@ndrange_order)
	{
		my ( $sequentiality, $balance ) = calculate_single_sequentiality( $cmd, $warmup_count );
		$average_sequentiality += $sequentiality;
		$average_balance       += $balance;

		push( @sequentiality, $sequentiality );
		push( @balance,       $balance );
	}	

	my $n_commands = $#sequentiality + 1;
	
	$average_sequentiality /= $n_commands;
	$average_balance       /= $n_commands;

	@sorted_sequentiality = sort {$a <=> $b} @sequentiality;
	@sorted_balance       = sort {$a <=> $b} @balance;

	my $median_sequentiality;
	my $median_balance;

	if (1 == ($n_commands % 2))
	{
		my $median_id = $n_commands / 2 + 1;
		$median_sequentiality = $sorted_sequentiality[ $median_id ];
		$median_balance       = $sorted_balance[ $median_id ];
	}
	else
	{
		my $median_left = $n_commands / 2;
		my $median_right = $median_left + 1;

		$median_sequentiality = ($sorted_sequentiality[ $median_left ] + $sorted_sequentiality[ $median_right ]) / 2;
		$median_balance       = ($sorted_balance[ $median_left ] + $sorted_balance[ $median_right ]) / 2;
	}

	printf ("AVERAGE_SEQUENTALITY=%5.1f%% MEDIAN_SEQUENTALITY=%5.1f%% AVERAGE_BALANCE=%5.1f%% MEDIAN_BALANCE=%5.1f%%\n",
			$average_sequentiality * 100, $median_sequentiality  * 100, 
			$average_balance  * 100, $median_balance  * 100 );
}

sub print_all_sequentiality
{ 
	my ( $warmup_count ) = @_;

	foreach $cmd (@ndrange_order)
	{
	    print_single_sequentiality( $warmup_count, $cmd );
	}	
}

sub help
{
		my $name = basename($0);
		
		# usage message
	    print "
Post-process output of MIC_TBB_TRACER.

Usages:
	1. Create global threads usage table:
		$name  <log-file>

	2. Create specific ndrange usage table:
		$name  -c <ndrange-id> <log-file>

	3. Calculate global sequentiality value:
		$name  -s <cache-warmup-count> <log-file>

	4. Calculate specific ndrange sequentiality value:
		$name  -c <ndrange-id> -s <cache-warmup-count> <log-file>

	5. Calculate specific ndrange sequentiality value for all ndranges:
		$name  -c * -s <cache-warmup-count> <log-file>

	<ndrange-id> should be taken from global threads usage table.
	
	<cache-warmup-count> is a number of sequential WorkGroups that is reqiored to warmup cache.

	specific ndrange sequentiality value - relative number of WGs that use warmed cache. 0..1, 1 - maximum possible.
	global sequentiality value           - average and median values between all ndranges
	
	In all cases pass '-' instead of <log-file> to read stdin.
	Output is printed to stdout in .csv format.

	In order to limit processing of NDRange commands upto <cound> NDRanges use -l <count> with any 
	invocation format.
";	
}

sub main
{
	# defaults
	$opt_c = -1; # specific command ID to process
	$opt_s = -1; # the number of WGs required to warm cache
	$opt_l = -1; # the number of NDRanges to process (unlimited)
	
	my $ok = getopts( 'c:s:l:' );
	my $mode = 'global';

	if (!$ok)
	{
		print "Wrong options\n";
		help();
		return 1;
	}

	# check that filename exists
	my $arguments_given = $#ARGV + 1;
	if ($arguments_given != 1)
	{
		print "No filename given\n";
		help();
		return 1;
	}

	# check  options
	if (-1 != $opt_l)
	{
		if ($opt_l !~ /^[0-9]+$/)
		{
			# not a number
			print "Wrong -l option value: $opt_l\n";
			help();
			return 1;
		}
		# convert to integer
		$opt_l += 0;
	}

	if (-1 != $opt_c)
	{
		if ('*' eq $opt_c)
		{
			$mode = 'single';
		}
		else
		{
			if ($opt_c !~ /^[0-9]+$/)
			{
				# not a number
				print "Wrong -c option value: $opt_c\n";
				help();
				return 1;
			}
			# convert to integer
			$opt_c += 0;
			$mode = 'single';
		}
	}
	
	if (-1 != $opt_s)
	{
		if ($opt_s !~ /^[0-9]+$/)
		{
			# not a number
			print "Wrong -s option value: $opt_s\n";
			help();
			return 1;
		}
		# convert to integer
		$opt_s += 0;
		if ('global' eq $mode)
		{
			$mode = 'global_sequentiality';
		}
		else
		{
			$mode = 'single_sequentiality';
		}
	}

	if (('*' eq $opt_c) && ('single_sequentiality' ne $mode))
	{
		print "Wrong options - -c * may be used only with -s option\n";
		help();
		return 1;		
	}

	parse_input($opt_l);

	if ('global' eq $mode)
	{
		print_global_table();
	}
	elsif ('single' eq $mode)
	{
		print_specific_table( $opt_c );
	}
	elsif ('global_sequentiality' eq $mode)
	{
		print_global_sequentiality( $opt_s );
	}
	elsif ('single_sequentiality' eq $mode)
	{
		if ('*' eq $opt_c)
		{
			print_all_sequentiality( $opt_s );
		}
		else
		{
			print_single_sequentiality( $opt_s, $opt_c );
		}
	}
	else
	{
		# usage message
	    help();
	    return 1;
	}

	return 0;
}

# run 
exit main();

	
