#!/bin/perl
#
use Text::Diff 'diff';

my $outfile="libintl-0.19.5.1.patch";

unlink $outfile;

my $orig_dir="intl.orig/";
my $dest_dir="intl/";

opendir(DIR, $dest_dir);
my @dest_files = readdir(DIR);
closedir(DIR);

@orig_files = sort grep { /\.([chyl]|rc)$/; } @orig_files;
@dest_files = sort grep { /\.([chyl]|rc)$/; } @dest_files;

if (open(OUT, "> $outfile")) {
	foreach my $dest_file (@dest_files) {
		my $diff = diff($orig_dir.$dest_file, $dest_dir.$dest_file, { STYLE => "Unified" });

		$diff =~ s/$orig_dir//g;
		$diff =~ s/$dest_dir//g;

		print OUT $diff;
	}
	close(OUT);
}

