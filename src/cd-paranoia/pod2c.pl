#!/usr/bin/perl
# $Id: pod2c.pl,v 1.4 2008/06/19 15:44:31 flameeyes Exp $
# Utility to turn pieces of pod text to help text.
use File::Basename;

die "Expecting exactly one argument, a filename" if @ARGV != 1;
$filename = shift;

($name, $path, $suffix)=fileparse($filename,"\.txt");
close(STDIN);
open(STDIN, "<$filename") 
  || die "Can't open $filename for reading:\n$!";

#$outfile="../${filename}.h";
#open(STDOUT, ">$outfile")
#  || die "Can't open $outfile for writing:\n$!";

print "/*
  Copyright (C) 1999, 2005, 2007, 2008 Rocky Bernstein
   
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
static const char ${name}_help[] =\n";
while(<STDIN>) {
  s/["]/\\"/g; 
  # Change POD'ed items to quoted items, e.g. See L<y> and L<z> becomes
  # See "y" and "z" (with \'s around the quotes)
  s/[C,L,I,B,F]<(.+?)>/\\"$1\\"/g; 
  chomp;
  if ( $^O eq "cygwin" ) {
     s/\//
  }
  print "\"$_\\n\"\n";
}
print ";\n";
