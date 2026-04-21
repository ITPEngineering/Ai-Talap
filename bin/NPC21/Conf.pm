package NPC21::Conf;

# "Split YAML" loading from a file tree:
#
# conf/conf.yml/
# conf/conf.yml/bc3.yml
# conf/conf.yml/bc3/slot-1.yml
# conf/conf.yml/bc3/slot1.yml
# conf/conf.yml/bc3/slot2.yml
# conf/conf.yml/lb340.yml
# ...

use YAML;
use JSON;
use Data::Dumper;

my $conffile;

sub load {
    my ($name) = @_;
#warn "$name";
    $conffile //= $file;  # hack for error reporting in sub macaddr

    if (-f $name) {
#warn CHECKPOINT, $name;
        my $file = $name;
        my $ret = eval {YAML::LoadFile($file)};
        if ($@) {
            my ($err) = ($@ =~ /YAML Error: (.*)/);
            my ($line) = ($@ =~ /Line: (\d+)/);
            $line -= 3;         # investigate later
            die "$file:$line: $err\n";
        }
        my $dir = ($name =~ s/\.yml$//r);
        if (-d $dir) {
            my $more = load($dir);
            for (keys %$more) {
                if (exists $ret->{$_}) {
                    die "$dir/$_: also seen in $file\n";
                }
                $ret->{$_} = $more->{$_};
            }
        }
        return $ret;
    }

    elsif (-d $name) {  # handle *.yml only
        my $dir = $name;
        my $ret = {};
#warn CHECKPOINT, $name;
        opendir (my $dh, $dir) or die "$dir: $!\n";
#warn CHECKPOINT;
        local $_;
        while (readdir $dh) {
            if (s/\.yml$//) {
                $ret->{$_} = load("$dir/$_.yml");
            }
            else {
                next if /^\.$/;
                next if /^\.\.$/;
                next if /^.#$/;  # emacs lock file
                next if /[-~]$/;
                next if /\.bak$/;
                next if /\.swp$/;  # vim swap file
                next if -d "$dir/$_" && -f "$dir/$_.yml";
                warn "$dir/$_ unrecognized, skipping";
            }
        }
        return $ret;
    }

    elsif (! -e $name) {
        die "$name: No such file or directory\n";
    }

    die "$name: don't know how to handle\n";
}


# return mac address of the "main" base module for $plc_name from
# conf.yml tree.  For LB241, it is the slot-1 module.  For LB340, it
# is the base module with the lowest slot number.
sub macaddr {
    my ($tree, $plc_name, $lan_slot) = @_;
    my @macaddr;
    for (keys %{$tree->{$plc_name}}) {
        ref $tree->{$plc_name}{$_} || next; # shallow, e.g. `hostname'
        my $macaddr = $tree->{$plc_name}{$_}{macaddr} // next;
        my $module = $tree->{$plc_name}{$_}{module} // next;
        if ($module =~ /^bcbase|LB241BC|LB241CPU/) {
            if (/^slot-1$/) {
                return $macaddr;    # LB241, no need to look further
            }
        }
        elsif ($module =~ /LB340CPU|LB340IEC104/) {
            /^slot(\d+)/ || die "`$_': expected slotN";
            if ($lan_slot) {
                if ($1 == $lan_slot) {
                    return $macaddr;  # expliticly specified slot
                }
            }
            else {
                push @macaddr, [$1, $macaddr];  # will pick leftmost slot below
            }
        }
        else {
            # i/o module with macaddr, not base
        }
    }

    if ($lan_slot) {
        die "$conffile: $plc_name:$lan_slot: not a base module or no macaddr\n";
    }

    # slot not explicitly specified, use mac addr from leftmost slot
    for (sort {$a->[0]<=>$b->[0]} @macaddr) {
        if ($_->[0] >= 1) {  # negative invalid for LB340
            return $_->[1];
        }
    }
    die "$conffile: $plc_name: no base module with macaddr\n";
}


#@ARGV or die "Usage: splityaml2json {FILE|DIR}\n";
#my $x = splityaml_load($ARGV[0]);
##print Dumper($x);
#print YAML::Dump($x);
##print encode_json($x);
#
#exit 0;

1;
