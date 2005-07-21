# make_name_map_util.pl
# John Garvin
# April 30, 2004
#

# Designed to be run on "$R_HOME/src/main/names.c".  Grabs entries in
# the R_FunTab table and outputs entries suitable for the "name_map"
# array in "$RCC_HOME/src/get_name.c". The output will include entries
# technically excluded by #ifdefs. You'll have to take care of those
# yourself. Duplicates, too--just pass it thru 'uniq'.

while (<>) {
    print if /^#/ or s/^{\S*?,\s*(\w*),.*$/  {\"$1\", $1},/;
}
