# Format the results of time testing for R read.table.

while (<STDIN>) {
    if (!($_ =~ /^\d+\.\d+u/)) { next; };  # only results of csh time
    @line = split;
    $_ = $line[2];         # get time
    ($min, $sec) = split ":";
    print $min * 60 + $sec, "\n";
}
