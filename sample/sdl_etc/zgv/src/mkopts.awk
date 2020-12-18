# mkopts.awk - make getopt_long(), config file tables, and short-option
#   string from options.src. (Based on xzgv's.)
#
# See options.src for details of the format that file uses.

# writing to /dev/stderr for errors is dodgy if we're not using gawk,
# so I just use stdout, which should be ok.


BEGIN \
  {
  # thanks to the comp.lang.awk FAQ for this old-awk-detecting trick :-)
  if(ARGC==0)
    {
    print "mkopts: error: must run under a `new' awk, e.g. gawk, mawk, nawk."
    exit 1
    }
  
  runend=1
  optfile="rcfile_opt.h"
  varfile="rcfile_var.h"
  shortoptfile="rcfile_short.h"
  getopt_string=""
  exit_val=0
  
  edit_warning="/* auto-generated from options.src, edits will be lost! */"
  
  print edit_warning >optfile
  print "struct option long_opts[]="	>>optfile
  print "  {"				>>optfile
  
  print edit_warning >varfile
  print "struct cfglookup_tag cfglookup[]=" >>varfile
  print "  {"				>>varfile

  print edit_warning >shortoptfile
  }


END \
  {
  if(runend)
    {
    print "  {NULL,0,NULL,0}"		>>optfile
    print "  };"			>>optfile
    
    print "  {\"\",0,NULL,NULL}"	>>varfile
    print "  };"			>>varfile
  
    if(getopt_string=="")
      {
      print "mkopts:" NR ": NULL short-options string!"
      exit_val=1
      }
    else
      {
      printf("#define SHORTOPT_STRING\t\"%s\"\n",getopt_string) >>shortoptfile
      }
    
    exit exit_val
    }
  }


# ignore comment lines and blank lines
/^$/ || /^#/	{ next }


{
if(NF!=7)
  {
  print "mkopts:" NR ": line must have 7 fields"
  exit_val=1
  next
  }

# check none of them end in commas
for(f=1;f<=NF;f++)
  {
  if($f ~ /,$/)
    {
    print "mkopts:" NR ": field ends with a comma"
    exit_val=1
    next
    }
  }

name=$1
shortopt=$2
allow_option=$3
allow_config=$4
has_arg=$5
funcptr=$6
dataptr=$7

# convert has_arg to getopt_long stuff
sub(/NO_ARG/,"no_argument",has_arg);
sub(/OPT_ARG/,"optional_argument",has_arg);
sub(/REQ_ARG/,"required_argument",has_arg);

# the config vars file
# They go here whether wanted as config vars or not, as this is
# also used to lookup funcptr/dataptr for command-line options.
#
printf("  {%s,\t%d,\t%s,\t%s},\n",name,allow_config,funcptr,dataptr) >>varfile

# the options file
if(allow_option)
  {
  printf("  {%s,\t%s,\tNULL,\t%s},\n",name,has_arg,shortopt) >>optfile
  
  # add any short option name on to getopt_string
  if(shortopt!="0" && shortopt!="NUL")
    {
    if(shortopt !~ /^'.'$/)
      {
      print "mkopts:" NR ": short option field is not of form 'c'"
      exit_val=1
      next
      }
    
    gsub(/'/,"",shortopt)
    getopt_string=getopt_string shortopt
    
    # only give `:' when arg is *required*; you can't do optional
    # args with short options.
    #
    if(has_arg=="required_argument")
      {
      getopt_string=getopt_string ":"
      }
    }
  }
}
