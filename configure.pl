#!perl
# 
use warnings;
use strict;

use Term::ANSIScreen qw/:color :cursor :screen/;
use POSIX qw(strftime);
use File::Which;
use File::Copy;
use File::Spec::Functions qw(:ALL);
use File::Basename;
use File::Path;
use Cwd qw(chdir);

#==============================================================================
#==============================================================================
# SUBROUTINES
#==============================================================================
#==============================================================================


#------------------------------------------------------------------------------
# OS Checking
#------------------------------------------------------------------------------
#
# Check the OS to see what we are running on.
# then we can make better decisions about what to load and run
#
use Config;
use feature qw(switch);

sub os_check {
   given ($Config{osname})
   {
      when('MSWin32') {return 'windows';}
      when('linux')   {return 'linux';}
      when('darwin')  {return 'mac';}
      default {return $Config{osname};}
   }
}
#=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-



sub setup_build {

   my $os = os_check();
   if ($os eq 'windows') {
      require Win32::Console::ANSI;
   }

   my $CMakeOptions;
   my $MakeCommand;

   print "Configuring build space for "; print color 'green'; 
   if ($os eq 'windows') {
         print "windows"; 
   } elsif ($os eq 'linux') {
      print "linux"; 
   } elsif ($os eq 'mac') {
      print "mac"; 
   } else {
      print color 'bold red'; print "\nError: "; print color 'reset';
      die "Could not determine your OS!\n";
   }
   print color 'reset';print ".\n"; 

   # Do some checking to see if our environment will be OK.
   print "Checking for `CMake'...\n";
   my $cmake_path = which 'cmake';
   if (!defined($cmake_path)) {
         print color 'bold red'; print "Error: "; print color 'reset';
         die "CMake is not in your path!\n";
   }
   print "CMake found at: "; print color 'bold yellow'; print "$cmake_path\n";print color 'reset';

   if ($os eq 'windows') {
      # Check for WT installation.
      my $st32_dir = $ENV{WTDIR};
      if (!defined($st32_dir)) {
         print color 'bold red'; print "Error: "; print color 'reset';
         die ("WT tools not installed, unable to continue.\n");
      }
      print "WT installation found at: "; print color 'bold yellow'; print "$st32_dir\n";print color 'reset';

      ## Now that we have WT, we need to makesure that is set so CMAKE can set the proper install path.
      $ENV{SAD_INSTALLDIR} = "$st32_dir/bin";
      print "Install directory set to: "; print color 'cyan'; print "$ENV{SAD_INSTALLDIR}"; print color 'reset'; print ".\n";

   } else {
      ## In OSs other than windows, we usually install sad in /usr/local/bin
      $ENV{SAD_INSTALLDIR} = "/usr/local/bin";
      print "Install directory set to: "; print color 'cyan'; print "$ENV{SAD_INSTALLDIR}"; print color 'reset'; print ".\n";
   }

   if ($os eq 'windows') {
      # Check to see which compiler we will be using.
      # In windows, we'll probably want to use WATCOM more often, so
      # we will look for that first.  Otherwise we will use BCC, which is in
      # the normal ST32 installation.
      print "Checking for compiler: ";
      my $watcom_dir = $ENV{WATCOM};
      if (!defined($watcom_dir)) {
         print color 'green'; print "Borland"; print color 'reset'; print ".\n";
         $CMakeOptions = "-G \"Borland Makefiles\"";
         $MakeCommand = "make";
      }
      else
      {
         print color 'green'; print "Watcom"; print color 'reset'; print ".\n";
         $CMakeOptions = "-G \"Watcom WMake\"";
         $MakeCommand = "wmake";
      }
   } else
   {
      # Linux and mac, we will use the defaults.
      $CMakeOptions = ' ';
      $MakeCommand = 'make';
   }

   # Change to the dir this file is in.
   my $srcdir;
   $srcdir = canonpath(catpath((splitpath($0))[0..1])) || curdir();
   if ($srcdir ne curdir()) {
     print "Setting directory to $srcdir\n";
     chdir($srcdir) || die "Can't cd to `$srcdir'";
   }

   # Now create a directory to build in.  Usually like 'build'
   my $build_dir = $srcdir;
   $build_dir .= "/build";
   print "Build directory: "; print color 'cyan'; print "$build_dir\n"; print color 'reset'; 
   
   MKDIR:   
   # First see if this directory already exists
   if (-e $build_dir) {
      unless (-d $build_dir){
         print color 'bold red'; print "Error: "; print color 'reset';
         die "File exits but is not a directory.";
      }
      printf "Note: Directory already exists.\n";
      ## Do we want to remove it?
      print color 'bold white'; print "\tWould you like to remove the directory "; print color 'bold yellow';
      print "$build_dir"; print color 'reset'; print color 'bold white'; print " and start fresh (Y/N)? "; print color 'reset';
      my $userin = <STDIN>;
      chomp $userin;
      if (($userin eq "Y") || ($userin eq "y"))  {
          print color 'magenta'; print "Removing directory `$build_dir'...\n"; print color 'reset';
          rmtree($build_dir);
          goto MKDIR;
      }
   }
   else
   {
      print "Creating directory...";
      eval {mkpath($build_dir)};
      if ($@) {
         print color 'bold red'; print "\nError: "; print color 'reset';
         die "Could not create the directory $build_dir: $@";
      }
      print color 'bold green'; print "OK\n";print color 'reset';
   }

   # Change to our build directory.
   chdir($build_dir) || die "Can't cd to `$build_dir'";

   my $cmake_command = "cmake $CMakeOptions ..";
   print "Running CMake ($cmake_command)...\n";
   my $cmake_ret = system($cmake_command);
   print "Cmake returned `$cmake_ret'\n";
 
  
   if ($cmake_ret == 0) {

      print color 'bold green'; print "CMake was successful.\n"; print color 'reset';
      print"\nTo build your project, change to the '$build_dir' directory and type:\n";
      print"\t$MakeCommand\n";
      print"\nTo install your project, change to the '$build_dir' directory and type:\n";
      print"\t$MakeCommand install\n\n";
   }
   
   return $cmake_ret
}

local $| = 1;
my $retval = setup_build();
exit ($retval>>8);




